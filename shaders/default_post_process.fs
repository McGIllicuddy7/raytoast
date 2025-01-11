#version 330
in vec2 fragTexCoord;
uniform sampler2D diffuse;
uniform float height;
uniform float width;
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
float weight(int x, int y){
    float divisor = 1;
    float d = x*x+y*y;
    return 1/(d*d);
}
void main(){
    vec4 total = vec4(0.0,0.0, 0.0, 1.0);
    int sz = 3;
    float sum = 0.0;
    float weight = 0;
    for(int y =-sz; y<=sz; y++){
        for(int x =-sz; x<=sz;x++){
            vec4 col = texture(diffuse, fragTexCoord+vec2(x/width,y/height));
            if(x == 0 && y == 0){
                if(length(col)>1.6){
                    total += vec4(80.0, 80.0, 80.0, 80.0);
                } else{
                    total += col;
                }
                sum += 1;
            } else if(length(col)>1.2){
                total += col;
                sum += weight;
            }
        }
    }
    finalColor = from_srgb(total/sum);
}