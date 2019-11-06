//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "My2DAnimInfo.h"
#include "../JSON/cJSONHelpers.h"
#include "../Sprites/SpriteSheet.h"
#include "../Textures/MaterialDefinition.h"
#include "../Textures/MaterialManager.h"

//====================================================================================================
// My2DAnimationFrame
//====================================================================================================

My2DAnimationFrame::My2DAnimationFrame()
{
    m_pMaterial = nullptr;
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
    if( pMaterial )
    {
        m_pMaterial->AddRef();
    }
}

//====================================================================================================
// My2DAnimation
//====================================================================================================

uint32 My2DAnimation::GetFrameCount()
{
    return m_Frames.Count();
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndex(uint32 frameIndex)
{
    MyAssert( frameIndex < m_Frames.Count() );

    return m_Frames[frameIndex];
}

My2DAnimationFrame* My2DAnimation::GetFrameByIndexClamped(uint32 frameIndex)
{
    MyClamp( frameIndex, (uint32)0, m_Frames.Count()-1 );

    return m_Frames[frameIndex];
}

void My2DAnimation::SetName(const char* name)
{
    if( strlen(name) > (unsigned int)MAX_ANIMATION_NAME_LEN )
        LOGInfo( LOGTag, "Warning: name longer than 32 characters - %s - truncating\n", name );
    strncpy_s( m_Name, MAX_ANIMATION_NAME_LEN+1, name, MAX_ANIMATION_NAME_LEN );
}

//====================================================================================================
// My2DAnimInfo
//====================================================================================================

My2DAnimInfo::My2DAnimInfo()
{
    m_AnimationFileLoaded = false;
    m_pSourceFile = nullptr;
}

My2DAnimInfo::~My2DAnimInfo()
{
    for( unsigned int i=0; i<m_Animations.Count(); i++ )
    {
        for( unsigned int j=0; j<m_Animations[i]->m_Frames.Count(); j++ )
        {
            delete m_Animations[i]->m_Frames[j];
        }

        delete m_Animations[i];
    }

    SAFE_RELEASE( m_pSourceFile );
}

bool My2DAnimInfo::LoadAnimationControlFile(MaterialManager* pMaterialManager)
{
    if( m_pSourceFile == nullptr || m_pSourceFile->GetFileLoadStatus() != FileLoadStatus_Success )
        return false;

    if( m_AnimationFileLoaded == true )
        return false;

    const char* buffer = m_pSourceFile->GetBuffer();

    MyAssert( buffer != nullptr );
    MyAssert( m_Animations.Count() == 0 );

    // If the file doesn't exist, do nothing for now.
    if( buffer == nullptr )
    {
        MyAssert( m_Animations.Count() == 0 );
    }
    else
    {
        cJSON* jRoot = cJSON_Parse( buffer );

        if( jRoot )
        {
            // Allocate and load the correct number of animations.
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

                // Allocate and load the correct number of frames for this animation.
                cJSON* jFrameArray = cJSON_GetObjectItem( jAnim, "Frames" );

                int numframes = cJSON_GetArraySize( jFrameArray );
                pAnim->m_Frames.AllocateObjects( numframes );

                for( int i=0; i<numframes; i++ )
                {
                    cJSON* jFrame = cJSON_GetArrayItem( jFrameArray, i );

                    My2DAnimationFrame* pFrame = MyNew My2DAnimationFrame();
                    pAnim->m_Frames.Add( pFrame );

                    cJSON* jMatName = cJSON_GetObjectItem( jFrame, "Material" );
                    if( jMatName )
                    {
                        // Load and set the necessary material.
                        MaterialDefinition* pMaterial = pMaterialManager->LoadMaterial( jMatName->valuestring );
                        if( pMaterial )
                        {
                            pFrame->SetMaterial( pMaterial );
                            pMaterial->Release();
                        }
                    }

                    cJSONExt_GetFloat( jFrame, "Duration", &pFrame->m_Duration );
                }
            }
        }

        cJSON_Delete( jRoot );
    }

    m_AnimationFileLoaded = true;

    return true;
}

My2DAnimation* My2DAnimInfo::GetAnimationByName(const char* name)
{
    for( uint32 i=0; i<m_Animations.Count(); i++ )
    {
        if( strcmp( m_Animations[i]->GetName(), name ) == 0 )
            return m_Animations[i];
    }

    return 0;
}

uint32 My2DAnimInfo::GetAnimationIndexByName(const char* name)
{
    for( uint32 i=0; i<m_Animations.Count(); i++ )
    {
        if( strcmp( m_Animations[i]->GetName(), name ) == 0 )
            return i;
    }

    LOGInfo( LOGTag, "Warning: GetAnimationIndexByName: Animation not found - %s\n", name );
    return 0;
}

