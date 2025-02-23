#type vertex
#version 420 core

layout(location = 0) in vec3 v_position;
//layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_TexCoords;

out vec2 o_TexCoords;

//mvp Matrices
uniform mat4 u_model;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_view;
	mat4 u_projection;
};

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);

	o_TexCoords = v_TexCoords;
}


// type geometry
// #version 450 core

// layout(triangles) in;
// layout(triangle_strip, max_vertices = 3) out;

// in vec2 o_TexCoords[];

// out vec2 g_TexCoords;

// uniform float u_time;

// vec3 CalculateNormals()
// {
// 	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
// 	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);

// 	return normalize(cross(a, b));
// }

// vec4 explode(vec4 position, vec3 normal)
// {
// 	float magnitude = 2.0;
// 	vec3 direction = normal * ((sin(u_time) + 1.0)/ 2.0) * magnitude;
// 	return position + vec4(direction, 0.0);
// }

// void main()
// {
// 	vec3 normal = CalculateNormals();
// 	for(int i = 0; i < gl_in.length(); i++)
// 	{
// 		gl_Position = explode(gl_in[i].gl_Position, normal);
// 		g_TexCoords = o_TexCoords[i];
// 		EmitVertex();
// 	}

// 	EndPrimitive();
// }


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
