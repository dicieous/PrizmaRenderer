#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_TexCoords;

//Normal and World Fragment Position
//out vec3 o_Normal;
//out vec3 o_FragPos;
out vec2 o_TexCoords;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);

	o_TexCoords = v_TexCoords;

	//o_Normal = mat3(transpose(inverse(u_model))) * v_normal;
	//o_FragPos = vec3(u_model * vec4(v_position, 1.0f));
}

#type fragment
#version 330 core

out vec4 FragColor;

in vec2 o_TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
	//Reflection
	//vec3 I = normalize(o_FragPos - u_viewPos);
	//vec3 R = reflect(I, normalize(normal));

	FragColor = texture(texture_diffuse1, o_TexCoords);
}
