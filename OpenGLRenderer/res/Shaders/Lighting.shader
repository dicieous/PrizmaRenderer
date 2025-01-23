#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;

uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}

#type fragment
#version 330 core

out vec4 FragColor;

uniform vec3 u_objectColor;
uniform vec3 u_lightColor;

void main()
{
	FragColor = vec4(u_objectColor * u_lightColor, 1.0);
}