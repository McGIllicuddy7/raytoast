#pragma once
#include "../../utils.h"
typedef struct {
    ByteVec(*func)(Arena * arena,void *, size_t data);
    size_t(*defunc)(Arena * arena,void *write_buff, void *read_buff, size_t size);
    size_t size;
    size_t array_size;
    size_t (*array_len)(void *);
    void * custom_data;
    bool is_ptr;
    bool is_array;
    bool use_custom;
}SerialArg;
ByteVec serialize(Arena * arena,void * ptr,size_t count,...);
ByteVec serialize_bytes(Arena * arena,void * ptr, size_t size);
size_t deserialize_bytes(Arena *arena,void * ptr, void*iter, size_t size);
size_t deserialize(Arena * arena,void * object, void * read_buff,size_t count,...);
ByteVec serialize_cstr(Arena * arena,void * obj, size_t size);
size_t deserialize_cstr(Arena * arena,void *write_buff, void * read_buff,size_t size);
ByteVec serialize_vec(Arena * arena , void * obj, void * data);
size_t deserialize_vec(Arena * arena,void *write_buff, void * read_buff,void* data);


#define CEREAL(T) (SerialArg){.func = serialize_bytes, .size = sizeof(T), .defunc = deserialize_bytes}
#define CEREAL_PTR(T) (SerialArg){.func = serialize_bytes, .size = sizeof(T), .is_ptr = true, .defunc = deserialize_bytes}
#define CEREAL_ARRAY(T, SIZE) (SerialArg){.func = serialize_bytes,.size = sizeof(T), .array_size = SIZE, .is_array = true, .defunc = deserialize_bytes}
#define CEREAL_PTR_ARRAY(T,ARRAY_LEN_FN) (SerialArg){.func = serialize_bytes, .size = sizeof(T), .is_array = true, .is_ptr = true, .defunc = deserialize_bytes, .array_len = ARRAY_LEN_FN}

#define CEREAL_FN(T, FN) (SerialArg){.func = FN, .size = sizeof(T), .defunc = de##FN}
#define CEREAL_PTR_FN(T, FN) (SerialArg){.func =FN, .size = sizeof(T), .is_ptr = true, .defunc = de##FN}
#define CEREAL_ARRAY_FN(T,SIZE, FN) (SerialArg){.func = FN,.size = sizeof(T), .array_size = SIZE, .is_array = true, .defunc = de##FN}
#define CEREAL_PTR_ARRAY_FN(T,ARRAY_LEN_FN, FN) (SerialArg){.func =FN, .size = sizeof(T), .is_array = true, .is_ptr = true, .defunc = de##FN, .array_len = ARRAY_LEN_FN}

#define CEREAL_SZ(SIZE) (SerialArg){.func =serialize_bytes, .size = SIZE, .defunc = deserialize_bytes}
#define CEREAL_SZ_FN(SIZE, FN) (SerialArg){.func = FN, .size = SIZE, .defunc = de##FN}
#define CEREAL_SZ_PTR_FN(SIZE, FN) (SerialArg){.func = FN, .size = SIZE, .is_ptr =true, .defunc = de##FN}


#define CEREAL_CUSTOM(T, FN, DATA)  (SerialArg){.func = (void*)FN, .use_custom= true, .defunc= (void*)de##FN,.size = sizeof(T), .custom_data = &(DATA)}

#define CEREAL_VEC(T) CEREAL_CUSTOM(T##Vec, serialize_vec, ((u64[]){sizeof(T), 4}))

#define enable_serial_type(T, field_count,fields...)\
size_t deserialize_##T(Arena * arena,void *write_buff, void * read_buff, size_t size);\
ByteVec serialize_##T(Arena * arena,void* obj,size_t size){\
    return serialize(arena,obj, field_count, fields);\
} \
size_t deserialize_##T(Arena *arena,void *write_buff, void * read_buff,size_t size){\
    return deserialize(arena,write_buff, read_buff,field_count, fields);\
}

