#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 o_texCoord;

void main()
{
	gl_Position = vec4(a_position, 1.0);
	o_texCoord = a_texCoord;
}


#type fragment
#version 450 core

out vec4 fragColor;

in vec2 o_texCoord;

uniform sampler2D u_hdrBuffer;
uniform float u_exposure;

void main()
{
	const float gamma = 2.2;

	vec3 hdrColor = texture(u_hdrBuffer, o_texCoord).rgb;

	vec3 result = vec3(1.0) - exp(-hdrColor * u_exposure);
	result = pow(result, vec3(1.0 / gamma));

	fragColor = vec4(result, 1.0);
}


