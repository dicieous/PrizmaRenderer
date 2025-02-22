#type vertex
#version 450 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_TexCoords;

//Normal and World Fragment Position
out VS_OUT
{
	vec3 o_Normal;
	vec3 o_FragPos;
	vec2 o_TexCoords;
}vs_out;

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
	gl_PointSize = gl_Position.z;

	vs_out.o_TexCoords = v_TexCoords;

	vs_out.o_Normal = mat3(transpose(inverse(u_model))) * v_normal;
	vs_out.o_FragPos = vec3(u_model * vec4(v_position, 1.0f));
}


#type fragment
#version 450 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;

	float shininess;
};

struct DirLight
{					//Offsets
	vec3 direction;  //0
	
	vec3 ambient;   //16
	vec3 diffuse;   //32
	vec3 specular;  //48
}; //64 bytes

struct PointLight
{					//Offsets
	vec3 position;  //0
	
	vec3 ambient;   //16
	vec3 diffuse;   //32
	vec3 specular;	//48

	float constant; //64
    float linear;   //68
    float quadratic; //72  
}; //80 bytes

struct SpotLight
{					//Offsets				
	vec3 position; //0
	vec3 direction;//16
	
	vec3 ambient;  //32
	vec3 diffuse;  //48
	vec3 specular; //64

	float constant; //80
    float linear;  //84
    float quadratic; //88

	float cutOff;  //92
	float outerCutOff;//96 
};// 112 bytes


//Input Variables from Vertex Shader
in VS_OUT
{
	vec3 o_Normal;
	vec3 o_FragPos;
	vec2 o_TexCoords;
}fs_in;

//Output Color
out vec4 FragColor;

//Uniform Variables
uniform vec3 u_viewPos;
uniform Material material;
//uniform samplerCube u_Skybox;

layout(std140, binding = 1) uniform Light
{
	DirLight dirLight; //0
	PointLight pointLightList[4]; //304
	SpotLight spotLightList[2]; //496
}; // 640 bytes

//uniform DirLight dirLight;
//uniform PointLight pointLightList[4];
//uniform SpotLight spotLightList[2];

//Light Calculations
vec3 AmbientLight(vec4 diffuseTexColor)
{
	return diffuseTexColor.rgb;
}

vec3 DiffuseLight(vec3 normal, vec3 lightDir, vec4 diffuseTexColor)
{
	float diff = max(dot(lightDir, normal), 0.0f);
	return (diffuseTexColor.rgb * diff);
}

vec3 SpecularLight(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 specularTexColor)
{
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), material.shininess);
	return (specularTexColor * spec);
}

vec3 EmissionColor(vec3 emissionTexColor, vec4 diffuseTexColor)
{
	float brightness = 1.0 - length(diffuseTexColor.rgb);
	float emissionMask = step(0.1, brightness);
	return emissionTexColor * emissionMask;
}

//Light Type Calculations
vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir, vec4 diffuseTexColor, vec3 specularTexColor)
{

	vec3 lightDir = normalize(-light.direction);

	vec3 ambient = light.ambient * diffuseTexColor.rgb;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec4 diffuseTexColor, vec3 specularTexColor)
{
	vec3 lightDir = normalize(light.position - fs_in.o_FragPos);
	
	float distance = length(light.position - fs_in.o_FragPos);
	float attenuation = 1.0f/ (light.constant + light.linear + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * diffuseTexColor.rgb;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec4 diffuseTexColor, vec3 specularTexColor)
{
	vec3 lightDir = normalize(light.position - fs_in.o_FragPos);

	float distance = length(light.position - fs_in.o_FragPos);
	float attenuation = 1.0f/ (light.constant + light.linear + light.quadratic * (distance * distance));

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 ambient = light.ambient * diffuseTexColor.rgb;
	vec3 diffuse = light.diffuse * DiffuseLight(normal, lightDir, diffuseTexColor);
	vec3 specular = light.specular * SpecularLight(normal, viewDir, lightDir, specularTexColor);

	ambient *= intensity * attenuation;
	diffuse *= intensity * attenuation;
	specular *= intensity * attenuation;

	return (ambient + diffuse + specular);
}

//depth Test
	float near = 0.1;
	float far = 100;

	float LinearizeDepth(float depth)
	{
		float z = depth * 2.0 - 1.0;
		return (2.0 * near * far) / (far + near - z * (far - near));
	}
	
	//For Fog
	float LogisticDepth(float Depth, float Steepness = 0.5f, float offset =5.0f)
	{
		float zValue = LinearizeDepth(Depth);
		return (1.0 / (1.0 + exp(-Steepness * (zValue - offset))));
	}

	vec3 Fog(float depth, vec3 color, vec3 fogColor)
	{
		float fogFactor = 1.0 - depth;
		return mix (fogColor, color, fogFactor);
	}

void main()
{
	vec3 normal = normalize(fs_in.o_Normal);
	vec3 viewDir = normalize(u_viewPos - fs_in.o_FragPos);

	vec4 diffuseTexColor = texture(material.diffuse, fs_in.o_TexCoords);
	vec3 specularTexColor = texture(material.specular, fs_in.o_TexCoords).rgb;

	//Use emission map only
	//vec3 emissionTexColor = texture(material.emission, fs_in.o_TexCoords).rgb;
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

	//Fog
	float depth = LogisticDepth(gl_FragCoord.z);
	vec3 foggedColor = Fog(depth, result, vec3(0.85f, 0.85f, 0.90f));


	//Reflection
	float ratio = 1.00 / 1.52;
	vec3 I = normalize(fs_in.o_FragPos - u_viewPos);
	vec3 R = refract(I, normalize(normal), ratio);

	//FragColor = vec4(texture(u_Skybox, R).rgb, 1.0f);

	#define FOG 0
	#if FOG
	FragColor = vec4(foggedColor, diffuseTexColor.a);
	#else
	FragColor = vec4((result), diffuseTexColor.a);
	#endif
}