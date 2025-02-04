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

struct DirLight
{
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;

	float cutOff;
	float outerCutOff;
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

uniform DirLight dirLight;
uniform PointLight pointLightList[4];
uniform SpotLight spotLightList[2];

//Light Calculations
vec3 AmbientLight(vec3 diffuseTexColor)
{
	return diffuseTexColor;
}

vec3 DiffuseLight(vec3 normal, vec3 lightDir, vec3 diffuseTexColor)
{
	float diff = max(dot(lightDir, normal), 0.0f);
	return (diffuseTexColor * diff);
}

vec3 SpecularLight(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 specularTexColor)
{
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), material.shininess);
	return (specularTexColor * spec);
}

vec3 EmissionColor(vec3 emissionTexColor, vec3 diffuseTexColor)
{
	float brightness = 1.0 - length(diffuseTexColor);
	float emissionMask = step(0.1, brightness);
	return emissionTexColor * emissionMask;
}

//Light Type Calculations
vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{

	vec3 lightDir = normalize(-light.direction);

	vec3 ambient = light.ambient * diffuseTexColor;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{
	vec3 lightDir = normalize(light.position - o_FragPos);
	
	float distance = length(light.position - o_FragPos);
	float attenuation = 1.0f/ (light.constant + light.linear + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * diffuseTexColor;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{
	vec3 lightDir = normalize(light.position - o_FragPos);

	float distance = length(light.position - o_FragPos);
	float attenuation = 1.0f/ (light.constant + light.linear + light.quadratic * (distance * distance));

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 ambient = light.ambient * diffuseTexColor;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	ambient *= intensity * attenuation;
	diffuse *= intensity * attenuation;
	specular *= intensity * attenuation;

	return (ambient + diffuse + specular);
}

void main()
{
	vec3 normal = normalize(o_Normal);
	vec3 viewDir = normalize(u_viewPos - o_FragPos);

	vec3 diffuseTexColor = texture(material.diffuse, o_TexCoords).rgb;
	vec3 specularTexColor = texture(material.specular, o_TexCoords).rgb;

	//Use emission map only
	//vec3 emissionTexColor = texture(material.emission, o_TexCoords).rgb;
	vec3 emissionTexColor = vec3(0.0f);
	
	//Result

	vec3 result = vec3(0.0f);
	result += CalculateDirLight(dirLight, normal, viewDir, diffuseTexColor, specularTexColor);

	for(int i = 0; i < 4; i++)
	{
		result += CalculatePointLight(pointLightList[i], normal, viewDir, diffuseTexColor, specularTexColor);
	}

	for(int i = 0 ; i < 2; i++)
	{
		result += CalculateSpotLight(spotLightList[i], normal, viewDir, diffuseTexColor, specularTexColor);
	}

	FragColor = vec4(result, 1.0);
}