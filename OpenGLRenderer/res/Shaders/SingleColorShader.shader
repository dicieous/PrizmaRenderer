#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texCoords;

out vec2 o_texCoords;

//mvp
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

void main()
{
	o_texCoords = v_texCoords;
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}

#type fragment
#version 330 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}