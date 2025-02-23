#type vertex
#version 450 core

layout(location = 0) in vec3 v_position;

uniform mat4 u_model;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_view;
	mat4 u_projection;
};

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}


#type fragment
#version 450 core

out vec4 FragColor;

uniform vec3 u_lightColor;

void main()
{
	FragColor = vec4(u_lightColor, 1.0);
}