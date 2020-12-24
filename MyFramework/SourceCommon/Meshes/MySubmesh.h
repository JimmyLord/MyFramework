//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MySubmesh_H__
#define __MySubmesh_H__

#include "../Renderers/BaseClasses/Renderer_Enums.h"
#include "MyMesh.h"

class BufferDefinition;
class MaterialDefinition;
class MyLight;
class Shader_Base;
class ShaderGroup;
class TextureDefinition;

class MySubmesh
{
    friend class MyMesh;

protected:
    MaterialDefinition* m_pMaterial;

#if _DEBUG && MYFW_WINDOWS
    bool m_TriggerBreakpointOnNextDraw; // Used by editor to trap the next time this object is drawn.
#endif

public:
    int m_VertexFormat;

    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;

    unsigned int m_NumVertsToDraw;
    unsigned int m_NumIndicesToDraw;
    MyRE::PrimitiveTypes m_PrimitiveType;
    int m_PointSize;

public:
    MySubmesh();
    virtual ~MySubmesh();

public:
    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    void SetMaterial(MaterialDefinition* pMaterial);
    unsigned int GetStride();

    virtual bool SetupShader(Shader_Base* pShader, MyMesh* pMesh, MyMatrix* pMatWorld, Vector3* pCamPos, Vector3* pCamRot, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex);
    virtual void SetupMeshSpecificShaderUniforms(Shader_Base* pShader, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, MyMatrix* matInverseWorld, Vector3* pCamPos, MyLight** pLightPtrs, int numLights, MyMatrix* shadowLightVP);
    virtual void SetupAttributes(Shader_Base* pShader);
    virtual void Draw(MaterialDefinition* pMaterial, MyMesh* pMesh, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, Vector3* pCamPos, Vector3* pCamRot, MyLight** pLightPtrs, int numLights, MyMatrix* shadowLightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride, bool hideFromDrawList);

#if _DEBUG && MYFW_WINDOWS
    void TriggerBreakpointOnNextDraw() { m_TriggerBreakpointOnNextDraw = true; }
#endif
};

#endif //__MySubmesh_H__
