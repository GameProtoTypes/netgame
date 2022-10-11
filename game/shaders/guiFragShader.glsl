#version 400
uniform sampler2D texture0;
uniform sampler2D texture1;
in vec3 Color;
in vec2 UV;

out vec4 FragColor;

void main() {


    vec4 c;
    if(UV.x > 1.0)
    {
        c = texture(texture1, UV);
    }
    else
    {
        c = texture(texture0, UV);
    }

    c.rgb = c.rgb*Color.rgb;
    FragColor = c;
}