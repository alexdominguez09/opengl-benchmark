#version 330 core
in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform bool useNormalMap;
uniform vec3 viewPos;

// Multiple lights
const int MAX_LIGHTS = 8;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform int numLights;

void main() {
    vec3 color = texture(diffuseMap, TexCoords).rgb;
    
    vec3 normal;
    if (useNormalMap) {
        normal = texture(normalMap, TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(TBN * normal);
    } else {
        normal = normalize(TBN[2]);
    }
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Accumulate lighting from all lights
    vec3 ambient = vec3(0.05) * color;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (i >= numLights) break;
        
        vec3 lightPos = lightPositions[i];
        vec3 lightColor = lightColors[i];
        
        // Attenuation
        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        // Diffuse
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * attenuation;
        
        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor * attenuation;
        
        // Attenuation for specular
        specular *= 0.5;
        
        totalDiffuse += diffuse;
        totalSpecular += specular;
    }
    
    vec3 result = ambient + (totalDiffuse + totalSpecular) * color;
    
    // Add slight rim lighting for visual interest
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 3.0) * 0.3;
    result += vec3(0.2, 0.3, 0.5) * rim;
    
    FragColor = vec4(result, 1.0);
}
