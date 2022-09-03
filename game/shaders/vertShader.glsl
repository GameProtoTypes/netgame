#version 400

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 VertexColor;


out vec3 Color;

uniform mat4 WorldToScreenTransform;
uniform mat4 LocalTransform;
uniform vec3 OverallColor;


void main()
{
   gl_Position = WorldToScreenTransform * (LocalTransform * vec4((VertexPosition.xy), 0.0, 1.0));
   Color = OverallColor;
}