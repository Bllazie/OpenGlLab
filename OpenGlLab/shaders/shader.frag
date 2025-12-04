#version 330 core

in vec3 FragPos;
flat in vec3 FlatNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform int mode;
uniform sampler2D texture1;  
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
    if (mode == 1) {
        FragColor = vec4(lightColor, 1.0);  
        return;
    }
    
    vec3 texColor = texture(texture1, TexCoord).rgb;  
    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(FlatNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    float levels = 8.0; 
    diff = floor(diff * levels) / levels;

    vec3 diffuse = diff * lightColor;
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    spec = floor(spec * levels) / levels;
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * texColor;
    FragColor = vec4(result, 1.0);
}
