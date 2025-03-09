#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_model;

void main()
{
	gl_Position = u_model * vec4(a_Position, 1.0);
}

#type geometry
#version 450 core
 layout(triangles) in;
 layout(triangle_strip, max_vertices = 18) out;

 uniform mat4 u_shadowMatrices[6];

 out vec4 o_FragPos;

 void main()
 {
	 for(int face = 0; face < 6; face++)
	 {
		 gl_Layer = face;
		 for(int i = 0; i < 3; i++)
		 {
			 o_FragPos = gl_in[i].gl_Position;
			 gl_Position = u_shadowMatrices[face] * o_FragPos;
			 EmitVertex();
		 }
		 EndPrimitive();
	 }
 }

#type fragment
#version 450 core

in vec4 o_FragPos;

uniform vec3 u_lightPos;
uniform float u_far_plane;

void main()
{
	float lightDistance = length(o_FragPos.xyz - u_lightPos);
	lightDistance /= u_far_plane;
	gl_FragDepth = lightDistance;
}