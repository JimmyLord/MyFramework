//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ParticleRendererInstanced_H__
#define __ParticleRendererInstanced_H__

#if MYFW_USEINSTANCEDPARTICLES

struct ParticleInstanceData
{
    float pos[3];
    float scale;
    unsigned char color[4];

    void Set( float x, float y, float z, float size, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
    {
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
        scale = size;
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
    }
};

class ParticleRendererInstanced : public ParticleRenderer
{
protected:
    GLuint m_ParticleDataBuffer;
    unsigned int m_NumParticlesAllocated;
    ParticleInstanceData* m_pParticleData;

    Vertex_XYZUV_RGBA* m_pParticleQuadVerts; // will be deleted by VBO.

public:
    ParticleRendererInstanced(bool creatematerial);
    virtual ~ParticleRendererInstanced();

    virtual void AllocateVertices(unsigned int numpoints, const char* category);
    virtual void RebuildParticleQuad(MyMatrix* matrot);
    virtual void Reset() { m_ParticleCount = 0; }

    virtual void AddPoint(Vector2 pos, float rot, ColorByte color, float size);
    virtual void AddPoint(Vector3 pos, float rot, ColorByte color, float size);

    //virtual void SetMaterial(MaterialDefinition* pMaterial);

    virtual void Draw(Vector3 campos, Vector3 camrot, MyMatrix* matviewproj);

    //Vertex_PointSprite* GetVerts() { return pVerts; }
};

#endif //MYFW_USEINSTANCEDPARTICLES

#endif //__ParticleRendererInstanced_H__
