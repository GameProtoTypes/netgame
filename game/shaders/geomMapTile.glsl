#version 450 


uniform mat4 projection;
uniform mat4 localTransform;

in VS_OUT {
    uint tileId;
    uint shadeAttr;
    uint otherAttr;
} gs_in[];

out vec2 texCoord;
out float shading;

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

float topShadingA;
float topShadingB;
float topShadingC;
float topShadingD;
float topShadingX;

float tileX;
float tileY;
const float B = 1 / 256.0;
const float S = 1 / 16.0;
uint rot = 0;
uint cornerIdx = 0;


void CornerA()
{
    texCoord = vec2(tileX + B, tileY + B);
}
void CornerB()
{
    texCoord = vec2(tileX + S - B, tileY + B);
}
void CornerC()
{
     texCoord = vec2(tileX + B, tileY + S - B);
}
void CornerD()
{
     texCoord = vec2(tileX + S - B, tileY + S - B);
}
void CornerX()
{
     texCoord = vec2(tileX + S*0.5 , tileY + S*0.5 );
}




void NextCorner()
{
        if(cornerIdx == 0)
        {
            CornerA();
            cornerIdx=1;
        }
        else if(cornerIdx == 1)
        {
            CornerB();
            cornerIdx=2;
        }
        else if(cornerIdx == 2)
        {
            CornerX();
            cornerIdx=3;
        }
        else if(cornerIdx == 3)
        {
            CornerB();
            cornerIdx=4;
        }
        else if(cornerIdx == 4)
        {
            CornerD();
            cornerIdx=5;
        }
        else if(cornerIdx == 5)
        {
            CornerX();
            cornerIdx=6;
        }
        else if(cornerIdx == 6)
        {
            CornerD();
            cornerIdx=7;
        }
        else if(cornerIdx == 7)
        {
            CornerC();
            cornerIdx=8;
        }
        else if(cornerIdx ==8)
        {
            CornerX();
            cornerIdx=9;
        }        
        else if(cornerIdx == 9)
        {
            CornerC();
            cornerIdx=10;
        }
        else if(cornerIdx == 10)
        {
            CornerA();
            cornerIdx=11;
        }
        else if(cornerIdx == 11)
        {
            CornerX();
            cornerIdx=0;
        }
}


void main() {
    uint cornerALow =  (gs_in[0].shadeAttr & (1)) >> 0;
    uint cornerBLow =  (gs_in[0].shadeAttr & (1 << 1)) >> 1;
    uint cornerCLow =  (gs_in[0].shadeAttr & (1 << 2)) >> 2;
    uint cornerDLow =  (gs_in[0].shadeAttr & (1 << 3 )) >> 3;
    uint cornerXLow =  (gs_in[0].shadeAttr & (3 << 4 )) >> 4;
    float shadingBase = float((gs_in[0].shadeAttr & (15u << 6)) >> 6)/15.0;
    uint lowConerBlackBit =  (gs_in[0].shadeAttr & (1 << 10));

    float shadingLowAdd = 1/15.0;
    if(lowConerBlackBit != 0)
        shadingLowAdd = shadingBase;


    topShadingA = shadingBase - float(cornerALow)*shadingLowAdd; 
    topShadingB = shadingBase - float(cornerBLow)*shadingLowAdd; 
    topShadingC = shadingBase - float(cornerCLow)*shadingLowAdd; 
    topShadingD = shadingBase - float(cornerDLow)*shadingLowAdd; 


    if(cornerXLow == 0)
        topShadingX = shadingBase - 0.0*shadingLowAdd; 
    if(cornerXLow == 1)
        topShadingX = shadingBase - 0.5*shadingLowAdd; 
    if(cornerXLow == 2)
        topShadingX = shadingBase - 1.0*shadingLowAdd; 


    rot = (gs_in[0].otherAttr & 15u);

    cornerIdx = rot*3;

    if(gs_in[0].tileId != 255)
    {
        uint tileId = gs_in[0].tileId & 255u;
        tileX = float(tileId & 15u) / 16.0;
        tileY = float((tileId >> 4u) & 15u) / 16.0;

        gl_Position = projection * (localTransform * gl_in[0].gl_Position);
        NextCorner();
        shading = topShadingA;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingB;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.5, 0.5, 0.0, 0.0)));
        NextCorner();
        shading = topShadingX;
        EmitVertex();
        EndPrimitive();





        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingB;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingD;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.5, 0.5, 0.0, 0.0)));
        NextCorner();
        shading = topShadingX;
        EmitVertex();
        EndPrimitive();





        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingD;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingC;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.5, 0.5, 0.0, 0.0)));
        NextCorner();
        shading = topShadingX;
        EmitVertex();
        EndPrimitive();





        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingC;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.0, 0.0, 0.0, 0.0)));
        NextCorner();
        shading = topShadingA;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.5, 0.5, 0.0, 0.0)));
        NextCorner();
        shading = topShadingX;
        EmitVertex();
        EndPrimitive();


    }
}  