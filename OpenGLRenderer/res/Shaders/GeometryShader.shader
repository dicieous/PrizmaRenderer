#type vertex
#version 450 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 a_Color;

out VS_OUT
{
	vec3 o_Color;
} vs_out;

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
	vs_out.o_Color = a_Color;
}

#type fragment
#version 450 core

in vec3 fColor;
out vec4 frag_color;

void main()
{
	frag_color = vec4(fColor, 1.0);
}

#type geometry
#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 5) out;

in VS_OUT
{
	vec3 o_Color;
} gs_in[];

out vec3 fColor;

void Build_House(vec4 position)
{
	fColor = gs_in[0].o_Color;

	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0);
	fColor = vec3(1.0, 1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

void main()
{
	Build_House(gl_in[0].gl_Position);
}