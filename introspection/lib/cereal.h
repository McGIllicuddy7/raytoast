#pragma once
#include "utils.h"
typedef struct {
    ByteVec(*func)(Arena * arena,void *, size_t data);
    size_t(*defunc)(Arena * arena,void *write_buff, void *read_buff, size_t size);
    size_t size;
    size_t array_size;
    size_t (*array_len)(void *);
    bool is_ptr;
    bool is_array;
}SerialArg;
void * align_ptr(void * ptr, size_t size);
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
#define enable_serial_type(T, field_count,fields...)\
ByteVec serialize_##T(Arena * arena,void* obj,size_t size);\
size_t deserialize_##T(Arena *arena,void *write_buff, void * read_buff,size_t size);

#define enable_serial_type_impl(T, field_count,fields...)\
size_t deserialize_##T(Arena * arena,void *write_buff, void * read_buff, size_t size);\
ByteVec serialize_##T(Arena * arena,void* obj,size_t size){\
    return serialize(arena,obj, field_count, fields);\
} \
size_t deserialize_##T(Arena *arena,void *write_buff, void * read_buff,size_t size){\
    return deserialize(arena,write_buff, read_buff,field_count, fields);\
}

#define enable_serial_vec(T) ByteVec serialize_##T##Vec(Arena * arena, void * obj, size_t size); size_t deserialize_##T##Vec(Arena * arena, void * write_buff, void * read_buff, size_t size); 
#define enable_serial_vec_impl(T) \
ByteVec serialize_##T##Vec(Arena * arena, void * obj, size_t size){\
    T##Vec v = *(T##Vec*)obj;\
    ByteVec out = make(arena, Byte);\
    size_t obj_size = sizeof(T);\
    v_append_slice(out, &obj_size,8); \
    v_append_slice(out, &v.length, 8);\
    v_append_slice(out, (void*)v.items, obj_size*v.length);\
    return out;\
}\
size_t deserialize_##T##Vec(Arena * arena, void * write_buff, void * read_buff, size_t size){\
    size_t obj_sz= *(size_t *)read_buff;\
    size_t ln = *(size_t *)(read_buff +8);\
    T * buff = arena_alloc(arena, ln*obj_sz);\
    memcpy(buff, read_buff+16, ln*obj_sz);\
    T##Vec tmp; tmp.items = buff; tmp.length = ln; tmp.capacity = ln; tmp.arena = arena;\
    *(T##Vec*)write_buff = tmp;\
    return 16+obj_sz*ln;\
}\

enable_serial_vec(Byte);
enable_serial_vec(i8);
enable_serial_vec(i16);
enable_serial_vec(i32);
enable_serial_vec(i64);
enable_serial_vec(i128);
enable_serial_vec(u8);
enable_serial_vec(u16);
enable_serial_vec(u32);
enable_serial_vec(u64);
enable_serial_vec(u128);
enable_serial_vec(f32);
enable_serial_vec(f64);
#define enable_serial_vec_fn(T,FN) \
ByteVec serialize_##T##Vec(Arena * arena, void * obj, size_t size);\
size_t deserialize_##T##Vec(Arena * arena, void * write_buff, void * read_buff, size_t size);

#define enable_serial_vec_fn_impl(T,FN) \
ByteVec serialize_##T##Vec(Arena * arena, void * obj, size_t size){\
    T##Vec v = *(T##Vec*)obj;\
    ByteVec out = make(arena, Byte);\
    size_t obj_size = sizeof(T);\
    v_append_slice(out, &obj_size,8); \
    v_append_slice(out, &v.length, 8);\
    for(int i =0; i<v.length; i++){\
        ByteVec tmp = FN(arena, &v.items[i],obj_size);\
        v_append_slice(out, tmp.items, tmp.length);\
        unmake(tmp);\
    }\
    return out;\
}\
size_t deserialize_##T##Vec(Arena * arena, void * write_buff, void * read_buff, size_t size){\
    size_t obj_sz= *(size_t *)read_buff;\
    size_t ln = *(size_t *)(read_buff +8);\
    T * buff = arena_alloc(arena, ln*obj_sz);\
    T##Vec tmp; tmp.items = buff; tmp.length = ln; tmp.capacity = ln; tmp.arena = arena;\
    size_t count =0;\
    for(int i =0; i<tmp.length; i++){\
        count += de##FN(arena, &tmp.items[i], read_buff+16+count, obj_sz);\
    }\
    *(T##Vec*)write_buff = tmp;\
    return 16+count;\
}\
ByteVec serialize_String(Arena * arena, void * obj, size_t size);
size_t deserialize_String(Arena * arena, void * write_buff, void * read_buff, size_t size);
enable_serial_vec_fn(String, serialize_String);
enable_serial_vec_fn(cstr, serialize_cstr);