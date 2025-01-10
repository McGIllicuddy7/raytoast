#pragma once 
#ifdef CTILS_IMPLEMENTATION
#ifndef ARENA_REGISTER
#warning "exception handling requires #ARENA_REGISTER to be defined before including and defining CTILS_IMPLEMENTATION for utils.h"
#endif
#endif
#include "utils.h"
#include <setjmp.h>
typedef struct {
    char * name;
    void * data;
    void (*print_fn)(void *);
    char * file;
    int line;
}Exception;
typedef struct ArenaQueueNode{
    Arena * arena;
    struct ArenaQueueNode * next;
}ArenaQueueNode;
typedef struct ExceptionHandler{
    jmp_buf buff;
    ArenaQueueNode * current_node;
    struct ExceptionHandler * next;
}ExceptionHandler;

Exception * new_exception(char * vtype, char * value);
void throw_fn(Exception * excp, int line, const char * file);
bool excpt_is(Exception * excp, const char * v);
void pop_exception_handler_thunk(void*);
void pop_exception_handler();
ExceptionHandler * get_except_handle();
Exception * get_current_exception();
void exception_setup();
#define try(SCOPE,STATEMENT, CATCH) { if(!setjmp(get_except_handle()->buff)) {__attribute__((cleanup(pop_exception_handler_thunk))) int deferer = 0; SCOPE }else{ pop_exception_handler();STATEMENT; CATCH}}

#define catch(name) Exception * name = get_current_exception();
#define catch_if(name, fn, data) Exception * name = get_current_exception(); if(!fn(name, data)){throw(name);}

#define throw(exp) throw_fn(exp, __LINE__, __FILE__)
