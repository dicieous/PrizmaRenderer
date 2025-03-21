#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

//Normal and World Fragment Position
out vec3 o_Normal;
out vec3 o_FragPos;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;


void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);

	o_Normal = mat3(transpose(inverse(u_model))) * v_normal;
	o_FragPos = vec3(u_model * vec4(v_position, 1.0f));
}



#type fragment
#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emission;

	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

//input from vertex shader
in vec3 o_Normal;
in vec3 o_FragPos;

//Output Color
out vec4 FragColor;

//Uniforms
uniform vec3 u_viewPos;
uniform Material material;
uniform Light light;


vec3 AmbientLight()
{
	return material.ambient * light.ambient;
}

vec3 DiffuseLight(vec3 normal, vec3 lighDir)
{
	float diff = max(dot(lighDir, normal), 0.0f);
	return (material.diffuse * diff) * light.diffuse;
}

vec3 SpecularLight(vec3 normal, vec3 viewDir, vec3 lighDir)
{
	vec3 reflectDir = reflect(-lighDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), material.shininess);
	return (material.specular * spec) * light.specular;
}

vec3 EmissionColor()
{
	float brightness = 1.0 - length(material.diffuse);
	float emissionMask = step(0.1, brightness);
	return material.emission * emissionMask;
}

void main()
{
	vec3 normal = normalize(o_Normal);
	vec3 lighDir = normalize(light.position - o_FragPos);
	vec3 viewDir = normalize(u_viewPos - o_FragPos);

	vec3 ambient = AmbientLight();
	vec3 diffuse = DiffuseLight(normal, lighDir);
	vec3 specular = SpecularLight(normal, viewDir, lighDir);
	vec3 emission = EmissionColor();

	//Final Color
	vec3 result = ambient + diffuse + specular + emission;

	FragColor = vec4(result, 1.0);
}