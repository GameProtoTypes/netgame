#version 450 

uniform sampler2D texture0;
in vec2 texCoord;
in float shading;
out vec4 FragColor;

void main()
{
    vec4 color = texture(texture0, texCoord);
    color.xyz *= (shading);
    FragColor = color;
}