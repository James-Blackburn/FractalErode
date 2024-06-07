#version 430 core

layout(location=2) uniform sampler2D tree;
layout(location=4) uniform vec3 cameraPosition;
layout(location=5) uniform vec3 lightDirection;
layout(location=6) uniform vec3 lightColour;
layout(location=7) uniform vec2 fogDistances;
layout(location=8) uniform vec3 fogColour;
layout(location=9) uniform float maxHeight;


in VS_OUT{
    vec3 position;
    vec2 texCoord;
} fs_in;

out vec4 fragColour;

const float ambientStrength = 0.4;

vec4 blinnPhong(vec4 colour){
    vec3 normal = vec3(0.0, (colour.r + colour.g + colour.b) / 3.0, 0.0);
	// Perform Blinn-Phong lighting
	const vec3 ambientColour = lightColour * ambientStrength;
	const vec3 diffuseColour = max(dot(normal, lightDirection), 0.0) * lightColour;
	return vec4(ambientColour + diffuseColour, 1.0) * colour;
}

float getFogAmount(){
	const float fogFactor = (fogDistances.y - length(cameraPosition - fs_in.position)) / (fogDistances.y - fogDistances.x);
	return 1.0 - clamp(fogFactor, 0.0, 1.0);
}

void main(){
    vec4 colour = texture(tree, fs_in.texCoord);
    if (colour.a < 0.75 || colour.b > 0.3){
        discard;
    }
    const float fogAmount = getFogAmount() * max((1.0 - fs_in.position.y / maxHeight), 0.75);
	fragColour = vec4(mix(blinnPhong(colour).rgb, fogColour, fogAmount), 1.0);
}