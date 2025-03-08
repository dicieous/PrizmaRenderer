#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 o_TexCoords;

void main()
{
	o_TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;
in vec2 o_TexCoords;

uniform sampler2D u_depthMap;
uniform float u_near_plane;
uniform float u_far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * u_near_plane * u_far_plane) / (u_far_plane + u_near_plane - z * (u_far_plane - u_near_plane));	
}

void main()
{
	float depthValue = texture(u_depthMap, o_TexCoords).r;
	FragColor = vec4(vec3(LinearizeDepth(depthValue) / u_far_plane), 1.0); // perspective
	//FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
