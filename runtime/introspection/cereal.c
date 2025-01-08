#include "cereal.h"
static void * align_ptr(void * ptr, size_t size){
    size_t p = (size_t)ptr;
    if(p%8 ==0){
        return (void*)p;
    } 
    if(size>5){
        return (void*)(p+8-p%8);
    } 
    if(size>2){
        if(p%4 ==0){
            return (void*)p;
        }
        return (void*)(p+4-p%4);
    }
    if(size == 2){
        if(p%2 == 0){
            return (void*)p;
        }
        return (void *)(p+2-p%2);
    }
    return (void*)p;
}
ByteVec serialize_bytes(Arena * arena,void * ptr, size_t size){
    ByteVec out = make(arena, Byte);
    v_append_slice(out, ptr,size);
    return out;
}
size_t deserialize_bytes(Arena * arena,void * ptr, void*iter, size_t size){
    memcpy(ptr, iter, size);
    return size;
}
ByteVec serialize(Arena *arena,void * ptr,size_t count,...){
    void * p = ptr;
    ByteVec out = {};
    va_list args;
    va_start(args);
    for(int i =0; i<count; i++){
        SerialArg fn = va_arg(args, SerialArg);
        ByteVec nxt;
        if(fn.use_custom){
            p = align_ptr(p,fn.size);
            nxt= ((ByteVec(*)(Arena * arena, void * ptr, void * data))fn.func)(arena,p, fn.custom_data); 
            p += fn.size;
        }
        else if(fn.is_ptr && !fn.is_array){
            p = align_ptr(p,8);
            nxt = (ByteVec)make(0, Byte);
            if(!*(void**)p){
                v_append_slice(nxt, p, sizeof(void*));
            } else{
                void * one = (void*)1;
                v_append_slice(nxt, &one, sizeof(void*));
                ByteVec tmp = fn.func(arena,*(void **)p, fn.size);
                v_append_slice(nxt, tmp.items, tmp.length)
                unmake(tmp);
            }
            p += fn.size;
        } else if(fn.is_ptr && fn.is_array){
            p = align_ptr(p,8);
            nxt = (ByteVec)make(0, Byte);
            v_append_slice(nxt, &fn.array_size, sizeof(fn.array_size));
            if(!*(void**)p){
                memset(nxt.items, 0, sizeof(fn.array_size));
            } else{
                ByteVec tmp = fn.func(arena,*(void **)p, fn.size*fn.array_size);
                v_append_slice(nxt, tmp.items, tmp.length);
                unmake(tmp);
            } 
            p += fn.size;
        }else if(!fn.is_ptr && fn.is_array){
            p = align_ptr(p,fn.size);
            nxt= fn.func(arena,p, fn.size*fn.array_size);  
            p += fn.size*fn.array_size;
        }else{
            p = align_ptr(p,fn.size);
            nxt= fn.func(arena,p, fn.size); 
            p += fn.size;
        }
        v_append_slice(out, nxt.items, nxt.length);
        unmake(nxt);
    }
    return out;
}
size_t deserialize(Arena * arena,void * object, void * read_buff,size_t count,...){
    void * p = object;
    void * iter = read_buff;
    va_list args;
    va_start(args);
    for(int i =0; i<count; i++){
        SerialArg fn = va_arg(args, SerialArg);
        if(fn.custom_data){
            void * p0 = p;
            p = align_ptr(p,fn.size);
            size_t c = ((size_t(*)(Arena *, void *, void *, void *))(fn.defunc))(arena,p, iter,fn.custom_data);
            p+= fn.size;
            iter += c; 
        } else if(!fn.is_array && !fn.is_ptr){
            void * p0 = p;
            p = align_ptr(p,fn.size);
            size_t c = fn.defunc(arena,p, iter,fn.size);
            p+= fn.size;
            iter += c;
        } else if(fn.is_array && !fn.is_ptr){
            p = align_ptr(p,fn.size);
            for(int j =0; j<fn.array_size;j++){
                size_t c = fn.defunc(arena,p, iter,fn.size);
                p+= fn.size;
                iter += c;
            }
        } else if(!fn.is_array && fn.is_ptr){
            p = align_ptr(p,8);
            if(!*(size_t*)iter){
                *(void**)p = 0;
                p+=8;
                iter += 8;
                continue;
            }
            iter += 8;
            void * wrt = arena_alloc(arena,fn.size);
            size_t c = fn.defunc(arena,wrt, iter,fn.size);
            *(void **)p = wrt;
            p+= 8;
            iter += c;
        } else if(fn.is_array && fn.is_ptr){
            size_t count = fn.array_len(p);
            if(!count){
                *(void**)p = 0;
                p+=8;
                iter += 8;
                continue;
            }
            iter += 8;
            for(int j=0; j<count; j++){
                void * wrt = arena_alloc( arena,fn.size*count);
                size_t c = fn.defunc(arena,wrt, iter,fn.size);
                *(void **)p = wrt;
                iter += c;
            }
            p+= 8;
        }
    }
    return iter-read_buff;
}

ByteVec serialize_cstr(Arena * arena,void* obj, size_t _size){
    char * str = *(char **)obj;
    size_t size = strlen(str)+1;
    ByteVec out = make(arena, Byte);
    v_append_slice(out, &size, 8);
    for(int i =0; i<size;i++){
        v_append(out, str[i]);
    }
    return out;
}

size_t deserialize_cstr(Arena * arena,void *write_buff, void * read_buff,size_t _size){
    size_t size = *(size_t*)read_buff;
    char * out =arena_alloc(arena, size);
    for(int i =0; i<size; i++){
        out[i] = ((char*)(read_buff+8))[i];
    }
    *(void**)write_buff = out;
    return size+8;
}
ByteVec serialize_vec(Arena * arena , void * obj, void * data){
    size_t * sz = data;
    size_t obj_sz = *sz;
    ByteVec out = make(arena, Byte);
    voidVec * v = obj;
    v_append_slice(out, &v->length, 8);
    size_t sz_bytes= obj_sz*v->length;
    v_append_slice(out, &sz_bytes, 8);
    v_append_slice(out,v->items, sz_bytes);
    return out;
}
size_t deserialize_vec(Arena * arena,void *write_buff, void * read_buff,void * data){
    voidVec v = {};
    v.arena = arena;
    v.length = *(size_t*)(read_buff);
    v.capacity = *(size_t*)(read_buff);
    size_t sz = *(size_t*)(read_buff+8);
    v.items = arena_alloc(arena, sz);
    memcpy(v.items, read_buff+16, sz);
    memcpy(write_buff, &v, sizeof(v));
    return sz+16;
}

