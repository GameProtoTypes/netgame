#version 400
uniform sampler2D texture0;
in vec3 Color;
in vec2 UV;

out vec4 FragColor;

void main() {
    vec4 c = texture(texture0, UV);
    c.rgb += 0.5*Color.rgb;
    FragColor = c;
}