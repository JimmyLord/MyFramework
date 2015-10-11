//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "VertexFormatManager.h"

VertexFormatManager* g_pVertexFormatManager = 0;

VertexFormatManager::VertexFormatManager()
{
    m_pDynamicVertexFormatDesc.AllocateObjects( MAX_DYNAMIC_VERTEX_FORMATS );
}

VertexFormatManager::~VertexFormatManager()
{
    while( m_pDynamicVertexFormatDesc.Count() )
        delete m_pDynamicVertexFormatDesc.RemoveIndex( 0 );
}

VertexFormat_Dynamic_Desc* VertexFormatManager::FindDynamicVertexFormat(int numuvs, bool normals, bool tangents, bool bitangents, bool color, int boneinfluences)
{
    for( unsigned int i=0; i<m_pDynamicVertexFormatDesc.Count(); i++ )
    {
        VertexFormat_Dynamic_Desc* pDesc = m_pDynamicVertexFormatDesc[i];

        if( pDesc->num_bone_influences != boneinfluences )
            continue;

        if( pDesc->num_uv_channels != numuvs )
            continue;

        if( normals && pDesc->offset_normal == 0 )
            continue;

        if( color && pDesc->offset_color == 0 )
            continue;

        if( tangents && pDesc->offset_tangent == 0 )
            continue;

        if( bitangents && pDesc->offset_bitangent == 0 )
            continue;

        return pDesc;
    }

    return 0;
}

VertexFormat_Dynamic_Desc* VertexFormatManager::GetDynamicVertexFormat(int numuvs, bool normals, bool tangents, bool bitangents, bool color, int boneinfluences)
{
    VertexFormat_Dynamic_Desc* pDesc = FindDynamicVertexFormat( numuvs, normals, tangents, bitangents, color, boneinfluences );

    // If we found a vertex format that matches, use it.
    if( pDesc )
        return pDesc;

    if( m_pDynamicVertexFormatDesc.Count() >= MAX_DYNAMIC_VERTEX_FORMATS )
    {
        MyAssert( false );
        return 0; // pretty much guananteeing a crash on the other end!
    }

    pDesc = MyNew VertexFormat_Dynamic_Desc;
    memset( pDesc, 0, sizeof( VertexFormat_Dynamic_Desc ) );

    m_pDynamicVertexFormatDesc.Add( pDesc );

    //int numuvs, bool normals, bool tangents, bool bitangents, bool color, int boneinfluences

    pDesc->num_bone_influences = boneinfluences;
    pDesc->num_uv_channels = numuvs;
    
    int offset = 0;

    pDesc->offset_pos = offset;             offset += sizeof(float) * 3;
    for( int i=0; i<numuvs; i++ )
    {
        pDesc->offset_uv[0] = offset;       offset += sizeof(float) * 2;
    }
    if( normals )
    {
        pDesc->offset_normal = offset;      offset += sizeof(float) * 3;
    }
    if( color )
    {
        pDesc->offset_color = offset;       offset += sizeof(unsigned char) * 4;
    }
    if( tangents )
    {
        pDesc->offset_tangent = offset;     offset += sizeof(float) * 3;
    }
    if( bitangents )
    {
        pDesc->offset_bitangent = offset;   offset += sizeof(float) * 3;
    }
    if( boneinfluences > 0 )
    {
        pDesc->offset_boneindex = offset;   offset += sizeof(unsigned char) * boneinfluences;
        pDesc->offset_boneweight = offset;  offset += sizeof(float) * boneinfluences;
    }

    pDesc->stride = offset;

    return pDesc;

    //if( numuvs == 1 && normals && color == false )
    //{
    //    if( boneinfluences == 0 )
    //        return VertexFormat_XYZUVNorm;
    //    if( boneinfluences == 1 )
    //        return VertexFormat_XYZUVNorm_1Bones;
    //    if( boneinfluences == 2 )
    //        return VertexFormat_XYZUVNorm_2Bones;
    //    if( boneinfluences == 3 )
    //        return VertexFormat_XYZUVNorm_3Bones;
    //    if( boneinfluences == 4 )
    //        return VertexFormat_XYZUVNorm_4Bones;
    //}
    //else if( numuvs == 1 && normals && color )
    //{
    //    if( boneinfluences == 0 )
    //        return VertexFormat_XYZUVNorm_RGBA;
    //    if( boneinfluences == 1 )
    //        return VertexFormat_XYZUVNorm_RGBA_1Bones;
    //    if( boneinfluences == 2 )
    //        return VertexFormat_XYZUVNorm_RGBA_2Bones;
    //    if( boneinfluences == 3 )
    //        return VertexFormat_XYZUVNorm_RGBA_3Bones;
    //    if( boneinfluences == 4 )
    //        return VertexFormat_XYZUVNorm_RGBA_4Bones;
    //}

    // ADDING_NEW_VertexFormat

    //MyAssert( false );
    //return VertexFormat_None;
}
