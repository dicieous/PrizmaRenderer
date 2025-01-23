#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

//Normal and World Fragment Position
out vec3 o_Normal;
out vec3 o_FragPos;
out vec3 o_lightPos;

//mvp Matrices
uniform mat4 u_model;
uniform mat4 u_view;	
uniform mat4 u_projection;

//Light Position
uniform vec3 u_lightPos;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);

	o_lightPos = vec3(u_view * vec4(u_lightPos, 1.0f));

	o_Normal = mat3(transpose(inverse(u_view * u_model))) * v_normal;
	o_FragPos = vec3(u_view * u_model * vec4(v_position, 1.0f));
}

#type fragment
#version 330 core

//Light Position, Normal and World Fragment Position
in vec3 o_Normal;
in vec3 o_FragPos;
in vec3 o_lightPos;

out vec4 FragColor;

//Camera Pos,Object Color and Light Color
uniform vec3 u_viewPos;
uniform vec3 u_objectColor;
uniform vec3 u_lightColor;

//Ambient Calculationa
float ambientStrength = 0.1f;

vec3 ambient = ambientStrength * u_lightColor;

//Diffuse Calculations
vec3 norm = normalize(o_Normal);
vec3 lighDir = normalize(o_lightPos - o_FragPos);

float diff = max(dot(lighDir, norm), 0.0f);
vec3 diffuse = diff * u_lightColor;

//Specular Calculations
float specularStrength = 0.6f;

vec3 viewDir = normalize(u_viewPos - o_FragPos);
vec3 reflectDir = reflect(-lighDir,norm);

float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 64);
vec3 specular = specularStrength * spec * u_lightColor;

//Final Color
vec3 result = (ambient + diffuse + specular) * u_objectColor;

void main()
{
	FragColor = vec4(result, 1.0);
}