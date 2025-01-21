#type vertex
#version 330 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec2 v_TexCoord;

out vec2 o_TexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view *  u_model * vec4(v_Position, 1.0f);
	o_TexCoord = v_TexCoord;
}

#type fragment
#version 330 core

out vec4 FragColor;

in vec2 o_TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_Texture_2;


void main()
{
	FragColor = mix(texture(u_Texture, o_TexCoord), texture(u_Texture_2, o_TexCoord), 0.2f) /* * vec4(o_color, 0.8f) */;
}