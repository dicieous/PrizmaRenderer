#type vertex
#version 330 core

layout(location = 0) in vec3 v_position;

out vec3 o_texCoords;

//vp
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	o_texCoords = v_position;
	vec4 pos = u_projection * u_view * vec4(v_position, 1.0);
	gl_Position = pos.xyww;
}

#type fragment
#version 330 core

out vec4 FragColor;

in vec3 o_texCoords;

uniform samplerCube u_skybox;
 
void main()
{
	FragColor = texture(u_skybox, o_texCoords);
}