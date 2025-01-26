#pragma once 
#ifdef __linux__
#define SYM_PREFIX ""
#else 
#define SYM_PREFIX "_"
#endif
void cintro_init(const char * path);
void * find_symbol(const char * symbol);
const char * find_name_of_ptr(void * ptr);