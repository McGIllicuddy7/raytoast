#include "utils.h"
#include <unistd.h>
static int alloc_count = 0;
static int global_free_count =0;

CTILS_STATIC
void * debug_alloc(size_t count, size_t size){
	alloc_count++;
	return calloc(count, size);
}

CTILS_STATIC
void debug_global_free(void * ptr){
	global_free_count++;
	free(ptr);
}

CTILS_STATIC
void *debug_realloc(void * ptr, size_t size){
    if(!ptr){
        alloc_count++;
    }
    return realloc(ptr, size);
}

CTILS_STATIC
void debug_alloc_and_global_free_counts(){
	printf("alloc count: %d, global_free_count: %d\n", alloc_count, global_free_count);
}
/*
Memory Stuff
*/

CTILS_STATIC
void mem_shift(void * start, size_t size, size_t count, size_t distance){
	char * data = (char *)start;
	for(int j = 0; j<size*distance; j++){
		for (int i = count*size; i>0; i--){
			data[i] = data[i-1];
		}
	}
}

CTILS_STATIC
void * memdup(Arena* arena, void * ptr, size_t size){
	if(!ptr){
		return 0;
	} else{
		void * out = arena_alloc(arena,size);
		memcpy(out, ptr, size);
		return out;
	}
}
/*
Arena stuff
*/
CTILS_STATIC
Arena * arena_create(){
    char * buffer = (char *)global_alloc(1,4096*8);
    char * next_ptr = buffer;
    char * end = buffer+4096*8;
    char * previous_allocation = 0;
    struct Arena * next = 0;
    Arena * out = (Arena*)global_alloc(1,sizeof(Arena));
    *out = (Arena){buffer, next_ptr, end, previous_allocation, next};
    return out;
}

CTILS_STATIC
Arena * arena_create_sized(size_t reqsize){
    size_t size = 4096;
    while(size<=reqsize){
        size *= 2;
    }
    char * buffer = (char *)global_alloc(1,size);
    char * next_ptr = buffer;
    char * end = buffer+size;
    char * previous_allocation = 0;
    struct Arena * next = 0;
    Arena * out = (Arena*)global_alloc(1,sizeof(Arena));
    *out = (Arena){buffer, next_ptr, end, previous_allocation, next};
    return out;
}

CTILS_STATIC
void arena_destroy(Arena * arena){
    if (arena == 0){
        return;
    }
    global_free(arena->buffer);
    arena_destroy(arena->next);
    global_free(arena);
}

CTILS_STATIC
void * arena_alloc(Arena * arena, size_t size){
    if(!arena){
        return global_alloc(1,size);
    }
    size_t act_sz = size+(8-size%8);
    char * previous = arena->next_ptr;
    if(previous + act_sz>arena->end){
        if (!arena->next){
            arena->next = arena_create_sized(size);
        }
        if (arena->next){
            return arena_alloc(arena->next, size);
        } else{
            return NULL;
        }
    }
    arena->next_ptr += act_sz;
    arena->previous_allocation = previous;
    return previous;
}
CTILS_STATIC
void * arena_realloc(Arena * arena, void * ptr, size_t previous_size, size_t new_size){
    if(!arena){
        return global_realloc(ptr,new_size);
    }
    size_t act_sz = new_size+(8-new_size%8);
    if (arena->previous_allocation == ptr && ptr){
        arena->next_ptr = (char*)ptr;
    }
    void * out = arena_alloc(arena, new_size);
    memmove(out, ptr, previous_size);
    return out;
}

CTILS_STATIC
void arena_reset(Arena * arena){
    arena_destroy(arena->next);
    arena->next_ptr= arena->buffer;
    arena->previous_allocation = 0;
}

CTILS_STATIC
void arena_free(Arena * arena, void * ptr){
    if(!arena){
        global_free(ptr);
    }
}
/*
Hashing
*/

