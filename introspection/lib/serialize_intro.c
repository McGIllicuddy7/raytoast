#include "serialize_intro.h"
#include "cintro.h"

//340 undecillion,282 decillion,366 nonillion ,920 octillion,938 septilion,463 sextillion,444 quintillion ,927 quadrillion,863 trillion ,358 billion,058 million ,659,thousand 840
//18 quintillion,446 quadrillion,744 trillion,041 billion,709 million ,551 thousand ,616
ByteVec serialize_intro(Arena * arena,void * ptr, ByteVec (*func)(Arena * arena, void *, size_t), size_t size){
    const char * name = find_name_of_ptr(func);
    assert(name);
    ByteVec b = func(arena, ptr, size);
    size_t name_len = strlen(name)+1;
    size_t header_len =b.length+name_len;
    ByteVec out = make(arena,Byte);
    v_append_slice(out, &header_len, 8);
    v_append_slice(out, &name_len, 8);
    v_append_slice(out, name, name_len);
    for(int i =0; i<8-name_len%8; i++){
        v_append(out, 0);
    }
    v_append_slice(out, b.items, b.length);
    unmake(b);
    return out;
}
size_t deserialize_Character(Arena * arena,void *write_buff, void * read_buff, size_t size);
size_t deserialize_intro(Arena *arena,void * writer_buff, void * read_buff){
    size_t len = *(size_t *)read_buff;
    size_t header_len = *(size_t*)(read_buff+8);
    const char * name = read_buff+16;
    void * reader = read_buff+header_len+16;
    for(int i =0; i<8-header_len%8; i++){
        reader++;
    }
    size_t (*def)(Arena * arena, void * write_buff, void * read_buff, size_t size);
    String s = string_format(arena, "_de%s", name+1);
    def = find_symbol(s.items);
    unmake(s);
    assert(def);
    void * out = arena_alloc(arena, len-header_len);
    size_t _ = def(0, out, reader, len-header_len);
    *(void **)writer_buff = out;
    return len;

}