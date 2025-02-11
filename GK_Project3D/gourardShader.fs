#version 330 core

in vec2 TexCoords;
in float fogFactor;
in vec3 vertexColor;

out vec4 FragColor;

uniform vec3 fogColor;

void main()
{
    vec3 color = mix(fogColor, vertexColor, fogFactor);
    FragColor = vec4(color, 1.0);
}
