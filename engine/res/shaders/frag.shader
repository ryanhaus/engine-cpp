#version 330 core

in float diff;

uniform vec3 objectColor;

out vec4 color;

void main()
{
	vec3 result = diff * objectColor;
	color = vec4(result, 1.0);
}