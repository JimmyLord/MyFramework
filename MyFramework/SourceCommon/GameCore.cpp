//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "GameCore.h"

GameCore* g_pGameCore = 0;

GameCore::GameCore()
{
    g_pGameCore = this;

    m_OneTimeInitWasCalled = false;

    m_GLSurfaceIsValid = false;

    m_pSoundPlayer = 0;
#if MYFW_BLACKBERRY
    m_pMediaPlayer = 0;
#endif

#if MYFW_IOS
    g_pIAPManager = MyNew IAPManager;
#endif

    m_HasFocus = true;
    m_Settled = false;

    m_KeyboardOpenRequested = false;
    m_KeyboardCloseRequested = false;

    m_LastInputMethodUsed = InputMethod_Touch;

    for( int i=0; i<GCBI_NumButtons; i++ )
        m_ButtonsHeld[i] = false;

    for( int i=0; i<255; i++ )
    {
        m_KeysHeld[i] = false;
        m_KeyMappingToButtons[i] = GCBI_NumButtons;
    }
}

GameCore::~GameCore()
{
    LOGInfo( LOGTag, "GameCore::~GameCore()\n" );

    g_FontManager.FreeAllFonts();

    SAFE_DELETE( g_pMaterialManager );
    SAFE_DELETE( g_pTextureManager );
    SAFE_DELETE( g_pBufferManager );
    SAFE_DELETE( g_pMeshManager );
    SAFE_DELETE( g_pLightManager );
    SAFE_DELETE( g_pShaderManager );
    SAFE_DELETE( g_pShaderGroupManager );
    SAFE_DELETE( m_pSoundPlayer );
    SAFE_DELETE( g_pGameServiceManager );
    SAFE_DELETE( g_pVertexFormatManager );

    SAFE_DELETE( g_pFileManager ); // will assert if all files aren't free, so delete last.

#if MYFW_BLACKBERRY
    SAFE_DELETE( m_pMediaPlayer );
#endif

#if MYFW_IOS
    SAFE_DELETE( g_pIAPManager );
#endif
}

void GameCore::InitializeManagers()
{
    if( g_pFileManager == 0 )
        g_pFileManager = MyNew FileManager;
    if( g_pTextureManager == 0 )
        g_pTextureManager = MyNew TextureManager;
    if( g_pMaterialManager == 0 )
        g_pMaterialManager = MyNew MaterialManager;
    if( g_pBufferManager == 0 )
        g_pBufferManager = MyNew BufferManager;
    if( g_pMeshManager == 0 )
        g_pMeshManager = MyNew MeshManager;
    if( g_pLightManager == 0 )
        g_pLightManager = MyNew LightManager;
    if( g_pShaderManager == 0 )
        g_pShaderManager = MyNew ShaderManager;
    if( g_pShaderGroupManager == 0 )
        g_pShaderGroupManager = MyNew ShaderGroupManager;
    if( g_pGameServiceManager == 0 )
        g_pGameServiceManager = MyNew GameServiceManager;
    if( g_pVertexFormatManager == 0 )
        g_pVertexFormatManager = MyNew VertexFormatManager;
}

void GameCore::OneTimeInit()
{
    if( m_OneTimeInitWasCalled )
        return;

    m_OneTimeInitWasCalled = true;

    // base seed on realtime if available.  don't fmod on emscripten, nanosecs aren't accurate enough.
#if MYFW_EMSCRIPTEN
    double time = MyTime_GetSystemTime(true);
    unsigned int seed = (unsigned int)time;
#else
    double time = MyTime_GetSystemTime(true) * 10000000;
    unsigned int seed = (unsigned int)fmod(time,100000);
#endif
    LOGInfo( LOGTag, "Seeding random: %d\n", seed );
    srand( seed );

    InitializeManagers();

    m_pSoundPlayer = MyNew SoundPlayer;

#if MYFW_BLACKBERRY
    m_pMediaPlayer = MyNew MediaPlayer;
#endif

    m_TimeSinceGameStarted = 0;
}

void GameCore::OnPrepareToDie()
{
}

bool GameCore::IsReadyToRender()
{
    return false;
}

double GameCore::Tick(double TimePassed)
{
    m_TimePassedUnpausedLastFrame = (float)TimePassed;

    m_TimeSinceGameStarted += (float)TimePassed;

    // generate held messages for keys and buttons.
    {
        for( int i=0; i<255; i++ )
        {
            if( m_KeysHeld[i] )
                g_pGameCore->OnKey( GCBA_Held, i, i );
        }

        for( int i=0; i<GCBI_NumButtons; i++ )
        {
            if( m_ButtonsHeld[i] == true )
                OnButtons( GCBA_Held, (GameCoreButtonIDs)i );
        }
    }

    //if( m_GLSurfaceIsValid == false )
    //    return;

    g_pFileManager->Tick();
    g_pTextureManager->Tick();
    g_pMaterialManager->Tick();
    g_pBufferManager->Tick();
    g_FontManager.Tick();
#if MYFW_BLACKBERRY10
    if( m_WindowWidth == m_WindowHeight )
        g_pGameServiceManager->Tick( "Q10" );
    else
        g_pGameServiceManager->Tick( "Z10" );
#else
    g_pGameServiceManager->Tick();
#endif //MYFW_BLACKBERRY10

    if( m_KeyboardOpenRequested )
    {
        m_KeyboardOpenRequested = false;
        ShowKeyboard( true );
    }
    if( m_KeyboardCloseRequested )
    {
        m_KeyboardCloseRequested = false;
        ShowKeyboard( false );
    }

#if MYFW_USING_WX
    if( g_pPanelWatch->m_NeedsRefresh )
    {
        g_pPanelWatch->m_RefreshCallbackFunc( g_pPanelWatch->m_RefreshCallbackObject );
        g_pPanelWatch->m_NeedsRefresh = false;
    }
#endif

    return TimePassed;
}

