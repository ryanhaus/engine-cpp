#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 v_normal;

out float diff;

uniform vec3 lPos;
uniform mat4 mvp;
uniform mat4 modelMat;
uniform mat4 normalMat;

void main()
{
	vec3 normal = (normalMat * vec4(v_normal, 1.0)).xyz;
	diff = clamp(dot(normal, lPos), 0.2, 1.0);

	gl_Position = mvp * vec4(pos, 1.0);
}