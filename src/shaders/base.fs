#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 normal;
in vec3 position;
// Input uniform values
uniform sampler2D diffuse;
uniform sampler2D normals;
uniform vec4 colDiffuse;
// Output fragment color
out vec4 finalColor;
uniform int divisor;
uniform int height;
uniform int width;
uniform int kernel_size;
void main()
{
    vec3 light_pos = vec3(-10.0, 0.0,0.0);
    vec3 delt = light_pos-position;
    float dist = delt.x*delt.x+delt.y*delt.y+delt.z*delt.z;
    vec4 norm = texture(normals,fragTexCoord);
    float delta = norm.x*delt.x+norm.y*delt.y+norm.z*delt.z;
    delta /= dist;
    delta*=5.0;
    if (delta<0.2){
        delta = 0.2;
    }
    else if(delta>1.0){
        delta = 1.0;
    }
    finalColor = delta * texture(diffuse, fragTexCoord);
}