#version 450 

layout(location = 0) in uint aTileId;
layout(location = 1) in uint aShadeAttr;
layout(location = 2) in uint aOtherAttr;

uniform mat4 projection;
uniform mat4 localTransform;
uniform ivec2 mapSize;


out VS_OUT {
    uint tileId;
    uint shadeAttr;
    uint otherAttr;
} vs_out;

void main()
{
    int i = gl_VertexID;
    float x = float(i % mapSize.y); //float(i & 15);
    float y = float(i / mapSize.y); //float((i >> 4) & 15);
    gl_Position = vec4(x, y, 0, 1);
    

    vs_out.tileId = aTileId;
    vs_out.shadeAttr = aShadeAttr;
    vs_out.otherAttr = aOtherAttr;
}


