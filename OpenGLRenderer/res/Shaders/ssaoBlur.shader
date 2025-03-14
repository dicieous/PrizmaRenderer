#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

out vec2 o_TexCoords;

void main()
{
    o_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Pos, 1.0);
}


#type fragment
#version 450 core

out float FragColor;

in vec2 o_TexCoords;

uniform sampler2D u_ssaoInput;

void main()
{
    vec2 texelSize = 1.0 / textureSize(u_ssaoInput, 0);
    float result = 0.0;

    for(int x = -2; x < 2; ++x)
	{
		for(int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(u_ssaoInput, o_TexCoords + offset).r;
		}
	}

	FragColor = result / (4.0 * 4.0);
}