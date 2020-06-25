#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 v_normal;

out vec3 normal;
out vec3 fPos;
out vec3 lPos;
out vec3 objectColor;

uniform mat4 mvp;
uniform mat4 modelMat;

void main()
{
	normal = (modelMat * vec4(v_normal, 1.0)).xyz;
	fPos = (modelMat * vec4(pos, 1.0)).xyz;

	gl_Position = mvp * vec4(pos, 1.0);
}