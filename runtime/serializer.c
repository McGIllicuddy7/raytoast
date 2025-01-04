#include "../utils.h"
typedef struct{
    bool is_ptr;
    bool _padding[7];
    size_t size;
}SerialHeader_t;
typedef struct {    
    int size:31;
    int is_ptr:1;
}SerialType;
const size_t size = sizeof(SerialType);
ByteVec serialize(Arena * arena, void * ptr, size_t type_count,...){
    va_list args;
    va_start(args);
    ByteVec out;
    for(int i =0; i<type_count; i++){
        todo("implement serialize");
    }
}
void deserialize(void * write_buffer, ...){
    todo("implement deserialize");
}