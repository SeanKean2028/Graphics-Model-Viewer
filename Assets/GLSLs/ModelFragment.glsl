#version 330 core

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

uniform vec4 ambientLight = vec4(0.5f,0.5f,0.5f, 1);

uniform vec3 lightPos;
uniform vec3 lightColor = vec3(1);
uniform sampler2D texture_diffuse1;


out vec4 FragColor;
void main()
{
    vec3 norm = normalize(Normal);
    
    float dist = length(lightPos - FragPos);
    float attenuation = 1.0 / (dist * dist);
    
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse =  diff * lightColor;
    vec3 result = (diffuse + ambientLight.xyz) * attenuation * texture(texture_diffuse1, TexCoord).xyz ;
    FragColor = vec4(result, 1);
}