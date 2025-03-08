#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_lightSpaceMatrix;
uniform mat4 u_model;

void main()
{
	gl_Position = u_lightSpaceMatrix * u_model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{
	gl_FragDepth = gl_FragCoord.z; //This will happen by default in the GPU
}