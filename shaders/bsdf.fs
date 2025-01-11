#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 normal;
in vec3 position;
// Input uniform values
uniform int use_diffuse;
uniform int use_normals;
uniform int use_roughness;
uniform sampler2D diffuse;
uniform sampler2D normals;
uniform sampler2D roughness;
uniform vec4 diffuse_tint;
uniform float roughness_tint;
uniform vec4 colDiffuse;
uniform int light_count;
uniform vec3 directional_light;
uniform vec3 light_positions[100];
uniform vec4 light_colors[100];

// Output fragment color
out vec4 finalColor;
float vlen(vec3 v){
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
vec4 calc_lighting(vec3 light_pos, vec4 light_color, vec4 col){
    vec3 delt= light_pos-position;
    float dist = delt.x*delt.x+delt.y*delt.y+delt.z*delt.z;
    vec3 norm= normalize(normal);
    float delta = dot(norm, normalize(delt));
    delta /= sqrt(dist/10.0);
    float dt = dot(light_color, col);
    if(dt<0.1){
        dt = 0.1;
    }
    return delta*dt*col;
}


void main(){   
    vec4 s  = vec4(0.0, 0.0, 0.0,0.0);
    vec4 col = diffuse_tint;
    if(use_diffuse == 1){
        col = texture(diffuse, fragTexCoord);
    }
    for(int i =0; i<light_count; i++){
        s += calc_lighting(light_positions[i], light_colors[i], col);
    }
    if(length(s) == 0.0){
        s = vec4(0.05, 0.05, 0.05, 1.0)*diffuse_tint;
    } else if(length(s)<0.2){
        s = normalize(s)*0.2;
    }
    finalColor.rgb =s.rgb;
    finalColor.a =diffuse_tint.a;
}
