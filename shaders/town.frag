#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fPosLightSpace;

out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 fogColor;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform int lightState;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

float computeShadow() {
	vec3 normalizedCoords = fPosLightSpace.xyz / fPosLightSpace.w;

	if(normalizedCoords.z > 1.0)
		return 0.0;

	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005;

	if(currentDepth - bias > closestDepth)
		return 1.0;

	return 0.0;
}

vec4 pointLight() {
	vec3 lightVec = lightPos - fPosEye.xyz;
	float dist = length(lightVec);
	if(dist > 10.0)
		return vec4(0.0);
	float a = 10.0;
	float b = 5.0;
	float inten = 1.0 / (a * dist * dist + b * dist + 1.0);

	float ambient = 0.02;

	vec3 normal = normalize(fNormal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0);

	float specularLight = 0.50;
	vec3 viewDirection = normalize(camPos - fPosEye.xyz);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 16);
	float specular = specAmount * specularLight;

	return (texture(diffuseTexture, fTexCoords) * (diffuse * inten + ambient) + 
	texture(specularTexture, fTexCoords).r * specular) * vec4(1.0, 0.8, 0.5, 1.0);
}

vec4 direcLight() {
	float shadow = computeShadow();
	float ambient = 0.20;

	// diffuse lighting
	vec3 normal = normalize(fNormal);
	vec3 lightDirection = normalize(lightDir);
	float diffuse = (1.0 - shadow) * max(dot(normal, lightDirection), 0.0);

	// specular lighting
	float specularLight = 0.50;
	vec3 viewDirection = normalize(camPos - fPosEye.xyz);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 16);
	float specular = (1.0 - shadow) * specAmount * specularLight;

	return (texture(diffuseTexture, fTexCoords) * (diffuse + ambient) + texture(specularTexture, fTexCoords).r * specular) * vec4(lightColor, 1.0);
}

float computeFog() {
	float fogDensity = 0.005;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0, 1.0);
}

void main() {

	vec4 point = pointLight();
	vec4 direc = direcLight();

	vec4 color;

	switch(lightState) {
		case 0:
			color = (point + direc) / 2;
			break;
		case 1:
			color = direc;
			break;
		case 2:
			color = point;
			break;
		default:
			break;
	}

	float fogFactor = computeFog();
	color = mix(vec4(fogColor, 1.0), color, fogFactor);

	fColor = color;
}
