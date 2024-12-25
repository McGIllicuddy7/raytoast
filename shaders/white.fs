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
void main(){
    finalColor = vec4(1.0, 1.0,1.0, 1.0);
}

