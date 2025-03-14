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

out vec4 FragColor;

in vec2 o_TexCoords;

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

uniform Light u_light;
//uniform vec3 u_viewPos;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(u_gPosition, o_TexCoords).rgb;
    vec3 Normal = normalize(texture(u_gNormal, o_TexCoords).rgb);
    vec3 Diffuse = texture(u_gAlbedo, o_TexCoords).rgb;
    float AmbientOcculsion = texture(u_ssao, o_TexCoords).r;
    
    // then calculate lighting as usual
    vec3 ambient = vec3(0.4 * Diffuse * AmbientOcculsion);
    vec3 lighting  = ambient; // hard-coded ambient component
    vec3 viewDir  = normalize(-FragPos);
    float distance = length(u_light.Position - FragPos);

    // diffuse
    vec3 lightDir = normalize(u_light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * u_light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = u_light.Color * spec;
    // attenuation
    float attenuation = 1.0 / (1.0 + u_light.Linear * distance + u_light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;        
    FragColor = vec4(lighting, 1.0);
}