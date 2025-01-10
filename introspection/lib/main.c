#define CTILS_IMPLEMENTATION
#define ARENA_REGISTER
#include "utils.h"
#include "cereal.h"
#include "serialize_intro.h"
typedef struct {
    char * name;
    int ac;
    int hp;
    int strength; 
    int dexterity;
    int constitution;
    int intelligence;
    int wisdom;
    int charisma;
    cstrVec inventory;
}Character;
enable_serial_type_impl(Character, 10, CEREAL_FN(char*, serialize_cstr),
CEREAL(int), CEREAL(int), CEREAL(int), CEREAL(int), CEREAL(int), CEREAL(int), CEREAL(int), CEREAL(int), CEREAL_FN(cstrVec*, serialize_cstrVec));
int gen_stat(){
    int stats[4] = {rand()%6+1, rand()%6+1, rand()%6+1, rand()%6+1};
    int min = 1000;
    int min_idx =-1;
    for(int i =0; i<4; i++){
        if(stats[i]<min){
            min = stats[i];
            min_idx =i;
        }
    }
    int out =0;
    for(int i =0; i<4; i++){
        if(i == min_idx){
            continue;
        }
        out+=stats[i];
    }
    return out;
}
int calculate_mod(int stat){
    return (stat-10)/2;
}
Character make_character(const char * name, const char ** items){
    Character out;
    out.name = (char*)name;
    out.strength = gen_stat();
    out.dexterity = gen_stat();
    out.constitution = gen_stat();
    out.intelligence = gen_stat();
    out.wisdom = gen_stat();
    out.charisma = gen_stat();
    out.ac = 11+calculate_mod(out.dexterity);
    out.hp = 8+calculate_mod(out.constitution);
    cstrVec its = make(0, cstr);
    const char ** item = items;
    while(*item){
        v_append(its, ((char*)*item));
        item++;
    }
     out.inventory = its;
    return out;
}
void generate(){
    Character c = make_character("harry seldon", (const char *[]){"sword", "leather armor","potion of greater healing",0});
    ByteVec vc = serialize_intro(0, &c, serialize_Character, sizeof(c));
    write_bytes_to_file(vc,"harry.bin");
}
void read_file(){
    ByteVec v = read_file_to_bytes(0, "harry.bin");
    Character *c2;
    deserialize_intro(0, &c2, v.items);
    printf("%s\n", c2->name);
    printf("ac: %d ", c2->ac);
    printf("hp: %d\n", c2->hp);
    printf("strength: %d\n", c2->strength);
    printf("dexterity: %d\n", c2->dexterity);
    printf("constitution: %d\n", c2->constitution);
    printf("intelligence: %d\n", c2->intelligence);
    printf("wisdom: %d\n", c2->wisdom);
    printf("charisma: %d\n", c2->charisma);
    printf("inventory: ");
    for(int i =0; i<c2->inventory.length; i++){
        printf("%s", c2->inventory.items[i]);
        if(i<c2->inventory.length-1){
            printf(", ");
        }
    }
    printf("\n");
}
int main(int argc, const char ** argv){
    srand(time(0));
    cintro_init(*argv);
    generate();
    read_file();
   //generate();
}