void GameCore::OnFocusGained()
{
    m_HasFocus = true;

    if( m_pSoundPlayer )
        m_pSoundPlayer->OnFocusGained();
}

void GameCore::OnFocusLost()
{
    m_HasFocus = false;

    for( int i=0; i<GCBI_NumButtons; i++ )
    {
        m_ButtonsHeld[i] = false;
    }

    if( m_pSoundPlayer )
        m_pSoundPlayer->OnFocusLost();
}

void GameCore::OnSurfaceCreated()
{
    m_GLSurfaceIsValid = true;

    LOGInfo( LOGTag, "onSurfaceCreated()\n" );

    //checkGlError( "Before Invalidated Shaders and Textures" );
    //if( g_pShaderManager )
    //    g_pShaderManager->InvalidateAllShaders( false );
    //if( g_pTextureManager )
    //    g_pTextureManager->InvalidateAllTextures( false );
    //if( g_pBufferManager )
    //    g_pBufferManager->InvalidateAllBuffers( false );
    //checkGlError( "Invalidated Shaders and Textures" );

#if !USE_D3D
    printGLString( "Version", GL_VERSION );
    printGLString( "Vendor", GL_VENDOR );
    printGLString( "Renderer", GL_RENDERER );
    printGLString( "Extensions", GL_EXTENSIONS );
#endif

    checkGlError( "OnSurfaceCreated\n" );
}

void GameCore::OnSurfaceLost()
{
    m_GLSurfaceIsValid = false;

    LOGInfo( LOGTag, "onSurfaceLost()\n" );

    checkGlError( "Before Invalidated Shaders and Textures" );
    if( g_pShaderManager )
        g_pShaderManager->InvalidateAllShaders( false );
    if( g_pTextureManager )
        g_pTextureManager->InvalidateAllTextures( false );
    if( g_pBufferManager )
        g_pBufferManager->InvalidateAllBuffers( false );
    checkGlError( "Invalidated Shaders and Textures" );

    checkGlError( "OnSurfaceLost" );
}

void GameCore::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
#if !MYFW_IOS
    LOGInfo( LOGTag, "OnSurfaceChanged(%d, %d)\n", width, height );
#endif
    
    m_WindowStartX = (float)startx;
    m_WindowStartY = (float)starty;
    m_WindowWidth = (float)width;
    m_WindowHeight = (float)height;

    // only draw to part of the window, but rest with scissor test and glViewPort.
    if( startx != 0 || starty != 0 )
    {
        // scissor test is really only needed for the glClear call.
        glEnable( GL_SCISSOR_TEST );
        glScissor( startx, starty, width, height );
    }

    glViewport( startx, starty, width, height );
    checkGlError( "glViewport" );
}

void GameCore::OnDrawFrame()
{
    g_GLStats.NewFrame();
    //LOGInfo( LOGTag, "OnDrawFrame()\n" );
}

void GameCore::OnDrawFrameDone()
{
    g_GLStats.EndFrame();
    //LOGInfo( LOGTag, "OnDrawFrame()\n" );
}

void GameCore::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    m_LastInputMethodUsed = InputMethod_Touch;

    //LOGInfo( LOGTag, "GameCore: OnTouch (%d %d)(%f,%f)(%f %f)\n", action, id, x, y, pressure, size);
}

void GameCore::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    m_LastInputMethodUsed = InputMethod_Pad;

    MyAssert( id >= 0 && id < GCBI_NumButtons );

    if( action == GCBA_Down )
        m_ButtonsHeld[id] = true;
    else if( action == GCBA_Up )
        m_ButtonsHeld[id] = false;
}

void GameCore::OnKey(GameCoreButtonActions action, int keycode, int unicodechar)
{
}

void GameCore::OnKeyDown(int keycode, int unicodechar)
{
    // if the key is mapped to a button, then call the button handler.
    if( m_KeyMappingToButtons[keycode] != GCBI_NumButtons && keycode < 255 )
    {
        OnButtons( GCBA_Down, m_KeyMappingToButtons[keycode] );
    }
    else
    {
        m_LastInputMethodUsed = InputMethod_Keyboard;

        if( keycode >= 0 && keycode < 255 )
            m_KeysHeld[keycode] = true;

        OnKey( GCBA_Down, keycode, unicodechar );
    }
}

void GameCore::OnKeyUp(int keycode, int unicodechar)
{
    // if the key is mapped to a button, then call the button handler.
    if( m_KeyMappingToButtons[keycode] != GCBI_NumButtons && keycode < 255 )
    {
        OnButtons( GCBA_Up, m_KeyMappingToButtons[keycode] );
    }
    else
    {
        m_LastInputMethodUsed = InputMethod_Keyboard;

        if( keycode >= 0 && keycode < 255 )
            m_KeysHeld[keycode] = false;

        OnKey( GCBA_Up, keycode, unicodechar );
    }
}

bool GameCore::IsKeyHeld(int keycode)
{
    // if the key is mapped to a button, then the key isn't held.
    if( m_KeyMappingToButtons[keycode] != GCBI_NumButtons )
    {
        return false;
    }
    else
    {
        if( keycode >= 0 && keycode < 255 )
            return m_KeysHeld[keycode];
    }

    return false;
}