CTILS_STATIC
size_t hash_bytes(Byte * bytes, size_t size){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 31;
	for(int i =0; i<size;i++){
		out += bytes[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}

CTILS_STATIC
size_t hash_int(int in){
	int tmp = in;
	return hash_bytes((Byte *)&tmp, sizeof(tmp));
}

CTILS_STATIC
size_t hash_float(float fl){
	float tmp = fl;
	return hash_bytes((Byte *)&tmp, sizeof(tmp));
}

CTILS_STATIC
size_t hash_long(long lg){
	long tmp = lg;
	return hash_bytes((Byte *)&tmp, sizeof(tmp));
}

CTILS_STATIC
size_t hash_double(double db){
	double tmp = db;
	return hash_bytes((Byte *)&tmp, sizeof(tmp));
}

CTILS_STATIC
size_t hash_string(String str){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 1;
	for(int i =0; i<len(str);i++){
		out += str.items[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
/*
Utils
*/
#ifdef __linux__
int fileno(FILE * file);
#endif

CTILS_STATIC
long get_time_microseconds(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec+tv.tv_sec*1000000;
}


static long profile_time = 0;
void begin_profile(){
	if(profile_time == 0){
		profile_time = get_time_microseconds();
	}
}

CTILS_STATIC
long end_profile(){
	if(profile_time != 0){
		long out =  get_time_microseconds()-profile_time;
		profile_time = 0;
		return out;
	}
	return -1;
}

CTILS_STATIC
void end_profile_print(const char * message){
	printf("%s took %f seconds\n",message, ((double)end_profile())/1000000);
}

CTILS_STATIC
int execute(const char ** strings){
    if(strings == nil){
        return 1;
    }
    if(*strings == nil){
        return 1;
    }
    int s = fork();
    if(!s){
        int a = execvp(strings[0], (char*const*)strings);
		exit(0);
    }else{
        return s;
    }
    return 1;
}

CTILS_STATIC
int execute_fd(int f_out, int f_in, int f_er, const char ** strings){
    if(strings == nil){
        return 1;
    }
    if(*strings == nil){
        return 1;
    }
    int s = fork();
    if(!s){
        dup2(f_out,fileno(stdout));
        dup2(f_in, fileno(stdin));
        dup2(f_er, fileno(stderr));
        int a = execvp(strings[0], (char*const*)strings);
		exit(0);
    }else{
        return s;
    }
    return 1;
}
/* 
Str stuff
*/

CTILS_STATIC
Str string_to_str(String s){
    return (Str){s.items, s.length};
}

CTILS_STATIC
String str_to_string(Arena * arena,Str s){
    char * out = (char*)arena_alloc(arena, s.length+1);
    memset(out, 0,s.length+1);
    memcpy(out, s.items, s.length);
    return (String){out, s.length, s.length, arena};
}

CTILS_STATIC
bool str_equals(Str a, Str b){
    if(a.length != b.length){
        return 0;
    } 
    for(int i= 0; i<a.length; i++){
        if(a.items[i] != b.items[i]){
            return false;
        }
    }
    return true;
}

CTILS_STATIC
void put_str_ln(Str str){
    printf("<");
    assert(str.length >0);
    for(int i =0;i<str.length; i++){
        printf("%c", str.items[i]);
    }
    printf(">\n");
}

CTILS_STATIC
char * str_to_c_string(Arena * arena, Str s){
    char * out = (char*)arena_alloc(arena, s.length+1);
    memset(out, 0,s.length+1);
    memcpy(out, s.items, s.length);
    return out;
}

CTILS_STATIC
bool lookahead_matches(Str base, int start, Str delim){
    if(start+delim.length>base.length){
        return false;
    }
    for(int i=start; i<start+delim.length; i++){
        if(base.items[i] != delim.items[i-start]){

            return false;
        }
    }
    return true;

}

CTILS_STATIC
StrVec str_split_by_delim(Arena * arena,Str base, Str delim){
    StrVec out = make(arena, Str);
    int start = 0;
    for(int i =0; i<base.length; i++){
        if(lookahead_matches(base, i, delim)){
            if(i>start){
                v_append(out, substring(base, start,i));
            }
            while(lookahead_matches(base, i, delim)){
                v_append(out, substring(base,i, i+delim.length));
                i += delim.length;
            }
            start = i;
        }
    }
    if(base.length>start){
        v_append(out, substring(base, start,base.length));
    }
    return out;
}

CTILS_STATIC
StrVec str_split_by_delim_no_delims(Arena * arena,Str base, Str delim){
    StrVec out = make(arena, Str);
    int start = 0;
    for(int i =0; i<base.length; i++){
        if(lookahead_matches(base, i, delim)){
            if(i>start){
                v_append(out, substring(base, start,i));
            }
            while(lookahead_matches(base, i, delim)){
                i += delim.length;
            }
            start = i;
        }
    }
    if(base.length>start){
        v_append(out, substring(base, start,base.length));
    }
    return out;
}

CTILS_STATIC
int strlen_cmp(const void *  a,const void * b){
    Str* s1 = (Str * )a;
    Str* s2 = (Str * )b;
    return s1->length>s2->length ? 1: s1->length<s2->length ? -1 : 0;
}

CTILS_STATIC
int strlen_cmp_reversed(const void *  a,const void * b){
    Str* s2 = (Str * )a;
    Str* s1 = (Str * )b;
    return s1->length>s2->length ? 1: s1->length<s2->length ? -1 : 0;
}
/*
String Stuff
*/
#include <stdarg.h>
bool string_equals(String a, String b);

CTILS_STATIC
String new_string(Arena * arena,const char* str){
	int l = strlen(str)+1;
    String out = make_with_cap(arena,str_type,l);
	for(int i = 0; i<l; i++){
		v_append(out, (str_type)str[i]);
	}
	v_append(out, '\0');
	return out;
}

CTILS_STATIC
String new_string_wide(Arena * arena,const wchar_t* str){
    int l = wcslen(str);
	String out = make_with_cap(arena,str_type, l);
	for(int i = 0; i<l; i++){
		v_append(out, (str_type)str[i]);
	}
	v_append(out, '\0');
	return out;
}

CTILS_STATIC
void _strconcat(String * a, const char* b, size_t b_size){
	if(sizeof(str_type) == 1){
        int l = (*a).length-2;
        int l2 = strlen(b);
		v_resize((*a), (*a).length+l2);
		for(int i=0; i<l2; i++){
			(*a).items[l+i] = (str_type)(b[i]);
		}
		(*a).items[l+l2] = '\0';
	}
	else{
		if(b_size <4){
			int l = len(*a)-1;
			v_resize((*a), len((*a))+strlen(b));
			int l2 = strlen(b);
			for(int i=0; i<strlen(b); i++){
				(*a).items[l+i] = (str_type)(b[i]);
			}
			(*a).items[l+l2] = '\0';
		}
		else {
			int l = len(*a)-1;
			v_resize((*a), len((*a))+wcslen((const wchar_t *)b));
			const wchar_t * v = (const wchar_t *)b;
			int l2 = wcslen(v);
			for(int i=0; i<l2; i++){
				(*a).items[l+i] = (str_type)(v[i]);
			}
			(*a).items[l+l2] = '\0';
		}
	}
}

CTILS_STATIC
String string_format(Arena *arena,const char * fmt, ...){
	String s =new_string(arena,"");
	va_list args;
	va_start(args, fmt);
	int l = strlen(fmt);
	for(int i = 0; i<l; i++){
		if(fmt[i] != '%'){
			str_v_append(s, fmt[i]);
		}
		else{
			if(fmt[i+1] == 'c'){
				char buff[2];
				buff[0] = (char)(va_arg(args,int));
				buff[1] = '\0';
				str_concat(s, buff);
				i++;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'u'){
				char buff[128];
				snprintf(buff,127, "%lu", va_arg(args,unsigned long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'u'){		
				char buff[128];
				snprintf(buff,127, "%u", va_arg(args,unsigned int));
				str_concat(s,buff);
				i+= 1;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'd'){
				char buff[128];
				snprintf(buff,127, "%ld", va_arg(args,long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 's'){
				str_concat(s,va_arg(args, wchar_t *));
				i+= 2;
			}
			else if(fmt[i+1] == 's'){
				char * s2 =  va_arg(args,char *);
				str_concat(s, s2);
				i++;
			}
			else if(fmt[i+1] == 'f'){
				char buff[128];
				snprintf(buff,127,"%f", va_arg(args,double));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == 'd'){
				char buff[128];
				snprintf(buff,127,"%d", va_arg(args,int));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == '%'){
				v_append(s, '%');
				i++;
			}
		}
	}
	va_end(args);
	return s;
}

CTILS_STATIC
bool string_equals(String a, String b){
	if(len(a) != len(b)){
		return 0;
	}
	for(int i= 0; i<len(a); i++){
		if(a.items[i] != b.items[i]){
			return 0;
		}
	}
	return 1;
}

CTILS_STATIC
String string_random(Arena * arena,int minlen, int maxlen){
	int length = rand()%(maxlen-minlen)+minlen;
	String out = make_with_cap(arena,str_type, length+1);
	for(int i= 0; i<length+1; i++){
		out.items[i] = 0;
	}
    v_resize(out, length+3);
	for(int i =0; i<length; i++){
		char c = rand()%(90-65)+65;
		if(rand()%2){
			c += 32;
		}
		out.items[i] = c;
	}
	out.items[length+1] = 0;
	return out;
}

/*
IO FUNCTIONALITY
*/

CTILS_STATIC
bool write_string_to_file(const char * s, const char * file_name){
	FILE * f = fopen(file_name, "w");
	if(f == 0){
		return 0;
	}
	size_t size = strlen(s);
	size_t w_size = fwrite(s, 1,size, f);
	fclose(f);
	return size == w_size;
}

CTILS_STATIC
String read_file_to_string(Arena * arena, const char *file_name){
	FILE *f= fopen(file_name, "rb");
	if (!f){
		perror("ERROR:");
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET); 
	String out = new_string(arena,"");
	v_resize(out, fsize+1);
	fread(out.items, 1, fsize, f);
	fclose(f);
	out.items[fsize]= 0;
	return out;
}

CTILS_STATIC
bool is_number(char a){
	return a == '0' || a == '1' || a == '2' || a == '3' || a == '4' || a == '5' || a == '6' || a == '7' || a == '8' || a == '9';
}
/*
 Noise stuff 
 */
typedef struct{double x; double y;} float2;

CTILS_STATIC
f64 interpolate(f64 a, f64 b, f64 s){
    return a*(1-s)+b*s;
}

CTILS_STATIC
NoiseOctave2d noise_octave_2d_new(double scale_divisor){
        i64 v0 = rand()%1000000000;
        i64 v1 = rand()%1000000000;
        i64 v2 = rand()%1000000000;
        return (NoiseOctave2d){v0,v1,v2,scale_divisor};
}

CTILS_STATIC
float2 random_gradient(NoiseOctave2d * self, i32 x, i32 y) {
        i64 w = 64;
        i64 s = w / 2;
        i64 a = x;
        i64 b = y;
        a *= self->v0;
        b ^= a << s | a >> (w - s);
        b *= self->v1;
        a &= b << s | b >> (w - s);
        a *= self->v2;
        f64 random = (f64)a * (3.14159265 / (f64)(!(!(u64)0 >> 1)));
        return (float2){cos(random), sin(random)};
        //return self.points[y as usize % self.points.len()][x as usize % self.points.len()];
}

CTILS_STATIC
f64 dot_grid_gradient(NoiseOctave2d * self, i32 ix, i32 iy, f64 x, f64 y){
    float2 gradient = random_gradient(self,ix, iy);
    f64 dx = x - (f64)ix;
    f64 dy = y - (f64)iy;
    return (dx * gradient.x + dy * gradient.y);
}

CTILS_STATIC
f64 perlin(NoiseOctave2d * self,f64 xbase, f64 ybase){
    f64 x = xbase / 16.0;
    f64 y = ybase / 16.0;
    i32 x0 = floor(x);
    i32 x1 = x0 + 1;
    i32 y0 = floor(y);
    i32 y1 = y0 + 1;
    f64 sx = (f64)x - (f64)x0;
    f64 sy = (f64)y- (f64)y0;
    f64 n00 = dot_grid_gradient(self,x0, y0, x, y);
    f64 n10 = dot_grid_gradient(self,x1, y0, x, y);
    f64 ix0 = interpolate(n00, n10, sx);
    f64 n01 = dot_grid_gradient(self,x0, y1, x, y);
    f64 n11 = dot_grid_gradient(self,x1, y1, x, y);
    f64 ix1 = interpolate(n01, n11, sx);
    f64 value = interpolate(ix0, ix1, sy);
    return value;
}