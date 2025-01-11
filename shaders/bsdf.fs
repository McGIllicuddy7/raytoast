#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 normal;
in vec3 tangent;
in vec3 position;
// Input uniform values
uniform int use_diffuse;
uniform int use_normals;
uniform int use_roughness;
uniform int use_emmision;
uniform sampler2D emmision;
uniform sampler2D diffuse;
uniform sampler2D normals;
uniform sampler2D roughness;
uniform vec4 diffuse_tint;
uniform float roughness_tint;
uniform vec4 emmision_tint;
uniform vec4 colDiffuse;
uniform int light_count;
uniform vec3 directional_light;
uniform vec4 directional_light_color;
uniform vec4 ambient_color;
uniform vec3 light_positions[100];
uniform vec4 light_colors[100];

// Output fragment color
out vec4 finalColor;

vec4 to_srgb(vec4 v){
    float V=0.0031308;
    float U=0.04045;
    float A = 12.92;
    float C = 0.055 ;
    float Gamma =2.4;
    float u = length(v);
    if(u<V){
        return A*v;
    } else{
        vec4 nrm = normalize(v); 
        float sc = (1+C)*pow(u, 1/Gamma)-C;
        return nrm*sc;
    }
}
vec4 from_srgb(vec4 v){
    float V=0.0031308;
    float U=0.04045;
    float A = 12.92;
    float C = 0.055 ;
    float Gamma =2.4;
    float u = length(v);
    if(u<=V){
        return v/A;
    } else{
        vec4 nrm = normalize(v);
        float sc = (u+C)/(1+C);
        sc = pow(sc, Gamma);
        return nrm*Gamma;
    }
}

float vlen(vec3 v){
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
vec4 reflected(vec4 dif, vec4 col){
    float r = dif.r;
    float g = dif.g;
    float b = dif.b;
    float a = dif.a;
    float thresh = 0.02;
    if(r<thresh){
        r = thresh;
    }
    if(g<thresh){
        g = thresh;
    }
    if(b<thresh){
        b = thresh;
    }
    if(a<thresh){
        a = thresh;
    }
    return vec4(r*col.r, g*col.g, b*col.b, a*col.a);
}
float shade(vec3 direction){
    vec3 norm= normalize(normal);
    if(use_normals == 1){
        vec3 norm_old = norm;
        vec3 smp = texture(normals,fragTexCoord).rgb;
        norm = vec3(0.0, 0.0, 0.0);
        norm += smp.b*norm_old;
        norm += smp.g*tangent;
        norm += smp.r* cross(norm_old, tangent);
    }
    return dot(norm, direction);
}
vec4 calc_lighting(vec3 light_pos, vec4 light_color, vec4 col){
    vec3 delt= light_pos-position;
    float dist = delt.x*delt.x+delt.y*delt.y+delt.z*delt.z;

    float delta = shade(normalize(delt));
    delta /= sqrt(dist/10.0);
    return delta*reflected(light_color, col);
}


void main(){   
    vec4 s  = vec4(0.0, 0.0, 0.0,0.0);
    vec4 col = diffuse_tint;
    if(use_diffuse == 1){
        col = texture(diffuse, fragTexCoord);
    }
    s += reflected(col, ambient_color);
    s += shade(-directional_light)*reflected(col, directional_light_color);
    for(int i =0; i<light_count; i++){
        s += calc_lighting(light_positions[i], light_colors[i], col);
    }
    if(length(s) == 0.0){
        s = vec4(0.05, 0.05, 0.05, 1.0)*diffuse_tint;
    } else if(length(s)<0.2){
        s = normalize(s)*0.2;
    }
    if(use_emmision == 1){
        s += texture(emmision, fragTexCoord);
    } else{
        s += emmision_tint;
    }
    col.rgb = to_srgb(s).rgb;
    finalColor =col;
}
