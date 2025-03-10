#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_BiTangent;

out VS_OUT
{
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

void main()
{
	vs_out.TexCoords = a_TexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_model)));
	vec3 T = normalize(vec3(normalMatrix * a_Tangent));
	vec3 B = normalize(vec3(normalMatrix * a_BiTangent));
	vec3 N = normalize(vec3(normalMatrix * a_Normals));
	T = normalize(T - dot(T, N) * N);

	//B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	vs_out.TangentLightPos = TBN * u_lightPos;
	vs_out.TangentViewPos = TBN * u_viewPos;
	vs_out.TangentFragPos = TBN * vec3(u_model * vec4(a_Position, 1.0));

	gl_Position = u_projection * u_view * u_model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in VS_OUT
{
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} fs_in;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_DepthMap;

uniform float u_HeightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 64.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
	
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;

	vec2 p = viewDir.xy/viewDir.z * u_HeightScale;
	vec2 deltaTexCoords= p / numLayers;
	
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(u_DepthMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = texture(u_DepthMap, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(u_DepthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

void main()
{
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoord = ParallaxMapping(fs_in.TexCoords, viewDir);
	if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
		discard;

	vec3 diffusecolor = texture(u_diffuseMap, texCoord).rgb;
	vec3 normal = texture(u_NormalMap, texCoord).rgb;
	normal = normalize(vec3(normal.xy * 2.0 - 1.0, normal.z * 2.0 - 1.0));
	vec3 lightColor = vec3(0.3);

	//ambient
	vec3 ambient = 0.3 * diffusecolor;

	//diffuse
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * diffusecolor;

	//specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = spec * lightColor;

	vec3 lighting = (ambient + diffuse + specular);

	FragColor = vec4(lighting, 1.0);
}

