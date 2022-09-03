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
layout (triangle_strip, max_vertices = 4) out;

float topShading1;
float topShading2;
float topShading3;
float topShading4;

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
    if(rot == 0)
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
            CornerC();
            cornerIdx=3;
        }
        else if(cornerIdx == 3)
        {
            CornerD();
            cornerIdx=0;
        }
    }
    else if(rot == 1)
    {
        if(cornerIdx == 0)
        {
            CornerC();
            cornerIdx=1;
        }
        else if(cornerIdx == 1)
        {
            CornerA();
            cornerIdx=2;
        }
        else if(cornerIdx == 2)
        {
            CornerD();
            cornerIdx=3;
        }
        else if(cornerIdx == 3)
        {
            CornerB();
            cornerIdx=0;
        }
    }
    else if(rot == 2)
    {
        if(cornerIdx == 0)
        {
            CornerD();
            cornerIdx=1;
        }
        else if(cornerIdx == 1)
        {
            CornerC();
            cornerIdx=2;
        }
        else if(cornerIdx == 2)
        {
            CornerB();
            cornerIdx=3;
        }
        else if(cornerIdx == 3)
        {
            CornerA();
            cornerIdx=0;
        }
    }
    else if(rot == 3)
    {
        if(cornerIdx == 0)
        {
            CornerB();
            cornerIdx=1;
        }
        else if(cornerIdx == 1)
        {
            CornerD();
            cornerIdx=2;
        }
        else if(cornerIdx == 2)
        {
            CornerA();
            cornerIdx=3;
        }
        else if(cornerIdx == 3)
        {
            CornerC();
            cornerIdx=0;
        }
    }
}


void main() {
    topShading1 = float((gs_in[0].shadeAttr & 15u))/15.0;//darkshading 4 bits
    topShading2 = float((gs_in[0].shadeAttr & (15u << 4))>>4)/15.0;//darkshading 4 bits
    topShading3 = float((gs_in[0].shadeAttr & (15u << 8))>>8)/15.0;//darkshading 4 bits
    topShading4 = float((gs_in[0].shadeAttr & (15u << 12))>>12)/15.0;//darkshading 4 bits

    rot = (gs_in[0].otherAttr & 15u);

    if(gs_in[0].tileId != 255)
    {
        uint tileId = gs_in[0].tileId & 255u;
        tileX = float(tileId & 15u) / 16.0;
        tileY = float((tileId >> 4u) & 15u) / 16.0;

        gl_Position = projection * (localTransform * gl_in[0].gl_Position);
        NextCorner();
        shading = topShading1;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0)));
        NextCorner();
        shading = topShading2;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShading4;
        EmitVertex();

        gl_Position = projection * (localTransform * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0)));
        NextCorner();
        shading = topShading3;
        EmitVertex();

        EndPrimitive();
    }
}  