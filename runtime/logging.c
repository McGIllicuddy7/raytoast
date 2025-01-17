#include "runtime.h"
typedef struct LogCall{
    String message;
    float remaining_duration;
    struct LogCall * next;
}LogCall;
typedef struct{
    pthread_mutex_t lock;
    LogCall * next;
}LoggingInstance;
static LoggingInstance instance = {};
void logging_init(){
    pthread_mutex_init(&instance.lock, 0);
    instance.next =0;
}
void draw_logging(){
    int start = 30;
    int x_start = 30;
    int size = 24;
    int char_offset = 4;
    int offset = 0;
    pthread_mutex_lock(&instance.lock);
    LogCall * cur = instance.next;
    float dt = GetFrameTime();
    LogCall * prev = 0;
    while(cur){
        if(cur->remaining_duration<0.0){
            if(prev){
                prev->next = cur->next;
            } else{
                instance.next = cur->next;
            }
            LogCall * old = cur;
            cur = cur->next;
            unmake(old->message);
            global_free(old);
            continue;
        } else{
            DrawText(cur->message.items, x_start, start+offset*size, size-char_offset, WHITE);
            cur->remaining_duration -= dt;
            offset += 1;
            prev = cur;
            cur = cur->next;
        }

    }
    pthread_mutex_unlock(&instance.lock);
}
void log_msg(char * msg, float duration){
    LogCall* call = global_alloc(1,sizeof(LogCall));
    call->next =0;
    call->remaining_duration = duration;
    call->message = new_string(0, msg);
    pthread_mutex_lock(&instance.lock);
    if(!instance.next){
        instance.next = call;
    } else{
        LogCall * cur = instance.next;
        while(cur->next){
            cur = cur->next;
        }
        cur->next = call;
    }
    pthread_mutex_unlock(&instance.lock);
}