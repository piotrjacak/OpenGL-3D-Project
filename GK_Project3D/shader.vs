#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 Normal;
    float fogFactor;
} vs_out;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 cameraPos;

void main()
{
	vs_out.FragPos = vec3(view * model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    
    // NORMAL MAPPING
    vs_out.Normal = mat3(transpose(inverse(view * model))) * aNormal;  
    vec3 T = normalize(vs_out.Normal * vec3(view * model * vec4(aTangent, 1.0)));
    vec3 N = normalize(vs_out.Normal * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * cameraPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    
    // FOG
    vec4 worldPosition = model * vec4(aPos, 1.0);
    float distance = length(worldPosition.xyz - cameraPos);
    float density = 0.05;
    vs_out.fogFactor = clamp(exp(-pow(density * distance, 2.0)), 0.0, 1.0);

    gl_Position = projection * vec4(vs_out.FragPos, 1.0);
}