#include "cexception.h"
#include "utils.h"


ArenaQueueNode * arenas = 0;
ExceptionHandler * exception_handlers = 0;
Exception * current_exception =0;

void register_arena(Arena * arena){
    ArenaQueueNode * node = global_alloc(1,sizeof(ArenaQueueNode));
    node->arena = arena;
    node->next = arenas;
    arenas = node;
}
void deregister_arena(Arena * arena){
    ArenaQueueNode * old = arenas;
    ArenaQueueNode * current = arenas;
    while(current->arena != arena){
        old = current;
        current = current->next;
    }
    if(old != current){
        old->next = current->next;
    }
    if(arenas == current){
        arenas = 0;
    }
    global_free(current);
}

ExceptionHandler * get_except_handle(){
    ExceptionHandler * excp =  global_alloc(1, sizeof(ExceptionHandler));
    ExceptionHandler * old =exception_handlers;
    excp->next = old;
    exception_handlers = excp;
    excp->current_node = arenas;
    return excp;
}
void pop_exception_handler(){
    if(!exception_handlers){
        return;
    }
    ExceptionHandler * old = exception_handlers;
    exception_handlers = exception_handlers->next;
    global_free(old);
}
void pop_exception_handler_thunk(void*){
    pop_exception_handler();
}
Exception * new_exception(char * vtype, char * value){
    Exception * out = global_alloc(1, sizeof(Exception));
    out->data = (void*)value;
    out->name = vtype;
    out->print_fn = (void*)printf;
    return out;
}
bool excpt_is(Exception * excp, const char * v){
    return strcmp(excp->name, v) == 0?1 : 0;
}
void throw_fn(Exception * excp, int line, const char * file){
    excp->line = line;
    excp->file = (char*)file;
    if(!exception_handlers){
        fprintf(stderr, "error: unhandled exception: %s, line: %d, file:%s", excp->name, excp->line, excp->file);
        exit(1);
    }
    current_exception = excp;
    while(arenas != exception_handlers->current_node){
        ArenaQueueNode * old = arenas;
        arenas = arenas->next;
        arena_destroy(old->arena);
        global_free(old);
    }
    longjmp(exception_handlers->buff,1);
}
Exception * get_current_exception(){
    return current_exception;
}
void handle_sig(int sig, siginfo_t *si, void *unused){
    printf("hit\n");
    if(sig == SIGSEGV){
        throw(new_exception("signal", "segmentation fault"));
    } else if(sig == SIGFPE){
        throw(new_exception("signal", "floating point error"));
    }

}
void exception_setup(){
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction =(void*)handle_sig;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) exit(0);
    if (sigaction(SIGFPE, &sa, NULL) == -1) exit(0);
}