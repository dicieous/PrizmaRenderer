#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_TexCoords;

//Normal and World Fragment Position
out vec3 o_Normal;
out vec3 o_FragPos;
out vec2 o_TexCoords;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);

	o_TexCoords = v_TexCoords;

	o_Normal = mat3(transpose(inverse(u_model))) * v_normal;
	o_FragPos = vec3(u_model * vec4(v_position, 1.0f));
}


#type fragment
#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;

	float shininess;
};

struct Light
{
	//vec3 position;
	vec4 lightVector;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

//Input Variables from Vertex Shader
in vec3 o_Normal;
in vec3 o_FragPos;
in vec2 o_TexCoords;

//Output Color
out vec4 FragColor;

//Uniform Variables
uniform vec3 u_viewPos;
uniform Material material;
uniform Light light;

vec3 AmbientLight(vec3 diffuseTexColor)
{
	return diffuseTexColor * light.ambient;
}

vec3 DiffuseLight(vec3 normal, vec3 lighDir, vec3 diffuseTexColor)
{
	float diff = max(dot(lighDir, normal), 0.0f);
	return (diffuseTexColor * diff) * light.diffuse;
}

vec3 SpecularLight(vec3 normal, vec3 viewDir, vec3 lighDir, vec3 specularTexColor)
{
	vec3 reflectDir = reflect(-lighDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), material.shininess);
	return (specularTexColor * spec) * light.specular;
}

vec3 EmissionColor(vec3 emissionTexColor, vec3 diffuseTexColor)
{
	float brightness = 1.0 - length(diffuseTexColor);
	float emissionMask = step(0.1, brightness);
	return emissionTexColor * emissionMask;
}

void main()
{
	vec3 normal = normalize(o_Normal);
	vec3 lighDir;
	if(light.lightVector.w == 1.0f)
	{
		//Postion Based Light
		lighDir = normalize(light.lightVector.xyz - o_FragPos);
	}
	else if(light.lightVector.w == 0.0f)
	{
		//Directional Light
		lighDir = normalize(-light.lightVector.xyz);
	}

	float distance = length(light.lightVector.xyz - o_FragPos);
	float atttenuation = 1.0f/ (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 viewDir = normalize(u_viewPos - o_FragPos);

	vec3 diffuseTexColor = texture(material.diffuse, o_TexCoords).rgb;
	vec3 specularTexColor = texture(material.specular, o_TexCoords).rgb;

	//Use emission map only
	//vec3 emissionTexColor = texture(material.emission, o_TexCoords).rgb;
	vec3 emissionTexColor = vec3(0.0f);

	vec3 ambient = AmbientLight(diffuseTexColor) * atttenuation;
	vec3 diffuse = DiffuseLight(normal, lighDir, diffuseTexColor) * atttenuation;
	vec3 specular = SpecularLight(normal, viewDir, lighDir, specularTexColor) * atttenuation;
	vec3 emission = EmissionColor(emissionTexColor, diffuseTexColor) * atttenuation;

	//Final Color
	vec3 result = ambient + diffuse + specular + emission;

	FragColor = vec4(result, 1.0);
}