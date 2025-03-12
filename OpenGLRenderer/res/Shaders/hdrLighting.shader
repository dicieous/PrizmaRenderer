#type vertex
#version 450 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_TexCoords;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;

uniform bool u_inverse_normals;

void main()
{

	vs_out.TexCoords = a_TexCoords;
	
	vec3 n = u_inverse_normals ? -a_normal : a_normal;
    
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    vs_out.Normal = normalize(normalMatrix * n);

	vs_out.FragPos = vec3(u_model * vec4(a_position, 1.0f));
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}


#type fragment
#version 450 core

out vec4 FragColor;

in VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

struct Light{
	vec3 position;
	vec3 color;
};

uniform Light u_light[5];
uniform sampler2D u_diffuseTexture;
uniform vec3 u_viewPos;

void main()
{
	vec3 color = texture(u_diffuseTexture, fs_in.TexCoords).rgb;
	
	vec3 normal = normalize(fs_in.Normal);
	
	vec3 result = vec3(0.0);

	for(int i = 0; i < 5; i++)
	{
		vec3 lightDir = normalize(u_light[i].position - fs_in.FragPos);
		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = diff * u_light[i].color * color;

		float distance = length(u_light[i].position - fs_in.FragPos);
		float attenuation = 1.0 / (distance * distance);
		diffuse *= attenuation;
		result += diffuse;
	}
	
	FragColor = vec4(result, 1.0);
}

