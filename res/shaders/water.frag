#version 430 core

layout(location=3) uniform vec3 cameraPosition;
layout(location=4) uniform vec3 lightDirection;
layout(location=5) uniform vec3 lightColour;
layout(location=6) uniform vec2 fogDistances;
layout(location=7) uniform vec3 fogColour;
layout(location=8) uniform float maxHeight;

uniform samplerCube skybox;

in VS_OUT {
    vec3 position;
    vec3 normal;
    float terrainHeight;
} fs_in;

out vec4 fragColour;

// constants - should be moved to CPU as uniforms
// -----------------------------------------------------
// lighting
const float ambientStrength = 0.3;
const float specularStrength = 2.5;	
const float shine = 512.0;

// misc
const vec3 upVector = vec3(0.0, 1.0, 0.0);
// -----------------------------------------------------

vec3 blinnPhong(vec3 colour, float specularStrength){
	// Perform Blinn-Phong lighting
	const vec3 ambientColour = colour * ambientStrength;
	// Perform Blinn-Phong lighting
	const vec3 diffuseColour = max(dot(fs_in.normal, lightDirection), 0.0) * colour;
	const vec3 viewDir = normalize(cameraPosition - fs_in.position);
	const vec3 halfwayDir = normalize(lightDirection + viewDir);
	const float specularAmount = pow(max(dot(fs_in.normal, halfwayDir), 0.0), shine);
	const vec3 specularColour = specularStrength * specularAmount * lightColour;
	
	return (ambientColour + diffuseColour) * lightColour + specularColour;
}

float getFogAmount(){
	const float fogFactor = (fogDistances.y - length(cameraPosition - fs_in.position)) / (fogDistances.y - fogDistances.x);
	return 1.0 - clamp(fogFactor, 0.0, 1.0);
}

void main(){
	if (abs(fs_in.position.y - fs_in.terrainHeight) < 0.01){
		discard;
	}

	const float transparency = smoothstep(0.01, 0.2, fs_in.position.y - fs_in.terrainHeight) * 0.7;
	vec3 incidence = normalize(fs_in.position - cameraPosition);
    vec3 reflectance = reflect(incidence, normalize(fs_in.normal));
	const float fogAmount = getFogAmount() * max((1.0 - fs_in.position.y / maxHeight), 0.75);
	fragColour = vec4(mix(blinnPhong(texture(skybox, reflectance).rgb, specularStrength), fogColour, fogAmount), transparency);
} 
