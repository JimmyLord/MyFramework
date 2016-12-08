//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

My2DAnimationFrame::My2DAnimationFrame()
{
    m_pMaterial = 0;
}

My2DAnimationFrame::~My2DAnimationFrame()
{
    SAFE_RELEASE( m_pMaterial );
}

void My2DAnimationFrame::SetMaterial(MaterialDefinition* pMaterial)
{
    if( m_pMaterial == pMaterial )
        return;

    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
    m_pMaterial->AddRef();
}

void My2DAnimation::SetName(const char* name)
{
    if( strlen(name) > (unsigned int)MAX_ANIMATION_NAME_LEN )
        LOGInfo( LOGTag, "Warning: name longer than 32 characters - %s - truncating\n", name );
    strncpy_s( m_Name, MAX_ANIMATION_NAME_LEN+1, name, MAX_ANIMATION_NAME_LEN );
}

uint32 My2DAnimation::GetFrameCount()
{
    return m_Frames.Count();
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndex(uint32 frameindex)
{
    MyAssert( frameindex < m_Frames.Count() );

    return m_Frames[frameindex];
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndexClamped(uint32 frameindex)
{
    MyClamp( frameindex, (uint32)0, m_Frames.Count()-1 );

    return m_Frames[frameindex];
}

My2DAnimInfo::My2DAnimInfo()
{
    m_pSourceFile = 0;
}

My2DAnimInfo::~My2DAnimInfo()
{
    for( unsigned int i=0; i<m_Animations.Length(); i++ )
    {
        for( unsigned int j=0; j<m_Animations[i]->m_Frames.Length(); j++ )
        {
            delete m_Animations[i]->m_Frames[j];
        }

        delete m_Animations[i];
    }
    SAFE_RELEASE( m_pSourceFile );
}

uint32 My2DAnimInfo::GetNumberOfAnimations()
{
    return m_Animations.Count();
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndex(uint32 animindex)
{
    MyAssert( animindex < m_Animations.Count() );

    return m_Animations[animindex];
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndexClamped(uint32 animindex)
{
    MyClamp( animindex, (uint32)0, GetNumberOfAnimations()-1 );

    return m_Animations[animindex];
}

#if MYFW_USING_WX
void My2DAnimInfo::SaveAnimationControlFile()
{
    char filename[MAX_PATH];
    m_pSourceFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".my2daniminfo", filename, MAX_PATH );

    cJSON* jRoot = cJSON_CreateObject();

    cJSON* jAnimArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "Anims", jAnimArray );

    for( unsigned int i=0; i<m_Animations.Count(); i++ )
    {
        cJSON* jAnim = cJSON_CreateObject();

        cJSON_AddItemToArray( jAnimArray, jAnim );

        cJSON_AddStringToObject( jAnim, "Name", m_Animations[i]->m_Name );

        // TODO: write out frame info
        MyAssert( false );
    }

    // dump animarray to disk
    char* jsonstr = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    fprintf( pFile, "%s", jsonstr );
    fclose( pFile );

    cJSONExt_free( jsonstr );
}
#endif

void My2DAnimInfo::LoadAnimationControlFile(char* buffer)
{
    MyAssert( buffer != 0 );
    MyAssert( m_Animations.Count() == 0 );

    // if the file doesn't exist, do nothing for now.
    if( buffer == 0 )
    {
        MyAssert( m_Animations.Count() == 0 );
    }
    else
    {
        cJSON* jRoot = cJSON_Parse( buffer );

        if( jRoot )
        {
            cJSON* jAnimArray = cJSON_GetObjectItem( jRoot, "Anims" );

            int numanims = cJSON_GetArraySize( jAnimArray );
            m_Animations.AllocateObjects( numanims );
            for( int i=0; i<numanims; i++ )
            {
                cJSON* jAnim = cJSON_GetArrayItem( jAnimArray, i );

                My2DAnimation* pAnim = MyNew My2DAnimation;
                m_Animations.Add( pAnim );

                cJSON* jName = cJSON_GetObjectItem( jAnim, "Name" );
                if( jName )
                {
                    pAnim->SetName( jName->valuestring );
                }

                cJSON* jFrameArray = cJSON_GetObjectItem( jAnim, "Frames" );

                int numframes = cJSON_GetArraySize( jFrameArray );
                pAnim->m_Frames.AllocateObjects( numframes );
                for( int i=0; i<numframes; i++ )
                {
                    cJSON* jFrame = cJSON_GetArrayItem( jFrameArray, i );

                    My2DAnimationFrame* pFrame = MyNew My2DAnimationFrame();
                    pAnim->m_Frames.Add( pFrame );

                    cJSON* jMatName = cJSON_GetObjectItem( jFrame, "Material" );
                    MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMatName->valuestring );
                    if( pMaterial )
                    {
                        pFrame->SetMaterial( pMaterial );
                        pMaterial->Release();
                    }

                    cJSONExt_GetFloat( jFrame, "Duration", &pFrame->m_Duration );
                }
            }
        }

        cJSON_Delete( jRoot );
    }
}
