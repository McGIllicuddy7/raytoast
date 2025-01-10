#include "cintro.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
static const char * exe_path = "";
typedef struct {
	char * items;
	size_t length;
}ByteArray;
typedef struct{
	int command_type;
	int command_size;
	char * symbols;
	int num_symbols;
	char * string_table;
	int string_table_size;
}SymbolTableInternal;
typedef struct  __attribute__((packed))SymbolInternal {
	int name;
	char symbol_type;
	char section_number;
	short data_info;
	void * address;
}SymbolInternal;
typedef struct Symbol{
	const char * name;
	void * ptr;
}Symbol;
typedef struct SymbolTable{
	Symbol * symbols;
	size_t count;
}SymbolTable;
static size_t hash_string(const char * string){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 1;
	while(*string){
		out += *string;
		mlt*=pmlt;
		string++;
	}
	return out;
}
static int cmp_symbols(Symbol * a, Symbol * b){
	if(!a->name && b->name){
		return 1;
	} 
	if(a->name && !b->name){
		return -1;
	}
	if(!a->name && !b->name){
		return 0;
	}
	int ia = hash_string(a->name);
	int ib = hash_string(b->name);
	if(ia<ib){
		return -1;
	} else if(ia == ib){
		return 0;
	} else{
		return 1;
	}
}

static SymbolTable symbol_table = {};
static ByteArray read_file_to_string(const char *file_name){
	FILE *f= fopen(file_name, "rb");
	if (!f){
		perror("ERROR:");
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET); 
	char * out = malloc(fsize);
	fread(out, 1, fsize, f);
	return (ByteArray){out, fsize};
}
size_t get_num_loads_macho(ByteArray array){
	return *(int *)(array.items+16);
}
size_t get_size_loads_macho(ByteArray array){
	return *(int *)(array.items+20);
}
ByteArray get_commands_macho(ByteArray array){
	return (ByteArray){array.items+32, array.length-32};
}
ByteArray get_symbol_table_macho(ByteArray array){
	ByteArray cmds = get_commands_macho(array);
	int idx =0;
	while(idx<cmds.length){
		if(*(int*)(cmds.items+idx) != 0x00000002){
			idx += *(int*)(cmds.items+idx+4);
		} else{
			return (ByteArray){cmds.items+idx, 24};
		}
	}
	return (ByteArray){};
}


SymbolTableInternal parse_symbol_table_internal(ByteArray file, ByteArray table){
	ByteArray cmd = get_commands_macho(file);
	SymbolTableInternal out;
	out.command_type = *(int*)table.items;
	out.command_size = *((int *)table.items+1);
	out.symbols = (cmd.items+*((int *)table.items+2)-32);
	out.num_symbols = *((int *)table.items+3);
	out.string_table = (cmd.items+*((int *)table.items+4)-32);
	out.string_table_size = *((int *)table.items+5);
	return out;
}
SymbolTable parse_symbol_table(SymbolTableInternal symbols){
	Symbol * sims = calloc(symbols.num_symbols,sizeof(Symbol));
	size_t count = symbols.num_symbols;
	SymbolInternal * sym = (SymbolInternal*)symbols.symbols;
	long offset = 0;
	for(int i =0; i<symbols.num_symbols; i++){
		size_t size;
		if(i<symbols.num_symbols-1){
			size= (sym+1)->name-(sym)->name;
			if(size>24){
				size = sym->name-(sym+1)->name;
			}
			if(size>512){
				sym++;
				continue;
			}
		} else{
			size = symbols.string_table_size-sym->name;
		}
		if(size ==0){
			sym++;
			continue;
		}
		char * name =calloc(1,size+1);
		memcpy(name,symbols.string_table+sym->name,size);
		sims[i] = (Symbol){name, sym->address};
		sym++;
	}
	return (SymbolTable){sims, count};
}
void print_symbol_table(SymbolTable symbols){
	for(int i =0; i<symbols.count; i++){
		if(!symbol_table.symbols[i].name){
			continue;
		}
		printf("{name:%s, ptr:%p}\n", symbols.symbols[i].name, symbols.symbols[i].ptr);
	}
}
void print_symbol_table_internal(SymbolTableInternal table){
	printf("{type:%d, size:%d, symbols:%p, num_symbols:%d, strings:%p, strings_size:%d}\n", table.command_type, table.command_size, table.symbols, table.num_symbols, table.string_table, table.string_table_size);
}
void print_symbol_strings(SymbolTableInternal table){
	write(1, table.string_table, table.string_table_size);
	printf("\n");
}
void * find_symbol(const char * symbol);
void symbol_fixups(SymbolTable symbols){
	void * fixups = find_symbol("_symbol_fixups");
	ssize_t delta =(ssize_t)symbol_fixups- (ssize_t)(fixups);
	for(int i =0; i<symbols.count; i++){
		if(symbols.symbols[i].ptr){
			symbols.symbols[i].ptr += delta;
		}
	}
}

void cintro_init(const char * path){
	exe_path= path;
	ByteArray array = read_file_to_string(path);
	ByteArray symbol_array = get_symbol_table_macho(array);
	SymbolTableInternal symbols_internal = parse_symbol_table_internal(array, symbol_array);
	SymbolTable symbols = parse_symbol_table(symbols_internal);
	symbol_table = symbols;
	symbol_fixups(symbol_table);
	//qsort(symbols.symbols, symbols.count, sizeof(Symbol), (void*)cmp_symbols);
	//print_symbol_table(symbol_table);
	free(array.items);
}

void * find_symbol(const char * symbol){
	for(int i =0; i<symbol_table.count; i++){
		if(!symbol_table.symbols[i].name){
			continue;
		}
		if(strcmp(symbol, symbol_table.symbols[i].name) == 0){
			return symbol_table.symbols[i].ptr;
		}
	}
	for(int i =0; i<symbol_table.count; i++){
		if(!symbol_table.symbols[i].name){
			continue;
		}
		if(strcmp(symbol, symbol_table.symbols[i].name) == 0){
			return symbol_table.symbols[i].ptr;
		}
	}
	return 0;
}

const char * find_name_of_ptr(void * ptr){
	for(int i=0; i<symbol_table.count; i++){
		if(symbol_table.symbols[i].ptr == ptr){
			return symbol_table.symbols[i].name; 
		}
	}
	return 0;
}

