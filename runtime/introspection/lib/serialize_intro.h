#pragma once
#include "cereal.h"
#include "cintro.h"
ByteVec serialize_intro(Arena * arena,void * ptr, ByteVec (*func)(Arena * arena, void *, size_t), size_t size);
size_t deserialize_intro(Arena * arena,void * writer_buff, void * read_buff);