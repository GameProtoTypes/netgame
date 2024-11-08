#version 400

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 VertexColor;
layout(location = 2) in vec2 VertexUV;

layout(location = 3) in vec2 InstanceWorldPos;
layout(location = 4) in vec3 InstanceColor;
layout(location = 5) in float InstanceAngle;

out vec3 Color;
out vec2 UV;

uniform float Scale;
uniform mat4 WorldToScreenTransform;




mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main()
{
   mat4 rotationMat = rotationMatrix(vec3(0,0,1), InstanceAngle);

   vec4 localVertexRotated;

   localVertexRotated = rotationMat * vec4((VertexPosition.xy), 0.0, 1.0);

   gl_Position = WorldToScreenTransform * ( vec4((InstanceWorldPos + localVertexRotated.xy), 0.0, 1.0));
   Color =InstanceColor;
   UV = VertexUV;

}