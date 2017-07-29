//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "SpriteSheet.h"
#include "../Helpers/FileManager.h"

SpriteSheet::SpriteSheet()
{
    m_FullyLoaded = false;

    m_pSpriteNames = 0;
    m_pSpriteUVs = 0;

    m_pSprites = 0;
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

    for( int i=0; i<m_NumSprites; i++ )
    {
        if( m_pSprites && m_pSprites[i] )
        {
            m_pSprites[i]->Release();
        }

        if( m_pMaterialList && m_pMaterialList[i] )
        {
            m_pMaterialList[i]->Release();
        }
    }

    SAFE_DELETE_ARRAY( m_pSprites );
    SAFE_DELETE_ARRAY( m_pMaterialList );
    SAFE_RELEASE( m_pJSONFile );
    SAFE_RELEASE( m_pMaterial );
}

void SpriteSheet::Create(const char* fullpath, ShaderGroup* pShader, int minfilter, int magfilter, bool createsprites, bool creatematerials)
{
    MyAssert( m_pMaterial == 0 );
    MyAssert( m_pSprites == 0 );
    MyAssert( m_pMaterialList == 0 );

    MyFileObject* pFile = RequestFile( fullpath );

    Create( pFile, pShader, minfilter, magfilter, createsprites, creatematerials );
}

void SpriteSheet::Create(MyFileObject* pFile, ShaderGroup* pShader, int minfilter, int magfilter, bool createsprites, bool creatematerials)
{
    MyAssert( m_pMaterial == 0 );
    MyAssert( m_pSprites == 0 );
    MyAssert( m_pMaterialList == 0 );

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

    TextureDefinition* pTextureDef = g_pTextureManager->CreateTexture( pTextureFile, minfilter, magfilter );
    m_pMaterial = g_pMaterialManager->CreateMaterial();
    m_pMaterial->SetTextureColor( pTextureDef );
    m_pMaterial->SetShader( pShader );
    
    pTextureDef->Release();
    pTextureFile->Release();

    m_CreateSprites = createsprites;
    m_CreateMaterials = creatematerials;

    pFile->Release();
}

