#version 430 core

// lighting uniforms
layout(location=3) uniform vec3 cameraPosition;
layout(location=4) uniform vec3 lightDirection;
layout(location=5) uniform vec3 lightColour;

// misc uniforms
layout(location=6) uniform vec2 fogDistances;
layout(location=7) uniform vec3 fogColour;
layout(location=8) uniform bool showWater;
layout(location=17) uniform float maxHeight;

struct Material {
	sampler2D diffuse_texture;
	sampler2D normal_texture;
};

// material uniforms
layout(location=9) uniform Material stone;
layout(location=11) uniform Material grass;
layout(location=13) uniform Material mud;
layout(location=15) uniform Material snow;

in VS_OUT {
    vec3 position;
    vec3 normal;
    float originalHeight;
    mat3 normalMatrix;
} fs_in;

out vec4 fragColour;

// constants - should be moved to CPU as uniforms
// -----------------------------------------------------
// lighting
const float ambientStrength = 0.4;
const float shine = 8;

// Define texture blending y coordinates
// no blending point for stone, as it is default
const float grassMaxY = 90.0;
const float snowMinY = 105.0;
const float mixingRegionSize = 15.0;
const vec2 grassBlend = vec2(grassMaxY-mixingRegionSize, grassMaxY);
const vec2 snowBlend = vec2(snowMinY, snowMinY + mixingRegionSize);

// misc
const vec3 upVector = vec3(0.0, 1.0, 0.0);
const float textureScaling = 0.25;
// -----------------------------------------------------

vec3 blinnPhong(vec3 colour, vec3 normal, float specularStrength, vec2 uvCoords){
	// Perform Normal-mapping

	// Calculate tangent and bitangent vectors
    const vec3 edge1 = dFdx(fs_in.position);
    const vec3 edge2 = dFdy(fs_in.position);
    const vec2 deltaUV1 = edge1.xz;
    const vec2 deltaUV2 = edge2.xz;

    const float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    const vec3 tangent = normalize((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f);
    //const vec3 bitangent = normalize((edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f);

	vec3 T = normalize(fs_in.normalMatrix * tangent);
    const vec3 N = fs_in.normal;
    T = normalize(T - dot(T, N) * N);
	const mat3 TBN = transpose(mat3(T, cross(N, T), N));

	// transform lighting and view directions
	const vec3 tangentLightDirection = TBN * lightDirection;
	const vec3 tangentViewPos = TBN * cameraPosition;
	const vec3 tangentFragPosition = TBN * fs_in.position;

	// Perform Blinn-Phong lighting
	
	// ambient component
	const vec3 ambientColour = ambientStrength * lightColour;
	
	// diffuse component
	const vec3 diffuseColour = max(dot(normal, tangentLightDirection), 0.0) * lightColour;

	// specular component
	const vec3 viewDir = normalize(tangentViewPos - tangentFragPosition);
	const vec3 halfwayDir = normalize(tangentLightDirection + viewDir);
	const float specularAmount = pow(max(dot(normal, halfwayDir), 0.0), shine);
	const vec3 specularColour = specularStrength * specularAmount * lightColour;

	return (ambientColour + diffuseColour + specularColour) * colour;
}

float getFogAmount(){
	const float fogFactor = (fogDistances.y - length(cameraPosition - fs_in.position)) / (fogDistances.y - fogDistances.x);
	return 1.0 - clamp(fogFactor, 0.0, 1.0);
}

void main(){
	// Calculate the slope of the terrain (use the Y component of the normal vector)
    const float slope = abs(dot(fs_in.normal, upVector));
    // Calculate the weights for each texture based on the slope and height
    float grassWeight = smoothstep(0.5, 0.75, slope) * (1.0 - smoothstep(grassBlend.x, grassBlend.y, fs_in.position.y));
	float mudWeight = 0.0;
    const float snowWeight = smoothstep(0.3, 0.6, slope) * smoothstep(snowBlend.x, snowBlend.y, fs_in.position.y);

	const float deltaH = fs_in.originalHeight - fs_in.position.y;
	if (deltaH < 0.0){
		const float m = 1.0 - smoothstep(0.0, 0.1, abs(deltaH));
		mudWeight = grassWeight - grassWeight * m;
		grassWeight *= m;
	}
	
	const float stoneWeight = 1.0 - grassWeight - mudWeight - snowWeight;

    // Calculate final color by blending the three textures
	const vec2 uvCoords = fs_in.position.xz * textureScaling;

	vec3 colour = vec3(0.0, 0.0, 0.0);
	vec3 normal = vec3(0.0, 0.0, 0.0);

	if (stoneWeight > 0.0){
		colour += stoneWeight * texture(stone.diffuse_texture, uvCoords).rgb;
		normal += stoneWeight * (texture(stone.normal_texture, uvCoords).rgb * 2.0 - 1.0);
	}
	if (snowWeight > 0.0){
		colour += snowWeight * texture(snow.diffuse_texture, uvCoords).rgb;
		normal += snowWeight * (texture(snow.normal_texture, uvCoords).rgb * 2.0 - 1.0);
	}
	if (grassWeight > 0.0){
		colour += grassWeight * texture(grass.diffuse_texture, uvCoords).rgb;
		normal += grassWeight * (texture(grass.normal_texture, uvCoords).rgb * 2.0 - 1.0);
	}
	if (mudWeight > 0.0){
		colour += mudWeight * texture(mud.diffuse_texture, uvCoords).rgb;
		normal += mudWeight * (texture(mud.normal_texture, uvCoords).rgb * 2.0 - 1.0);
	}

	const vec3 lighting = blinnPhong(colour, normalize(normal), 0.5, fs_in.position.xz);

	const float fogAmount = getFogAmount() * max((1.0 - fs_in.position.y / maxHeight), 0.75);
	fragColour = vec4(mix(lighting, fogColour, fogAmount), 1.0);
} 
