//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
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
    m_NumSprites = 0;
    m_pJSONFile = 0;
    m_pMaterial = 0;

    m_SpriteScale = 1;

    m_SubspriteStartX = 0;
    m_SubspriteEndX = 1;
    m_SubspriteStartY = 0;
    m_SubspriteEndY = 1;
}

SpriteSheet::~SpriteSheet()
{
    //this->Remove();
    SAFE_DELETE_ARRAY( m_pSpriteNames );
    SAFE_DELETE_ARRAY( m_pSpriteUVs );

    for( int i=0; i<m_NumSprites; i++ )
    {
        m_pSprites[i]->Release();
    }

    SAFE_DELETE_ARRAY( m_pSprites );
    SAFE_RELEASE( m_pJSONFile );
    SAFE_RELEASE( m_pMaterial );
}

void SpriteSheet::Create(MaterialDefinition* pMaterial)
{
    TextureDefinition* pTextureDef = pMaterial->GetTextureColor();

    MyAssert( pMaterial && pTextureDef && pTextureDef->m_pFile );
    MyAssert( m_pMaterial == 0 );
    if( pMaterial == 0 || pTextureDef == 0 || pTextureDef->m_pFile == 0 )
        return;

    LOGInfo( LOGTag, "SpriteSheet::Load %s\n", pTextureDef->m_pFile->m_FilenameWithoutExtension );

    char jsonpath[MAX_PATH];
    pTextureDef->m_pFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".json", jsonpath, MAX_PATH );
    m_pJSONFile = RequestFile( jsonpath );

    pMaterial->AddRef();
    m_pMaterial = pMaterial;
}

void SpriteSheet::Tick(double TimePassed)
{
    if( m_FullyLoaded )
        return;

    // parse json and create array of sprites.
    if( m_pJSONFile->m_FileLoadStatus == FileLoadStatus_Success && m_pMaterial->GetTextureColor()->m_FullyLoaded )
    {
        cJSON* root = cJSON_Parse( m_pJSONFile->m_pBuffer );

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
                    CreateSprites( numfiles, m_pMaterial );

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
                            cJSON* obj;

                            obj = cJSON_GetObjectItem( file, "filename" );
                            if( obj )
                            {
                                strcpy_s( &m_pSpriteNames[i*64], 64, obj->valuestring );
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

                            //m_pSprites[i].CreateSubsection(
                            //                (float)trimw * m_SpriteScale, (float)trimh * m_SpriteScale,
                            //                (posx+offset)/sheetw, (posx+trimw-offset)/sheetw,
                            //                (posy+offset)/sheeth, (posy+trimh-offset)/sheeth,
                            //                Justify_CenterX|Justify_CenterY,
                            //                m_SubspriteStartX,
                            //                m_SubspriteEndX,
                            //                m_SubspriteStartY,
                            //                m_SubspriteEndY );

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
                        CreateSprites( numframes, m_pMaterial );

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

        cJSON_Delete( root );

        m_FullyLoaded = true;
    }
}

void SpriteSheet::CreateSprites(int numsprites, MaterialDefinition* pMaterial)
{
    m_pSprites = MyNew MySprite*[numsprites];
    for( int i=0; i<numsprites; i++ )
    {
        m_pSprites[i] = MyNew MySprite( false );
        m_pSprites[i]->SetMaterial( pMaterial );
    }
    m_NumSprites = numsprites;
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

MySprite* SpriteSheet::GetSpriteByName(const char* name, ...)
{
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
    m_pJSONFile = sourcesheet->m_pJSONFile;

    m_pMaterial = sourcesheet->m_pMaterial;
    m_pMaterial->AddRef();

    m_SpriteScale = sourcesheet->m_SpriteScale;

    m_SubspriteStartX = sx;
    m_SubspriteEndX = ex;
    m_SubspriteStartY = sy;
    m_SubspriteEndY = ey;

    // actually create the sprites... everything should be loaded, so one tick should be enough.
    Tick( 0 );

    m_pJSONFile = 0;
}
