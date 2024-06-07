#version 430 core

out vec4 FragColour;

in vec3 TexCoords;

uniform samplerCube skybox;
layout(location=2) uniform vec3 fogColour;

void main()
{   
    FragColour = mix(vec4(fogColour, 1.0), texture(skybox, TexCoords), smoothstep(0.0, 0.25, TexCoords.y));
}