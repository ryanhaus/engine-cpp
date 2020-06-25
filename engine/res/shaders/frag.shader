#version 330 core

in vec3 normal;
in vec3 fPos;

uniform vec3 lPos;
uniform vec3 vPos;
uniform vec3 objectColor;

out vec4 color;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lPos - fPos);
	float diff = max(dot(norm, lightDir), 0.1);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(vPos - fPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	color = vec4(result, 1.0);
}