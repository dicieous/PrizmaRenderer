#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec2 a_TexCoords;

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform bool u_reverse_normals;

void main()
{
	vs_out.FragPos = vec3(u_model * vec4(a_Position, 1.0));
	
	if(u_reverse_normals)
		vs_out.Normal = transpose(inverse(mat3(u_model))) * (-1.0 * a_Normals);
	else
		vs_out.Normal = transpose(inverse(mat3(u_model))) * a_Normals;
	
	vs_out.TexCoords = a_TexCoords;
	gl_Position = u_projection * u_view * vec4(vs_out.FragPos, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_SpecularMap;
uniform samplerCube u_shadowDepthMap;

uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

uniform float u_far_plane;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float ShadowCalculation(vec3 fragPos)
{
	vec3 fragToLight = fragPos - u_lightPos;
	float currentDepth = length(fragToLight);

	float bias = 0.15;
	float shadow = 0.0;
	int samples = 20;
	float viewDistance = length(u_viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance/u_far_plane)) / 25.0;

	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(u_shadowDepthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= u_far_plane;
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}

	shadow /= float(samples);
	return shadow;
}

void main()
{
	vec3 diffusecolor = texture(u_diffuseMap, fs_in.TexCoords).rgb;
	vec3 specularcolor = texture(u_SpecularMap, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0);

	//ambient
	vec3 ambient = 0.3 * diffusecolor;

	//diffuse
	vec3 lightDir = normalize(u_lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * diffusecolor;

	//specular
	vec3 viewDir = normalize(u_viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * specularcolor;

	//Calculate shadow
	float shadow = ShadowCalculation(fs_in.FragPos);
	vec3 lighting = (ambient  + (1.0 - shadow) * (diffuse + specular));

	FragColor = vec4(lighting, 1.0);
}

