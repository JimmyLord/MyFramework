//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MySprite.h"
#include "SpriteSheet.h"
#include "../Helpers/FileManager.h"
#include "../JSON/cJSONHelpers.h"
#include "../Textures/MaterialDefinition.h"
#include "../Textures/MaterialManager.h"
#include "../Textures/TextureDefinition.h"
#include "../Textures/TextureManager.h"

SpriteSheet::SpriteSheet()
{
    m_FullyLoaded = false;

    m_pSpriteNames = 0;
    m_pSpriteUVs = 0;

    m_ppSpriteArray = 0;
    m_pMaterialList = 0;

    m_NumSprites = 0;
    m_pJSONFile = 0;
    m_pMaterial = 0;

    m_SpriteScale = 1;

    m_SubspriteStartX = 0;
    m_SubspriteEndX = 1;
    m_SubspriteStartY = 0;
    m_SubspriteEndY = 1;

    m_CreateSprites = false;
    m_CreateMaterials = false;
}

SpriteSheet::~SpriteSheet()
{
    //this->Remove();
    SAFE_DELETE_ARRAY( m_pSpriteNames );
    SAFE_DELETE_ARRAY( m_pSpriteUVs );

    for( uint32 i=0; i<m_NumSprites; i++ )
    {
        if( m_ppSpriteArray && m_ppSpriteArray[i] )
        {
            m_ppSpriteArray[i]->Release();
        }

        if( m_pMaterialList && m_pMaterialList[i] )
        {
            m_pMaterialList[i]->Release();
        }
    }

    SAFE_DELETE_ARRAY( m_ppSpriteArray );
    SAFE_DELETE_ARRAY( m_pMaterialList );
    SAFE_RELEASE( m_pJSONFile );
    SAFE_RELEASE( m_pMaterial );
}

void SpriteSheet::Create(TextureManager* pTextureManager, const char* fullpath, ShaderGroup* pShader, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, bool createSprites, bool createMaterials)
{
    MyAssert( m_pMaterial == 0 );
    MyAssert( m_ppSpriteArray == 0 );
    MyAssert( m_pMaterialList == 0 );

    MyFileObject* pFile = RequestFile( fullpath );

    Create( pTextureManager, pFile, pShader, minFilter, magFilter, createSprites, createMaterials );
}

void SpriteSheet::Create(TextureManager* pTextureManager, MyFileObject* pFile, ShaderGroup* pShader, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, bool createSprites, bool createMaterials)
{
    MyAssert( pTextureManager != nullptr );
    MyAssert( m_pMaterial == nullptr );
    MyAssert( m_ppSpriteArray == nullptr );
    MyAssert( m_pMaterialList == nullptr );

    LOGInfo( LOGTag, "SpriteSheet::Load %s\n", pFile->GetFullPath() );

    MyFileObject* pJSONFile = 0;
    MyFileObject* pTextureFile = 0;

    if( strcmp( pFile->GetExtensionWithDot(), ".json" ) == 0 ||
        strcmp( pFile->GetExtensionWithDot(), ".myspritesheet" ) == 0 )
    {
        pJSONFile = pFile;
        pJSONFile->AddRef();

        char otherpath[MAX_PATH];
        pFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".png", otherpath, MAX_PATH );
        pTextureFile = RequestFile( otherpath );
    }
    else if( strcmp( pFile->GetExtensionWithDot(), ".png" ) == 0 )
    {
        pTextureFile = pFile;
        pTextureFile->AddRef();

        char otherpath[MAX_PATH];
        pFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".json", otherpath, MAX_PATH );
        pJSONFile = RequestFile( otherpath );
    }
    else
    {
        MyAssert( false );
    }

    m_pJSONFile = pJSONFile;

    TextureDefinition* pTextureDef = pTextureManager->CreateTexture( pTextureFile, minFilter, magFilter );
    m_pMaterial = g_pMaterialManager->CreateMaterial();
    m_pMaterial->SetTextureColor( pTextureDef );
    m_pMaterial->SetShader( pShader );
#if MYFW_EDITOR
    m_pMaterial->SetPreviewType( MaterialDefinition::PreviewType_Flat );
