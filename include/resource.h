#pragma once
#include "utils.h"
#define enable_resource_type(T)\
typedef struct {\
    Option##T##Vec values;\
    void (*destructor)(T value);\
}Resource##T;\
static Resource##T Resource##T##_make(void (*destructor)(T value)){\
    return (Resource##T){make(0, Option##T), destructor};\
}\
static void Resource##T##_unmake(Resource##T * self){\
    if(self->destructor){\
        for(int i =0; i<self->values.length; i++){\
                if(self->values.items[i].is_valid){\
                    self->destructor(self->values.items[i].value);\
                }\
        }\
    }\
   unmake(self->values);\
   self->values = (Option##T##Vec){};\
}\
static u32 Resource##T##_create(Resource##T * self, T value){\
    for(int i =0; i<self->values.length; i++){\
        if(!self->values.items[i].is_valid){\
            self->values.items[i] = (Option##T)Some(value);\
            self->values.items[i].is_valid = true;\
            return i;\
        }\
    }\
    Option##T v; v.is_valid = true; v.value = value;\
    int len = self->values.length;\
    v_append(self->values, v);\
    assert(len != self->values.length);\
    return self->values.length-1;\
}\
static void Resource##T##_destroy(Resource##T * self, u32 id){\
    if (self->values.length >= id){\
        return;\
    }\
    if (self->values.items[id].is_valid){\
        self->destructor(self->values.items[id].value);\
    }\
    self->values.items[id].is_valid = false;\
    \
}\
static T* Resource##T##_get(Resource##T * self, u32 id){\
    if (id >self->values.length){\
        return 0;\
    }\
    else if (!self->values.items[id].is_valid){\
        return 0;\
    }\
    return &self->values.items[id].value;\
}
