#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    vs_out.FragPos = vec3(u_model * vec4(a_Pos, 1.0));   
    vs_out.TexCoords = a_TexCoords;
        
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    vs_out.Normal = normalize(normalMatrix * a_Normal);
    
    gl_Position = u_projection * u_view * u_model * vec4(a_Pos, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light
{
    vec3 position;
    vec3 color;
};

uniform Light u_lights[4];
uniform sampler2D u_diffuseColor;
//uniform vec3 u_viewPos;

void main()
{
    vec3 color = texture(u_diffuseColor, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    //vec3 viewDir = normalize(u_viewPos - fs_in.FragPos);

    vec3 lighting = vec3(0.0);

    for(int i = 0; i < 4; i++)
    {
         // diffuse
        vec3 lightDir = normalize(u_lights[i].position - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 result = u_lights[i].color * diff * color;      
        // attenuation (use quadratic as we have gamma correction)
        float distance = length(fs_in.FragPos - u_lights[i].position);
        result *= 1.0 / (distance * distance);
        lighting += result;
    }

    vec3 result = lighting;
    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(result, 1.0);
}
