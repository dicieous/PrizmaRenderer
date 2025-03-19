#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out VS_OUT {
    vec3 worldPos;
    vec2 TexCoords;
    vec3 Normal;
} vs_out;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	vs_out.worldPos = vec3(u_model * vec4(a_Position, 1.0));
	vs_out.TexCoords = a_TexCoord;

	mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    vs_out.Normal = normalize(normalMatrix * a_Normal);

    gl_Position = u_projection * u_view * u_model * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

out vec4 FragColor;

const float PI = 3.14159265359;

in VS_OUT {
	vec3 worldPos;
	vec2 TexCoords;
	vec3 Normal;
} fs_in;

uniform vec3 u_viewPos;
uniform vec3 u_lightPositions[4];
uniform vec3 u_lightColors[4];


uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;

uniform samplerCube u_irradianceMap;


vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 0.5);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void main()
{
	vec3 N = normalize(fs_in.Normal); //NormalDir
	vec3 V = normalize(u_viewPos - fs_in.worldPos); //ViewDir

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, u_albedo, u_metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < 4; i++)
	{
		vec3 L = normalize(u_lightPositions[i] - fs_in.worldPos); //LightDir
		vec3 H = normalize(V + L); //HalfwayDir

		float distance = length(u_lightPositions[i] - fs_in.worldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = u_lightColors[i] * attenuation;

		vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
		float NDF = DistributionGGX(N, H, u_roughness);
		float G = GeometrySmith(N, V, L, u_roughness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(0.0, dot(N, V)) * max(0.0, dot(N, L)) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;

		kD *= 1.0 - u_metallic;

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * u_albedo / PI + specular) * radiance * NdotL;
	}

	vec3 kS = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, u_roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - u_metallic;
	vec3 irradiance = texture(u_irradianceMap, N).rgb;
	vec3 diffuse = irradiance * (u_albedo / PI);
	vec3 ambient = (kD * diffuse) * u_ao;
	vec3 color = ambient + Lo;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color, 1.0);
}
