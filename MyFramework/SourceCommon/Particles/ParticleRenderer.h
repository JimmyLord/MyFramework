//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ParticleRenderer_H__
#define __ParticleRenderer_H__

class ParticleRenderer : public MySubmesh
{
protected:
    unsigned int m_NumVertsAllocated;

    //MaterialDefinition* m_pMaterial;
    //BufferDefinition* m_pVertexBuffer;
    //BufferDefinition* m_pIndexBuffer;

    Vertex_XYZUV m_pRotatedQuadVerts[4];
    
    unsigned int m_ParticleCount;

public:
    bool m_Additive;

    bool m_ScalePosToScreenSize;
    float m_2DCameraZoom;

    float m_ScreenOffsetX;
    float m_ScreenOffsetY;
    float m_GameWidth;
    float m_GameHeight;
    float m_DeviceWidth;
    float m_DeviceHeight;
    //MyMatrix m_Position;

public:
    ParticleRenderer(bool creatematerial);
    virtual ~ParticleRenderer();

    virtual void AllocateVertices(unsigned int numpoints, const char* category);
    virtual void RebuildParticleQuad(MyMatrix* matrot);
    virtual void Reset() { m_ParticleCount = 0; }

    virtual void AddPoint(Vector2 pos, float rot, ColorByte color, float size);
    virtual void AddPoint(Vector3 pos, float rot, ColorByte color, float size);

    //virtual void SetMaterial(MaterialDefinition* pMaterial);

    virtual void Draw(MyMesh* pMesh, MyMatrix* matworld, MyMatrix* matviewproj, Vector3* campos, Vector3* camrot, MyLight* lights, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride);
    virtual void DrawParticles(Vector3 campos, Vector3 camrot, MyMatrix* matviewproj, ShaderGroup* pShaderOverride);

    //Vertex_PointSprite* GetVerts() { return pVerts; }
};

#endif //__ParticleRenderer_H__