uint32 My2DAnimInfo::GetNumberOfAnimations()
{
    return m_Animations.Count();
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndex(uint32 animIndex)
{
    MyAssert( animIndex < m_Animations.Count() );

    return m_Animations[animIndex];
}

My2DAnimation* My2DAnimInfo::GetAnimationByIndexClamped(uint32 animIndex)
{
    MyClamp( animIndex, (uint32)0, GetNumberOfAnimations()-1 );

    return m_Animations[animIndex];
}

void My2DAnimInfo::SetSourceFile(MyFileObject* pSourceFile)
{
    // This class currently doesn't support changing the source file.
    MyAssert( m_pSourceFile == nullptr );

    if( m_pSourceFile == pSourceFile )
        return;

    SAFE_RELEASE( m_pSourceFile );
    m_pSourceFile = pSourceFile;
    m_pSourceFile->AddRef();
}

#if MYFW_EDITOR
struct SpriteSheetAnimData
{
    std::string name;
    int numFrames;
    int spriteIndexOfFirstFrame;

    SpriteSheetAnimData(std::string n, int nf, int si) { name = n; numFrames = nf; spriteIndexOfFirstFrame = si; }
};

void My2DAnimInfo::LoadFromSpriteSheet(SpriteSheet* pSpriteSheet, float duration)
{
    MyAssert( pSpriteSheet != nullptr );
    MyAssert( pSpriteSheet->GetNumSprites() != 0 );
    MyAssert( m_Animations.Count() == 0 );

    std::vector<SpriteSheetAnimData> animations;

    for( uint32 i=0; i<pSpriteSheet->GetNumSprites(); i++ )
    {
        // Calculate number of animations.  Assumes filenames in json file are stored alphabetically.
        std::string fullname = pSpriteSheet->GetSpriteNameByIndex( i );
        size_t pos = fullname.find_last_of( '.' );
        std::string fullnameWithoutExtension = fullname.substr( 0, pos );
        pos = fullnameWithoutExtension.find_last_of( '_' );
        std::string name = fullnameWithoutExtension.substr( 0, pos );
        std::string number = fullnameWithoutExtension.substr( pos+1 );

        int frame = atoi( number.c_str() );
        if( frame == 0 || frame == 1 )
        {
            animations.push_back( SpriteSheetAnimData( name, 1, i ) );
        }
        else
        {
            animations.back().numFrames = frame;
        }
    }

    uint32 numAnims = (uint32)animations.size();

    m_Animations.AllocateObjects( numAnims );
    for( uint32 i=0; i<numAnims; i++ )
    {
        int numframes = animations[i].numFrames;
        unsigned int spriteIndex = animations[i].spriteIndexOfFirstFrame;

        My2DAnimation* pAnim = MyNew My2DAnimation;
        m_Animations.Add( pAnim );

        pAnim->SetName( animations[i].name.c_str() );
        pAnim->m_Frames.AllocateObjects( numframes );

        for( int frame=0; frame<numframes; frame++ )
        {
            My2DAnimationFrame* pFrame = MyNew My2DAnimationFrame();
            pAnim->m_Frames.Add( pFrame );
            MaterialDefinition* pMaterial = pSpriteSheet->GetSpriteMaterial( spriteIndex + frame );
            pFrame->SetMaterial( pMaterial );
            pFrame->m_Duration = duration;
        }
    }
}

void My2DAnimInfo::SaveAnimationControlFile()
{
    const char* filename = m_pSourceFile->GetFullPath();

    cJSON* jRoot = cJSON_CreateObject();

    cJSON* jAnimArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "Anims", jAnimArray );

    for( unsigned int animIndex=0; animIndex<m_Animations.Count(); animIndex++ )
    {
        My2DAnimation* pAnim = m_Animations[animIndex];

        cJSON* jAnim = cJSON_CreateObject();
        cJSON_AddItemToArray( jAnimArray, jAnim );

        cJSON_AddStringToObject( jAnim, "Name", pAnim->m_Name );

        // Write out frame info.
        cJSON* jFrameArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jAnim, "Frames", jFrameArray );

        for( unsigned int frameindex=0; frameindex<pAnim->m_Frames.Count(); frameindex++ )
        {
            My2DAnimationFrame* pFrame = pAnim->m_Frames[frameindex];

            cJSON* jFrame = cJSON_CreateObject();
            cJSON_AddItemToArray( jFrameArray, jFrame );

            if( pFrame->GetMaterial() )
                cJSON_AddStringToObject( jFrame, "Material", pFrame->GetMaterial()->GetMaterialDescription() );
            cJSON_AddNumberToObject( jFrame, "Duration", pFrame->m_Duration );
        }
    }

    // Dump animation array to disk.
    char* jsonstr = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    FILE* pFile = nullptr;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    if( pFile != nullptr )
    {
        fprintf( pFile, "%s", jsonstr );
        fclose( pFile );
    }
    else
    {
        LOGError( "File failed to open: %s\n", filename );
    }

    cJSONExt_free( jsonstr );
}

void My2DAnimInfo::OnAddAnimationPressed()
{
    if( m_Animations.Count() >= MAX_ANIMATIONS )
        return;

    My2DAnimation* pAnim = MyNew My2DAnimation;

    pAnim->SetName( "New" );
    pAnim->m_Frames.AllocateObjects( MAX_FRAMES_IN_ANIMATION );

    m_Animations.Add( pAnim );
}

void My2DAnimInfo::OnRemoveAnimationPressed(unsigned int animIndex)
{
    My2DAnimation* pAnim = m_Animations[animIndex];
    m_Animations.RemoveIndex_MaintainOrder( animIndex );
    delete pAnim;
}

void My2DAnimInfo::OnRemoveFramePressed(unsigned int animIndex, unsigned int frameIndex)
{
    if( animIndex >= m_Animations.Count() )
        return;
    if( frameIndex >= m_Animations[animIndex]->m_Frames.Count() )
        return;

    My2DAnimationFrame* pFrame = m_Animations[animIndex]->m_Frames.RemoveIndex_MaintainOrder( frameIndex );
    delete pFrame;
}

void My2DAnimInfo::OnAddFramePressed(int animIndex)
{
    if( m_Animations[animIndex]->m_Frames.Count() >= MAX_FRAMES_IN_ANIMATION )
        return;

    My2DAnimationFrame* pFrame = MyNew My2DAnimationFrame;

    pFrame->m_Duration = 0.2f;

    m_Animations[animIndex]->m_Frames.Add( pFrame );
}

void My2DAnimInfo::OnSaveAnimationsPressed()
{
    SaveAnimationControlFile();
}
#endif //MYFW_EDITOR
