#version 330 core
in vec3 FragPos;
flat in vec3 FlatNormal;
in vec2 TexCoord;
out vec4 FragColor;
uniform int mode;
uniform bool isTerrain;  // <-- НОВОЕ: Для procedural terrain
uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
// Fog uniforms
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
            FragColor = vec4(final, 1.0);
            return;
        }
        FragColor = vec4(base, 1.0);
        return;
    }
    
    vec3 texColor;
    if (isTerrain) {  // <-- ФИКС: Procedural для terrain
        // Цвет по высоте: низ=коричневый (земля), высок=зелёный (трава)
        float height = FragPos.y + 2.0;  // Offset
        texColor = mix(vec3(0.4, 0.2, 0.1), vec3(0.2, 0.6, 0.2), smoothstep(-0.5, 0.5, height));
    } else {
        texColor = texture(texture1, TexCoord).rgb;
    }
    
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
    
    // Прозрачность только для моделей (!terrain)
    float alpha = isTerrain ? 1.0 : 0.5;  // <-- ФИКС: Terrain opaque
    FragColor = vec4(color, alpha);
}