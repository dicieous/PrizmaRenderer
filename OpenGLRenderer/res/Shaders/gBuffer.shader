#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    vec4 worldPos = u_model * vec4(a_Pos, 1.0);
    vs_out.FragPos = worldPos.xyz;   
    vs_out.TexCoords = a_TexCoords;
        
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    vs_out.Normal = normalize(normalMatrix * a_Normal);
    
    gl_Position = u_projection * u_view * worldPos;
}

#type fragment
#version 450 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    gPosition = fs_in.FragPos;
    gNormal =normalize(fs_in.Normal);
    gAlbedoSpec.rgb = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    gAlbedoSpec.a = texture(texture_specular1, fs_in.TexCoords).r;
}
