#version 430 core

layout(location=0) in float height;
layout(location=1) in vec3 normal;
layout(location=2) in float terrainHeight;

layout(location=0) uniform mat4 mvp;
layout(location=1) uniform mat4 model;
layout(location=2) uniform int size;

out VS_OUT {
    vec3 position;
    vec3 normal;
    float terrainHeight;
} vs_out;

void main(){
    const vec3 position = vec3(gl_VertexID % size, height, gl_VertexID / size);
    
    vs_out.normal = normalize(transpose(inverse(mat3(model))) * normalize(normal));
    vs_out.position = (vec4(position, 1.0) * model).xyz;
    vs_out.terrainHeight = terrainHeight;

    gl_Position = mvp * vec4(position, 1.0);
}