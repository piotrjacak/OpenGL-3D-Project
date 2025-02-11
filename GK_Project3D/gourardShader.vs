#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out float fogFactor;
out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform SpotLight spotLightMoving;
uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 FragPos = vec3(view * model * vec4(aPos, 1.0));
    vec3 Normal = normalize(mat3(transpose(inverse(view * model))) * aNormal);
    vec3 viewDir = normalize(viewPos - FragPos);

    TexCoords = aTexCoords;
    
    vec3 color = CalcDirLight(dirLight, Normal, viewDir);
    color += CalcSpotLight(spotLight, Normal, FragPos, viewDir);
    color += CalcSpotLight(spotLightMoving, Normal, FragPos, viewDir);

    vertexColor = color;
    
    vec4 worldPosition = model * vec4(aPos, 1.0);
    float distance = length(worldPosition.xyz - cameraPos);
    float density = 0.05;
    fogFactor = clamp(exp(-pow(density * distance, 2.0)), 0.0, 1.0);
    
    gl_Position = projection * vec4(FragPos, 1.0);
}



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Ambient
    vec3 ambient = light.ambient * vec3(textureLod(material.diffuse, TexCoords, 0.0).rgb);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(textureLod(material.diffuse, TexCoords, 0.0).rgb);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(textureLod(material.specular, TexCoords, 0.0).rgb);

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Ambient
    vec3 ambient = light.ambient * vec3(textureLod(material.diffuse, TexCoords, 0.0).rgb);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(textureLod(material.diffuse, TexCoords, 0.0).rgb);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(textureLod(material.specular, TexCoords, 0.0).rgb);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    
    // Intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}