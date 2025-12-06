#version 330 core
in vec3 FragPos;
in vec3 Tangent;
flat in vec3 FlatNormal;
in vec2 TexCoord;
out vec4 FragColor;
uniform int mode;
uniform bool isTerrain;
uniform sampler2D texture1;  // Diffuse
uniform sampler2D normalTexture;  // Normal map (unit 1)
uniform vec3 viewPos;
uniform vec3 ambientColor;  
uniform int invertNormal;
// Multi-light
struct Light {
    vec3 position;
    vec3 color;
    float constant;  // Attenuation
    float linear;
    float quadratic;
};
uniform int numLights;
uniform vec3 lightPositions[4];  // Max 4
uniform vec3 lightColors[4];
uniform int currentLightIndex;
// Fog
uniform int fogMode;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;


vec3 calcNormal() {
    vec3 normal = normalize(FlatNormal);
    if (textureSize(normalTexture, 0).x > 0) { 
        vec3 tangent = normalize(Tangent);
        vec3 bitangent = cross(normal, tangent);  // Simple TBN
        mat3 TBN = mat3(tangent, bitangent, normal);
        
        vec3 normalMap = texture(normalTexture, TexCoord).rgb * 2.0 - 1.0;
        normal = normalize(TBN * normalMap);
    }
    if (invertNormal == 1) {
        normal = -normal; 
    }
    return normal;
}

void main() {
    if (mode == 1) {
    vec3 base = lightColors[currentLightIndex];  
    vec3 color = base;
    if (fogMode != 0) {
        float dist = length(viewPos - FragPos);
        float fogFactor = 1.0;
        if (fogMode == 1) fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
        else if (fogMode == 2) fogFactor = clamp(exp(-fogDensity * dist), 0.0, 1.0);
        else if (fogMode == 3) fogFactor = clamp(exp(- (fogDensity * dist) * (fogDensity * dist)), 0.0, 1.0);
        color = mix(fogColor, base, fogFactor);
    }
    FragColor = vec4(color, 1.0);
    return;
}
    vec3 texColor;
    if (isTerrain) {
        float height = FragPos.y + 1.0;
        texColor = mix(vec3(0.4, 0.2, 0.1), vec3(0.2, 0.6, 0.2), smoothstep(-0.5, 0.5, height));
    } else {
        texColor = texture(texture1, TexCoord).rgb;
    }
    
    vec3 norm = calcNormal();
    
    // Multi-light
    vec3 lighting = ambientColor * texColor;  // Ambient
    for (int i = 0; i < numLights; ++i) {
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i];
        
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = 0.5 * spec * lightColors[i];  // Strength 0.5
        
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        
        lighting += (diffuse + specular) * texColor;
    }
    
    vec3 sceneColor = lighting;
    
    // Fog
    vec3 color = sceneColor;
    if (fogMode != 0) {
        float dist = length(viewPos - FragPos);
        float fogFactor = 1.0;
        if (fogMode == 1) fogFactor = clamp((fogEnd - dist) / max(0.0001, (fogEnd - fogStart)), 0.0, 1.0);
        else if (fogMode == 2) fogFactor = clamp(exp(-fogDensity * dist), 0.0, 1.0);
        else if (fogMode == 3) fogFactor = clamp(exp(- (fogDensity * dist) * (fogDensity * dist)), 0.0, 1.0);
        color = mix(fogColor, sceneColor, fogFactor);
    }
    
    //float alpha = isTerrain ? 1.0 : 0.5;
    float alpha = 1.0;
    FragColor = vec4(color, alpha);
}