#version 430 core

layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

layout(location=0) uniform mat4 projection;
layout(location=1) uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  