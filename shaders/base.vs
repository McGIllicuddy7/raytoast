#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec4 vertexColor;
// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 position;
out vec3 normal;
out vec3 tangent;
// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    position = (matModel*vec4(vertexPosition, 1.0)).xyz;
    normal = vertexNormal;
    tangent = vertexTangent;
    // Calculate final vertex position
    
    gl_Position = mvp*vec4(vertexPosition, 1.0);

}