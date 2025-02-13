#type vertex
#version 330 core

layout(location = 0 ) in vec2 v_position;
layout(location = 1 ) in vec2 v_texCoords;

out vec2 o_texCoords;

void main()
{
	o_texCoords = v_texCoords;
	gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0);
}

#type fragment
#version 330 core

out vec4 FragColor;

in vec2 o_texCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

void main()
{
	//Color Inversion
	//FragColor = vec4(vec3(1.0 - texture(screenTexture, o_texCoords).r), 1.0);
	
	//GrayScale
	//FragColor = texture(screenTexture, o_texCoords);
	//float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
	//FragColor = vec4(average, average, average, 1.0);

	vec2 offsets[9] = vec2[](
		vec2(-offset, offset), // top-left
		vec2(0.0f, offset), // top-center
		vec2(offset, offset), // top-right
		vec2(-offset, 0.0f), // center-left
		vec2(0.0f, 0.0f), // center-center
		vec2(offset, 0.0f), // center-right
		vec2(-offset, -offset), // bottom-left
		vec2(0.0f, -offset), // bottom-center
		vec2(offset, -offset) // bottom-right
	);

	//Sharpen
	 float KernelSharpen[9] = float[](
	 	-1, -1, -1,
	 	-1 , 8, -1,
	 	-1, -1, -1
	 );

	//Blur
	 float KernelBlur[9] = float[](
	 	1.0/16, 2.0/16, 1.0/16,
	 	2.0/16, 4.0/16, 2.0/16,
	 	1.0/16, 2.0/16, 1.0/16
	 );

	//Edge Detection
	float KernelED[9] = float[](
		1,  1, 1,
		1 ,-8, 1,
		1,  1, 1
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++){
		sampleTex[i] = vec3(texture(screenTexture, o_texCoords.st + offsets[i]));
	}

	vec3 color = vec3(0.0);
	for(int i = 0; i < 9; i++){
		color += sampleTex[i] * KernelBlur[i];
	}

	FragColor = vec4(color, 1.0);
}