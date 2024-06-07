#version 430 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;
layout(location=3) in vec3 aOffset;

layout(location=0) uniform mat4 projection;
layout(location=1) uniform mat4 view;
layout(location=3) uniform mat4 model;

out VS_OUT {
    vec3 position;
    vec2 texCoord;
} vs_out;

void main(){
    vs_out.texCoord = aTexCoord;
    vs_out.position = (vec4(aPos + aOffset, 1.0) * model).xyz;
    gl_Position = projection * view * model * vec4(aPos + aOffset, 1.0);
}