#endif //MYFW_EDITOR
    
    pTextureDef->Release();
    pTextureFile->Release();

    m_CreateSprites = createSprites;
    m_CreateMaterials = createMaterials;

    pFile->Release();

    if( pFile->IsFinishedLoading() )
    {
        FinishLoadingFile();
    }
}

void SpriteSheet::FinishLoadingFile()
{
    if( m_FullyLoaded )
    {
        return;
    }

    // Parse json and create array of sprites.
    if( m_pJSONFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        cJSON* jRoot = cJSON_Parse( m_pJSONFile->GetBuffer() );

        if( jRoot == 0 )
            return;
    
        // Handle SpriteTool sprite sheets if we find a "SpriteTool" tag in the json file.

        //"SpriteTool": 1                   //"SpriteTool": 2
        //"file": "Sheet.png",              //"Texture": "Sheet.png",
        //"width": 1024,                    //"Width": 1024,
        //"height": 1024,                   //"Height": 1024,
        //"Files": [{                       //"Sprites": [{
		//  "filename": "Sprite.png",       //  "Name": "Sprite.png",
		//  "origw": 0,                     //  "X": 0,
		//  "origh": 0,                     //  "Y": 0,
		//  "posx": 100,                    //  "W": 100,
		//  "posy": 100,                    //  "H": 100,
		//  "trimx": 0,                     //  "TrimX": 0,
		//  "trimy": 0,                     //  "TrimY": 0,
		//  "trimw": 100,                   //  "TrimW": 100,
		//  "trimh": 100                    //  "TrimH": 100
        //}, {                              //}, {
        cJSON* jVersion = cJSON_GetObjectItem( jRoot, "SpriteTool" );
        if( jVersion )
        {
            // Keywords to handle both SpriteTool formats.
            const char* jsonKey_SpriteArray = "Sprites";
            const char* jsonKey_Sprite_Name = "Name";
            const char* jsonKey_Sprite_X = "X";
            const char* jsonKey_Sprite_Y = "Y";
            const char* jsonKey_Sprite_W = "W";
            const char* jsonKey_Sprite_H = "H";

            if( jVersion->valueint == 1 )
            {
                jsonKey_SpriteArray = "Files";
                jsonKey_Sprite_Name = "filename";
                jsonKey_Sprite_X = "posx";
                jsonKey_Sprite_Y = "posy";
                jsonKey_Sprite_W = "origw";
                jsonKey_Sprite_H = "origh";
            }

            int sheetW=0, sheetH=0;
            cJSONExt_GetInt( jRoot, "Width", &sheetW );
            cJSONExt_GetInt( jRoot, "Height", &sheetH );

            MyAssert( sheetW >= 64 && sheetH >= 64 );

            cJSON* jSpriteArray = cJSON_GetObjectItem( jRoot, jsonKey_SpriteArray );
            if( jSpriteArray )
            {
                int numSprites = cJSON_GetArraySize( jSpriteArray );
                MyAssert( numSprites > 0 );
                if( numSprites > 0 )
                {
                    m_pSpriteNames = MyNew char[numSprites * 64];
                    m_pSpriteUVs = MyNew Vector4[numSprites];

                    m_NumSprites = numSprites;

                    CreateSprites();
                    CreateMaterials( false );

                    for( int i=0; i<numSprites; i++ )
                    {
                        cJSON* jSprite = cJSON_GetArrayItem( jSpriteArray, i );

                        MyAssert( jSprite );
                        if( jSprite )
                        {
                            cJSON* jSpriteName = cJSON_GetObjectItem( jSprite, jsonKey_Sprite_Name );
                            if( jSpriteName )
                            {
                                strcpy_s( &m_pSpriteNames[i*64], 64, jSpriteName->valuestring );
                            }

                            int originalW=0, originalH=0, posX=0, posY=0, trimX=0, trimY=0, trimW=0, trimH=0;
                            float offset = 0.5f;
                            cJSONExt_GetInt( jSprite, jsonKey_Sprite_W, &originalW );
                            cJSONExt_GetInt( jSprite, jsonKey_Sprite_H, &originalH );
                            cJSONExt_GetInt( jSprite, jsonKey_Sprite_X, &posX );
                            cJSONExt_GetInt( jSprite, jsonKey_Sprite_Y, &posY );
                            cJSONExt_GetInt( jSprite, "TrimX", &trimX );
                            cJSONExt_GetInt( jSprite, "TrimY", &trimY );
                            cJSONExt_GetInt( jSprite, "TrimW", &trimW );
                            cJSONExt_GetInt( jSprite, "TrimH", &trimH );

                            float startU = (posX       + offset)/sheetW;
                            float endU   = (posX+trimW - offset)/sheetW;
                            float startV = (posY       + offset)/sheetH;
                            float endV   = (posY+trimH - offset)/sheetH;

                            float uWidth = endU - startU;
                            float vHeight = endV - startV;

                            m_pSpriteUVs[i].x = startU + uWidth * m_SubspriteStartX;
                            m_pSpriteUVs[i].y = endU - uWidth * (1 - m_SubspriteEndX);
                            m_pSpriteUVs[i].z = startV + vHeight * m_SubspriteStartY;
                            m_pSpriteUVs[i].w = endV - vHeight * (1 - m_SubspriteEndY);

                            if( m_CreateSprites )
                            {
                                MyAssert( m_ppSpriteArray[i] );

                                m_ppSpriteArray[i]->CreateSubsection( "SpriteSheet",
                                                (float)originalW * m_SpriteScale, (float)originalH * m_SpriteScale,
                                                (posX-trimX+offset)/sheetW, (posX-trimX+originalW-offset)/sheetW,
                                                (posY-trimY+offset)/sheetH, (posY-trimY+originalH-offset)/sheetH,
                                                Justify_CenterX|Justify_CenterY,
                                                m_SubspriteStartX > trimX/(float)originalW ? m_SubspriteStartX : trimX/(float)originalW,
                                                m_SubspriteEndX < (trimX+trimW)/(float)originalW ? m_SubspriteEndX : (trimX+trimW)/(float)originalW,
                                                m_SubspriteStartY > trimY/(float)originalH ? m_SubspriteStartY : trimY/(float)originalH,
                                                m_SubspriteEndY < (trimY+trimH)/(float)originalH ? m_SubspriteEndY : (trimY+trimH)/(float)originalH );
                            }

                            if( m_CreateMaterials )
                            {
                                char matname[MaterialDefinition::MAX_MATERIAL_NAME_LEN+1];
                                sprintf_s( matname, MaterialDefinition::MAX_MATERIAL_NAME_LEN+1, "%s_%s.mymaterial", 
                                           m_pJSONFile->GetFilenameWithoutExtension(), jSpriteName->valuestring );
                                char fullpath[MAX_PATH];
                                m_pJSONFile->GenerateNewFullPathFilenameInSameFolder( matname, fullpath, MAX_PATH );

#if MYFW_EDITOR
                                // In editor mode, check if the file exists before loading.
                                if( g_pFileManager->DoesFileExist( fullpath ) == false )
                                {
                                    // The material might not exist on disk but a load was previously attempted, so find that object.
                                    m_pMaterialList[i] = g_pMaterialManager->FindMaterialByFilename( fullpath );
                                    if( m_pMaterialList[i] )
                                        m_pMaterialList[i]->AddRef();

                                    // If the material still isn't there, create a new one along with a file for it.
                                    if( m_pMaterialList[i] == 0 )
                                    {
                                        MyFileObject* pFile = g_pFileManager->CreateFileObject( fullpath );
                                    
                                        m_pMaterialList[i] = g_pMaterialManager->CreateMaterial( pFile );

                                        pFile->Release();
                                    }
                                }
                                else
#endif
                                {
                                    // Load the existing material if it exists.
                                    m_pMaterialList[i] = g_pMaterialManager->LoadMaterial( fullpath );
                                }

                                MyAssert( m_pMaterialList[i] );

                                // Copy all base material properties into material for each sprite. This doesn't affect file info stored in material.
                                // TODO: Don't use the equal operator for this...
                                *m_pMaterialList[i] = *m_pMaterial;

                                // Update uv offset and scale for each sprite.
                                Vector4 uvs = m_pSpriteUVs[i];
                                m_pMaterialList[i]->m_UVScale.Set( uvs.y - uvs.x, uvs.w - uvs.z );
                                m_pMaterialList[i]->m_UVOffset.Set( uvs.x, uvs.z );
                            }
                        }
                    }
                }
            }
        }

        // Partially handle TexturePacker sprite sheets if we find a "meta" tag in json.

        //"meta": {
        //  "app": "http://www.texturepacker.com",
        //  "version": "1.0",
        //  "image": "CharacterBits.png",
        //  "format": "RGBA4444",
        //  "size": {"w":256,"h":256},
        //  "scale": "1",
        //  "smartupdate": "$TexturePacker:SmartUpdate:ef29b5217bb797448db9c6c73eb8b6cb$"
        //}
        cJSON* jMetaData = cJSON_GetObjectItem( jRoot, "meta" );
        if( jMetaData )
        {
            cJSON* jSize = cJSON_GetObjectItem( jMetaData, "size" );
            if( jSize )
            {
                float sheetw=0, sheeth=0;
                cJSON* subobj;
                subobj = cJSON_GetObjectItem( jSize, "w" ); if( subobj ) sheetw = (float)subobj->valueint;
                subobj = cJSON_GetObjectItem( jSize, "h" ); if( subobj ) sheeth = (float)subobj->valueint;

                MyAssert( sheetw >= 64 && sheeth >= 64 );

                cJSON* jFramesArray = cJSON_GetObjectItem( jRoot, "frames" );
                if( jFramesArray )
                {
                    int numframes = cJSON_GetArraySize( jFramesArray );
                    MyAssert( numframes > 0 );
                    if( numframes > 0 )
                    {
                        m_pSpriteNames = MyNew char[numframes * 64];
                        m_pSpriteUVs = MyNew Vector4[numframes];

                        m_NumSprites = numframes;

                        CreateSprites();
                        CreateMaterials( true );

                        for( int i=0; i<numframes; i++ )
                        {
                            cJSON* jFrame = cJSON_GetArrayItem( jFramesArray, i );

                            MyAssert( jFrame );
                            if( jFrame )
                            {
                                // "filename": "Circle.png",
                                // "frame": {"x":152,"y":116,"w":36,"h":34},
                                // "rotated": false,
                                // "trimmed": true,
                                // "spriteSourceSize": {"x":110,"y":112,"w":36,"h":34},
                                // "sourceSize": {"w":256,"h":256}
                                cJSON* obj;

                                obj = cJSON_GetObjectItem( jFrame, "filename" );
                                if( obj )
                                {
                                    strcpy_s( &m_pSpriteNames[i*64], 64, obj->valuestring );
                                }

                                obj = cJSON_GetObjectItem( jFrame, "frame" );
                                if( obj )
                                {
                                    cJSON* subobj;
                                    int x=0, y=0, w=0, h=0;
                                    float offset = 0.5f;
                                    subobj = cJSON_GetObjectItem( obj, "x" ); if( subobj ) x = subobj->valueint;
                                    subobj = cJSON_GetObjectItem( obj, "y" ); if( subobj ) y = subobj->valueint;
                                    subobj = cJSON_GetObjectItem( obj, "w" ); if( subobj ) w = subobj->valueint;
                                    subobj = cJSON_GetObjectItem( obj, "h" ); if( subobj ) h = subobj->valueint;

                                    float startu = (x+offset)/sheetw;
                                    float endu   = (x+w-offset)/sheetw;
                                    float startv = (y+offset)/sheeth;
                                    float endv   = (y+h-offset)/sheeth;

                                    float uwidth = endu - startu;
                                    float vheight = endv - startv;

                                    m_pSpriteUVs[i].x = startu + uwidth * m_SubspriteStartX;
                                    m_pSpriteUVs[i].y = endu - uwidth * (1 - m_SubspriteEndX);
                                    m_pSpriteUVs[i].z = startv + vheight * m_SubspriteStartY;
                                    m_pSpriteUVs[i].w = endv - vheight * (1 - m_SubspriteEndY);

                                    if( m_CreateSprites )
                                    {
                                        m_ppSpriteArray[i]->CreateSubsection( "SpriteSheet",
                                                        (float)w * m_SpriteScale, (float)h * m_SpriteScale,
                                                        (x+offset)/sheetw, (x+w-offset)/sheetw,
                                                        (y+offset)/sheeth, (y+h-offset)/sheeth,
                                                        Justify_CenterX|Justify_CenterY,
                                                        m_SubspriteStartX, m_SubspriteEndX, 
                                                        m_SubspriteStartY, m_SubspriteEndY );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        cJSON_Delete( jRoot );

        m_FullyLoaded = true;
    }
}

void SpriteSheet::Tick(float deltaTime)
{
    if( m_FullyLoaded )
    {
        return;
    }

    if( m_pJSONFile->IsFinishedLoading() )
    {
        FinishLoadingFile();
    }
}

void SpriteSheet::CreateSprites()
{
    if( m_CreateSprites == false )
        return;

    MyAssert( m_ppSpriteArray == 0 );

    m_ppSpriteArray = MyNew MySprite*[m_NumSprites];
    for( uint32 i=0; i<m_NumSprites; i++ )
    {
        m_ppSpriteArray[i] = MyNew MySprite( false );
        m_ppSpriteArray[i]->SetMaterial( m_pMaterial );
    }
}

void SpriteSheet::CreateMaterials(bool creatematerials)
{
    if( m_CreateMaterials == false )
        return;

    MyAssert( m_pMaterialList == 0 );

    m_pMaterialList = MyNew MaterialDefinition*[m_NumSprites];

    if( creatematerials )
    {
        for( uint32 i=0; i<m_NumSprites; i++ )
        {
            m_pMaterialList[i] = g_pMaterialManager->CreateMaterial( "SpriteSheet" );
            *m_pMaterialList[i] = *m_pMaterial;
        }
    }
}

uint32 SpriteSheet::GetSpriteIndexByName(const char* name, ...)
{
    MyAssert( name );
    if( name == 0 )
        return -1;

#define MAX_MESSAGE 1024
    char buffer[MAX_MESSAGE];
    va_list arg;
    va_start( arg, name );
    vsnprintf_s( buffer, sizeof(buffer), _TRUNCATE, name, arg );
    va_end(arg);
    buffer[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear

    for( uint32 i=0; i<m_NumSprites; i++ )
    {
        if( strcmp( &m_pSpriteNames[i*64], buffer ) == 0 )
        {
            return i;
        }
    }
    return -1;
}

MySprite* SpriteSheet::GetSpriteByIndex(uint32 index)
{
    MyAssert( index < m_NumSprites );

    return m_ppSpriteArray[index];
}

MySprite* SpriteSheet::GetSpriteByName(const char* name, ...)
{
    MyAssert( m_ppSpriteArray != 0 );
    if( m_ppSpriteArray == 0 )
        return 0;

#define MAX_MESSAGE 1024
    char buffer[MAX_MESSAGE];
    va_list arg;
    va_start( arg, name );
    vsnprintf_s( buffer, sizeof(buffer), _TRUNCATE, name, arg );
    va_end(arg);
    buffer[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear

    uint32 index = GetSpriteIndexByName( buffer );
    MyAssert( index != -1 );

    if( index == -1 )
        return 0;
    else
        return m_ppSpriteArray[index];
}

void SpriteSheet::CreateNewSpritesFromOtherSheet(SpriteSheet* sourcesheet, float sx, float ex, float sy, float ey)
{
    if( m_pJSONFile )
        m_pJSONFile->Release();
    m_pJSONFile = sourcesheet->m_pJSONFile;
    m_pJSONFile->AddRef();

    if( m_pMaterial )
        m_pMaterial->Release();
    m_pMaterial = sourcesheet->m_pMaterial;
    m_pMaterial->AddRef();

    m_CreateSprites = sourcesheet->m_CreateSprites;
    m_CreateMaterials = sourcesheet->m_CreateMaterials;

    m_SpriteScale = sourcesheet->m_SpriteScale;

    m_SubspriteStartX = sx;
    m_SubspriteEndX = ex;
    m_SubspriteStartY = sy;
    m_SubspriteEndY = ey;

    // actually create the sprites... everything should be loaded, so one tick should be enough.
    Tick( 0 );
}

const char* SpriteSheet::GetSpriteNameByIndex(uint32 index)
{
    return &m_pSpriteNames[index*64];
}
