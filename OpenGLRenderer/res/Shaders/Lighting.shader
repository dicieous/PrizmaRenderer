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

	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

//Light Position, Normal and World Fragment Position
in vec3 o_Normal;
in vec3 o_FragPos;

out vec4 FragColor;

//Camera Pos,Object Color and Light Color
uniform vec3 u_viewPos;

//Material and Light
uniform Material material;
uniform Light light;

//Ambient Calculations
vec3 ambient = material.ambient * light.ambient;

//Diffuse Calculations
vec3 norm = normalize(o_Normal);
vec3 lighDir = normalize(light.position - o_FragPos);

float diff = max(dot(lighDir, norm), 0.0f);
vec3 diffuse = (material.diffuse * diff) * light.diffuse;

//Specular Calculations
vec3 viewDir = normalize(u_viewPos - o_FragPos);
vec3 reflectDir = reflect(-lighDir,norm);

float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
vec3 specular = (material.specular * spec) * light.specular;

//Final Color
vec3 result = ambient + diffuse + specular;

void main()
{
	FragColor = vec4(result, 1.0);
}