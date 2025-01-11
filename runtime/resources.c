#include <raylib.h>
#define RUNTIME_MOD
#include "runtime.h"
#include <raymath.h>
#include <rlgl.h>
#define true 1
#define false 0
extern Runtime RT;
u32 load_shader(const char * vertex_path, const char *frag_path){
    String name = new_string(0,vertex_path);
    str_concat(name, "\0");
    str_concat(name, frag_path);
    if(Stringu32HashTable_contains(RT.loaded_shaders, name)){
        u32 * out = Stringu32HashTable_find(RT.loaded_shaders, name);
        unmake(name);
        return *out;
    }  else{
        Shader shader = LoadShader(vertex_path, frag_path);
        u32 out = create_shader(shader);
        Stringu32HashTable_insert(RT.loaded_shaders, name, out);
        return out;
    }

}
void unload_shader(u32 id){
    if(!RT.shaders.values.items[id].is_valid){
        return;
    }
    UnloadShader(RT.shaders.values.items[id].value);
    RT.shaders.values.items[id] = (OptionShader){};
    for(int i =0; i<RT.loaded_shaders->TableSize; i++){
        Stringu32KeyValuePairVec * v = &RT.loaded_shaders->Table[i];
        for(int j =0; j<v->length; j++){
            Stringu32KeyValuePair p = v->items[i];
            if(p.value == id){
                unmake(p.key);
                v_remove((*v), j);
            }
        }
    }
}

u32 load_model(const char * path){
    String name = new_string(0,path);
    if(Stringu32HashTable_contains(RT.loaded_shaders, name)){
        u32 * out = Stringu32HashTable_find(RT.loaded_models, name);
        unmake(name);
        return *out;
    }  else{
        Model model = LoadModel(path);
        u32 out = create_model(model);
        Stringu32HashTable_insert(RT.loaded_models,name, out );
        return out;
    }
}

void unload_model(u32 id){
     if(!RT.models.values.items[id].is_valid){
        return;
    }
    UnloadModel(RT.models.values.items[id].value);
    RT.models.values.items[id] = (OptionModel){};
    for(int i =0; i<RT.loaded_models->TableSize; i++){
        Stringu32KeyValuePairVec * v = &RT.loaded_models->Table[i];
        for(int j =0; j<v->length; j++){
            Stringu32KeyValuePair p = v->items[i];
            if(p.value == id){
                unmake(p.key);
                v_remove((*v), j);
            }
        }
    }
}

String* get_model_name(u32 id){
    for(int i =0; i<RT.loaded_models->TableSize; i++){
        for(int j =0; j<RT.loaded_models->Table[i].length;j++){
            if(RT.loaded_models->Table[i].items[j].value == id){
                return &RT.loaded_models->Table[i].items[j].key;
            }
        }
    }
    return 0;
}

String* get_shader_name(u32 id){
    for(int i =0; i<RT.loaded_shaders->TableSize; i++){
        for(int j =0; j<RT.loaded_shaders->Table[i].length;j++){
            if(RT.loaded_shaders->Table[i].items[j].value == id){
                return &RT.loaded_shaders->Table[i].items[j].key;
            }
        }
    }
    return 0;
}

OptionShader get_shader(u32 id){
    return RT.shaders.values.items[id];
}
OptionModel get_model(u32 id){
    return RT.models.values.items[id];
}
OptionTexture get_texture(u32 id){
    return RT.textures.values.items[id];
}

u32 create_shader(Shader shader){
    return ResourceShader_create(&RT.shaders, shader);
}
u32 create_texture(Texture texture){
   return ResourceTexture_create(&RT.textures, texture); 
}
bool remove_texture(u32 id){
    ResourceTexture_destroy(&RT.textures, id);
    return true;
}
bool remove_shader(u32 id){
    ResourceShader_destroy(&RT.shaders, id);
    return true;
}

u32 create_model(Model model){
    return ResourceModel_create(&RT.models, model);
}
bool remove_model(u32 id){
  ResourceModel_destroy(&RT.models, id);
  return true;
}

u32 load_texture(const char * path){
    String name = new_string(0,path);
    if(Stringu32HashTable_contains(RT.loaded_textures, name)){
        u32 * out = Stringu32HashTable_find(RT.loaded_textures, name);
        unmake(name);
        return *out;
    }  else{
        Texture text = LoadTexture(path);
        u32 out = create_texture(text);
        Stringu32HashTable_insert(RT.loaded_models,name, out );
        return out;
    } 
}
String* get_texture_name(u32 id){
    for(int i =0; i<RT.loaded_textures->TableSize; i++){
        for(int j =0; j<RT.loaded_textures->Table[i].length;j++){
            if(RT.loaded_textures->Table[i].items[j].value == id){
                return &RT.loaded_textures->Table[i].items[j].key;
            }
        }
    }
    return 0;
}
void unload_texture(u32 id){
    if(!RT.textures.values.items[id].is_valid){
        return;
    }
    UnloadTexture(RT.textures.values.items[id].value);
    RT.textures.values.items[id] = (OptionTexture){};
    for(int i =0; i<RT.loaded_textures->TableSize; i++){
        Stringu32KeyValuePairVec * v = &RT.loaded_textures->Table[i];
        for(int j =0; j<v->length; j++){
            Stringu32KeyValuePair p = v->items[i];
            if(p.value == id){
                unmake(p.key);
                v_remove((*v), j);
            }
        }
    }
}