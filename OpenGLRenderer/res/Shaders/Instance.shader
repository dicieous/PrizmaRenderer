#type vertex
#version 450 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_Offset;

out vec3 o_Color;

void main()
{
	gl_Position = vec4(a_Position + a_Offset, 0.0, 1.0f);
	o_Color = a_Color;
}


#type fragment
#version 450 core

out vec4 FragColor;

in vec3 o_Color;

void main()
{
	FragColor = vec4(o_Color, 1.0f);
}
