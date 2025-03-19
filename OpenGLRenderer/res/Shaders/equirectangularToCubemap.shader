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

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

in vec3 o_localPos;

uniform sampler2D u_equirectangularMap;

void main()
{
    vec2 uv = SampleSphericalMap(normalize(o_localPos));
	vec3 color = texture(u_equirectangularMap, uv).rgb;
	
	FragColor = vec4(color, 1.0);
}


