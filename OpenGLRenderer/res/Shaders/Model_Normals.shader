#type vertex
#version 450 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

out vec3 o_Normal;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	

void main()
{
	gl_Position = u_view * u_model * vec4(v_position, 1.0);
	mat3 normalMatrix = mat3(transpose(inverse(u_view * u_model)));

	o_Normal = normalize(vec3(vec4(normalMatrix * v_normal, 0.0)));
}


#type geometry
#version 450 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 o_Normal[];

const float MAGNITUDE = 0.4;

uniform mat4 u_projection;

void GenerateLine(int index)
{
	gl_Position = u_projection * gl_in[index].gl_Position;
	EmitVertex();

	gl_Position = u_projection * (gl_in[index].gl_Position + vec4(o_Normal[index], 0.0) * MAGNITUDE);
	EmitVertex();

	EndPrimitive();
}

void main()
{
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}


#type fragment
#version 450 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}