void SpriteSheet::Tick(double TimePassed)
{
    if( m_FullyLoaded )
        return;

    // parse json and create array of sprites.
    if( m_pJSONFile->GetFileLoadStatus() == FileLoadStatus_Success && m_pMaterial->GetTextureColor()->m_FullyLoaded )
    {
        cJSON* root = cJSON_Parse( m_pJSONFile->GetBuffer() );

        if( root == 0 )
            return;
    
        // handle SpriteTool sprite sheets if we find a "file" tag in json

        //"SpriteTool": 1
        //"file": "BlocksEnemies.png",
        //"width": 1024,
        //"height": 1024,
        //"Files": [{
        //}, {
        cJSON* versionobj = cJSON_GetObjectItem( root, "SpriteTool" );
        if( versionobj )
        {
            float sheetw=0, sheeth=0;
            cJSON* subobj;
            subobj = cJSON_GetObjectItem( root, "width" ); if( subobj ) sheetw = (float)subobj->valueint;
            subobj = cJSON_GetObjectItem( root, "height" ); if( subobj ) sheeth = (float)subobj->valueint;

            MyAssert( sheetw >= 64 && sheeth >= 64 );

            cJSON* files = cJSON_GetObjectItem( root, "Files" );
            if( files )
            {
                int numfiles = cJSON_GetArraySize( files );
                MyAssert( numfiles > 0 );
                if( numfiles > 0 )
                {
                    m_pSpriteNames = MyNew char[numfiles * 64];
                    m_pSpriteUVs = MyNew Vector4[numfiles];

                    m_NumSprites = numfiles;

                    CreateSprites();
                    CreateMaterials( false );

                    for( int i=0; i<numfiles; i++ )
                    {
                        cJSON* file = cJSON_GetArrayItem( files, i );

                        MyAssert( file );
                        if( file )
                        {
                            // "filename": "ChestClosed.png",
                            // "origw": 150,
                            // "origh": 150,
                            // "posx": 0,
                            // "posy": 0,
                            // "trimx": 0,
                            // "trimy": 43,
                            // "trimw": 137,
                            // "trimh": 105
                            cJSON* jFilename = cJSON_GetObjectItem( file, "filename" );
                            if( jFilename )
                            {
                                strcpy_s( &m_pSpriteNames[i*64], 64, jFilename->valuestring );
                            }

                            cJSON* subobj;
                            int origw=0, origh=0, posx=0, posy=0, trimx=0, trimy=0, trimw=0, trimh=0;
                            float offset = 0.5f;
                            subobj = cJSON_GetObjectItem( file, "origw" ); if( subobj ) origw = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "origh" ); if( subobj ) origh = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "posx" ); if( subobj ) posx = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "posy" ); if( subobj ) posy = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "trimx" ); if( subobj ) trimx = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "trimy" ); if( subobj ) trimy = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "trimw" ); if( subobj ) trimw = subobj->valueint;
                            subobj = cJSON_GetObjectItem( file, "trimh" ); if( subobj ) trimh = subobj->valueint;

                            float startu = (posx       + offset)/sheetw;
                            float endu   = (posx+trimw - offset)/sheetw;
                            float startv = (posy       + offset)/sheeth;
                            float endv   = (posy+trimh - offset)/sheeth;

                            float uwidth = endu - startu;
                            float vheight = endv - startv;

                            m_pSpriteUVs[i].x = startu + uwidth * m_SubspriteStartX;
                            m_pSpriteUVs[i].y = endu - uwidth * (1 - m_SubspriteEndX);
                            m_pSpriteUVs[i].z = startv + vheight * m_SubspriteStartY;
                            m_pSpriteUVs[i].w = endv - vheight * (1 - m_SubspriteEndY);

                            if( m_CreateSprites )
                            {
                                MyAssert( m_pSprites[i] );

                                m_pSprites[i]->CreateSubsection( "SpriteSheet",
                                                (float)origw * m_SpriteScale, (float)origh * m_SpriteScale,
                                                (posx-trimx+offset)/sheetw, (posx-trimx+origw-offset)/sheetw,
                                                (posy-trimy+offset)/sheeth, (posy-trimy+origh-offset)/sheeth,
                                                Justify_CenterX|Justify_CenterY,
                                                m_SubspriteStartX > trimx/(float)origw ? m_SubspriteStartX : trimx/(float)origw,
                                                m_SubspriteEndX < (trimx+trimw)/(float)origw ? m_SubspriteEndX : (trimx+trimw)/(float)origw,
                                                m_SubspriteStartY > trimy/(float)origh ? m_SubspriteStartY : trimy/(float)origh,
                                                m_SubspriteEndY < (trimy+trimh)/(float)origh ? m_SubspriteEndY : (trimy+trimh)/(float)origh );
                            }

                            if( m_CreateMaterials )
                            {
                                char matname[MaterialDefinition::MAX_MATERIAL_NAME_LEN+1];
                                sprintf_s( matname, MaterialDefinition::MAX_MATERIAL_NAME_LEN+1, "%s_%s.mymaterial", 
                                           m_pJSONFile->GetFilenameWithoutExtension(), jFilename->valuestring );
                                char fullpath[MAX_PATH];
                                m_pJSONFile->GenerateNewFullPathFilenameInSameFolder( matname, fullpath, MAX_PATH );

#if MYFW_USING_WX
                                // In editor mode, check if the file exists before loading
                                if( g_pFileManager->DoesFileExist( fullpath ) == false )
                                {
                                    // the material might not exist on disk but a load was previously attempted, so find that object
                                    m_pMaterialList[i] = g_pMaterialManager->FindMaterialByFilename( fullpath );
                                    if( m_pMaterialList[i] )
                                        m_pMaterialList[i]->AddRef();

                                    // if the material still isn't there, create a new one along with a file for it.
                                    if( m_pMaterialList[i] == 0 )
                                    {
                                        MyFileObject* pFile = g_pFileManager->CreateFileObject( fullpath );
                                    
                                        m_pMaterialList[i] = g_pMaterialManager->CreateMaterial( pFile );

                                        pFile->Release();
                                    }
                                }
                                else
                                {
                                    // Load the existing material if it exists
                                    m_pMaterialList[i] = g_pMaterialManager->LoadMaterial( fullpath );
                                }
#else
                                // Load the existing material if it exists
                                m_pMaterialList[i] = g_pMaterialManager->LoadMaterial( fullpath );
#endif

                                *m_pMaterialList[i] = *m_pMaterial;

                                MyAssert( m_pMaterialList[i] );

                                Vector4 uvs = m_pSpriteUVs[i];
                                m_pMaterialList[i]->m_UVScale.Set( uvs.y - uvs.x, uvs.w - uvs.z );
                                m_pMaterialList[i]->m_UVOffset.Set( uvs.x, uvs.z );
                            }
                        }
                    }
                }
            }
        }

        // handle TexturePacker sprite sheets if we find a "meta" tag in json

        //"meta": {
        //  "app": "http://www.texturepacker.com",
        //  "version": "1.0",
        //  "image": "CharacterBits.png",
        //  "format": "RGBA4444",
        //  "size": {"w":256,"h":256},
        //  "scale": "1",
        //  "smartupdate": "$TexturePacker:SmartUpdate:ef29b5217bb797448db9c6c73eb8b6cb$"
        //}
        cJSON* metadata = cJSON_GetObjectItem( root, "meta" );
        if( metadata )
        {
            cJSON* sizeobj = cJSON_GetObjectItem( metadata, "size" );
            if( sizeobj )
            {
                float sheetw=0, sheeth=0;
                cJSON* subobj;
                subobj = cJSON_GetObjectItem( sizeobj, "w" ); if( subobj ) sheetw = (float)subobj->valueint;
                subobj = cJSON_GetObjectItem( sizeobj, "h" ); if( subobj ) sheeth = (float)subobj->valueint;

                MyAssert( sheetw >= 64 && sheeth >= 64 );

                cJSON* frames = cJSON_GetObjectItem( root, "frames" );
                if( frames )
                {
                    int numframes = cJSON_GetArraySize( frames );
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
                            cJSON* frame = cJSON_GetArrayItem( frames, i );

                            MyAssert( frame );
                            if( frame )
                            {
                                // "filename": "Circle.png",
                                // "frame": {"x":152,"y":116,"w":36,"h":34},
                                // "rotated": false,
                                // "trimmed": true,
                                // "spriteSourceSize": {"x":110,"y":112,"w":36,"h":34},
                                // "sourceSize": {"w":256,"h":256}
                                cJSON* obj;

                                obj = cJSON_GetObjectItem( frame, "filename" );
                                if( obj )
                                {
                                    strcpy_s( &m_pSpriteNames[i*64], 64, obj->valuestring );
                                }

                                obj = cJSON_GetObjectItem( frame, "frame" );
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
                                        m_pSprites[i]->CreateSubsection( "SpriteSheet",
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

        cJSON_Delete( root );

        m_FullyLoaded = true;
    }
}

void SpriteSheet::CreateSprites()
{
    if( m_CreateSprites == false )
        return;

    MyAssert( m_pSprites == 0 );

    m_pSprites = MyNew MySprite*[m_NumSprites];
    for( int i=0; i<m_NumSprites; i++ )
    {
        m_pSprites[i] = MyNew MySprite( false );
        m_pSprites[i]->SetMaterial( m_pMaterial );
    }
}

void SpriteSheet::CreateMaterials(bool creatematerials)
{
    if( m_CreateMaterials == false )
        return;

    MyAssert( m_pSprites == 0 );

    m_pMaterialList = MyNew MaterialDefinition*[m_NumSprites];

    if( creatematerials )
    {
        for( int i=0; i<m_NumSprites; i++ )
        {
            m_pMaterialList[i] = g_pMaterialManager->CreateMaterial( "SpriteSheet" );
            *m_pMaterialList[i] = *m_pMaterial;
        }
    }
}

int SpriteSheet::GetSpriteIndexByName(const char* name, ...)
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

    for( int i=0; i<m_NumSprites; i++ )
    {
        if( strcmp( &m_pSpriteNames[i*64], buffer ) == 0 )
        {
            return i;
        }
    }
    return -1;
}

MySprite* SpriteSheet::GetSpriteByIndex(int index)
{
    MyAssert( index >= 0 && index < m_NumSprites );

    return m_pSprites[index];
}

MySprite* SpriteSheet::GetSpriteByName(const char* name, ...)
{
    MyAssert( m_pSprites != 0 );
    if( m_pSprites == 0 )
        return 0;

#define MAX_MESSAGE 1024
    char buffer[MAX_MESSAGE];
    va_list arg;
    va_start( arg, name );
    vsnprintf_s( buffer, sizeof(buffer), _TRUNCATE, name, arg );
    va_end(arg);
    buffer[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear

    int index = GetSpriteIndexByName( buffer );
    MyAssert( index != -1 );

    if( index == -1 )
        return 0;
    else
        return m_pSprites[index];
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
