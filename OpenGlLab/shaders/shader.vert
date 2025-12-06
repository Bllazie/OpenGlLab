#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
flat out vec3 FlatNormal;
out vec2 TexCoord;
out vec3 Tangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    FlatNormal = mat3(transpose(inverse(uModel))) * aNormal;  
    TexCoord = aTexCoord;
    vec3 T = normalize(vec3(1.0, 0.0, 0.0) - dot(vec3(1.0, 0.0, 0.0), FlatNormal) * FlatNormal);
    Tangent = T;
    gl_Position = uProj * uView * vec4(FragPos, 1.0);
}
