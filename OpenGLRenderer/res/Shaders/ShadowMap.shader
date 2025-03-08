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
	vec4 FragPosLightSpace;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_lightSpaceMatrix;

void main()
{
	vs_out.FragPos = vec3(u_model * vec4(a_Position, 1.0));
	vs_out.Normal = transpose(inverse(mat3(u_model))) * a_Normals;
	vs_out.TexCoords = a_TexCoords;
	vs_out.FragPosLightSpace = u_lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
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
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_SpecularMap;
uniform sampler2D u_shadowMap;

uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

float ShadowCalculation(vec4 FragPosLightSpace, vec3 normal, vec3 lightDir)
{
	 vec3 projCoords = FragPosLightSpace.xyz/FragPosLightSpace.w; //Done for Perspective Only
	 projCoords = projCoords * 0.5 + 0.5;
	 float currentDepth = projCoords.z;

	 float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); //0.005 is the bias value
	 
	 float shadow = 0.0;
	 vec2 texelSize = 1.0/textureSize(u_shadowMap, 0);
	 for(int x = -1; x <= 1; ++x)
	 {
		 for(int y = -1; y<= 1; ++y)
		 {
			float pcfDepth = texture(u_shadowMap ,projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		 }
	 }

	 shadow /= 9.0;

	 if(projCoords.z > 1.0) shadow = 0.0;
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
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
	vec3 lighting = (ambient  + (1.0 - shadow) * (diffuse + specular));

	FragColor = vec4(lighting, 1.0);
}

