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

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_texNoise;

uniform vec3 u_samples[64];
uniform mat4 u_projection;

int kernelSize = 64;
float radius = 0.4;
float bias = 0.05;

const vec2 noiseScale = vec2(1600.0/4.0, 900.0/4.0);

void main()
{
    vec3 fragPos = texture(u_gPosition, o_TexCoords).xyz;
    vec3 normal = normalize(texture(u_gNormal, o_TexCoords).rgb);
    vec3 randomVec = normalize(texture(u_texNoise, o_TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(normalize(randomVec - normal * dot(randomVec, normal)));
    vec3 biTangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, biTangent, normal);

    float occulsion = 0.0;
    for(int i =0 ;i < kernelSize; i++)
    {
        vec3 samplePos = TBN * u_samples[i];
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = u_projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(u_gPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occulsion += (sampleDepth > samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occulsion = 1.0 - (occulsion / kernelSize);
    FragColor = occulsion;
}

