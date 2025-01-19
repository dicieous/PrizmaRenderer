#type vertex
#version 330 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_TexCoord;

out vec2 o_TexCoord;
out vec3 o_color;

uniform mat4 u_MVP;

void main()
{
	gl_Position = vec4(v_Position, 0.0f, 1.0f) * u_MVP;
	o_TexCoord = v_TexCoord;
	o_color = v_color;
}

#type fragment
#version 330 core

out vec4 FragColor;
in vec3 o_color;

in vec2 o_TexCoord;

uniform sampler2D u_Texture;


void main()
{
	FragColor = texture(u_Texture, o_TexCoord) * vec4(o_color, 0.8f);
}