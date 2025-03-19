#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;

out vec3 o_localPos;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
    o_localPos = a_Position;
    
    mat4 rotView = mat4(mat3(u_view)); //Remove Rotation from View Matrix
    vec4 clipPos = u_projection * rotView * vec4(a_Position, 1.0);
    
    gl_Position = clipPos.xyww;
}


#type fragment
#version 450 core
out vec4 FragColor;

in vec3 o_localPos;

uniform samplerCube u_environmentMap;

void main()
{
	vec3 envColor = texture(u_environmentMap, o_localPos).rgb;

    envColor = envColor / (envColor + vec3(1.0)); //Tonemapping
    envColor = pow(envColor, vec3(1.0/2.2)); //Gamma Correction

	FragColor = vec4(envColor, 1.0);
}
