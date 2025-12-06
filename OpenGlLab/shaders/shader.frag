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
// Fog uniforms (твои)
uniform int fogMode;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;

void main() {
    if (mode == 1) {
        vec3 base = lightColor;
        if (fogMode != 0) {
            float dist = length(viewPos - FragPos);
            float fogFactor = 1.0;
            if (fogMode == 1) {
                fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
            } else if (fogMode == 2) {
                fogFactor = clamp(exp(-fogDensity * dist), 0.0, 1.0);
            } else if (fogMode == 3) {
                fogFactor = clamp(exp(- (fogDensity * dist) * (fogDensity * dist)), 0.0, 1.0);
            }
            vec3 final = mix(fogColor, base, fogFactor);
            FragColor = vec4(final, 1.0);  // Свет непрозрачный (alpha=1)
            return;
        }
        FragColor = vec4(base, 1.0);
        return;
    }
    
    // Mode 0: Текстурное освещение + fog
    vec3 texColor = texture(texture1, TexCoord).rgb;
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(FlatNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    vec3 sceneColor = (ambient + diffuse + specular) * texColor;
    
    // Fog
    vec3 color = sceneColor;
    if (fogMode != 0) {
        float dist = length(viewPos - FragPos);
        float fogFactor = 1.0;
        if (fogMode == 1) {
            fogFactor = clamp((fogEnd - dist) / max(0.0001, (fogEnd - fogStart)), 0.0, 1.0);
        } else if (fogMode == 2) {
            fogFactor = clamp(exp(-fogDensity * dist), 0.0, 1.0);
        } else if (fogMode == 3) {
            fogFactor = clamp(exp(- (fogDensity * dist) * (fogDensity * dist)), 0.0, 1.0);
        }
        color = mix(fogColor, sceneColor, fogFactor);
    }
    
    // Прозрачность для моделей
    float alpha = 0.5;  // 50% прозрачности — подбери (0.3=более прозрачно, 0.8=менее)
    FragColor = vec4(color, alpha);
}