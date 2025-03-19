#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

out vec3 o_localPos;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
    o_localPos = a_Position;  
    gl_Position =  u_projection * u_view * vec4(o_localPos, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in vec3 o_localPos;

const float PI = 3.14159265359;

uniform samplerCube u_equirectangularMap;

void main()
{
    vec3 normal = normalize(o_localPos);
    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.005;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

			irradiance += texture(u_equirectangularMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

	FragColor = vec4(irradiance, 1.0);
}


