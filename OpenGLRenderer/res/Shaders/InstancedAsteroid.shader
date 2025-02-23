#type vertex
#version 420 core

layout(location = 0) in vec3 v_position;
//layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_TexCoords;
layout(location = 3) in mat4 v_InstancedPos;

out vec2 o_TexCoords;

//mvp Matrices
//uniform mat4 u_model;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_view;
	mat4 u_projection;
};

void main()
{
	gl_Position = u_projection * u_view * v_InstancedPos * vec4(v_position, 1.0);

	o_TexCoords = v_TexCoords;
}


#type fragment
#version 420 core

out vec4 FragColor;

//in vec2 g_TexCoords;
in vec2 o_TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
	FragColor = texture(texture_diffuse1, o_TexCoords);
}
