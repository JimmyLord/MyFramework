//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "GameCore.h"
#include "../SourceCommon/Input/GamepadManager.h"
#include "../SourceWindows/GamepadManagerXInput.h"
#include "Renderers/Renderer_Base.h"
#include "Renderers/OpenGL/Renderer_OpenGL.h"

#if MYFW_EDITOR
#include "../SourceEditor/CommandStack.h"
#endif

GameCore* g_pGameCore = 0;

GameCore::GameCore(Renderer_Base* pRenderer)
{
    g_pGameCore = this;

    TestMyTypeSizes();

    m_OneTimeInitWasCalled = false;
    m_GameConfirmedCloseIsOkay = false;

    if( pRenderer )
        m_pRenderer = pRenderer;
    else
        m_pRenderer = MyNew Renderer_OpenGL();

    m_pSoundPlayer = 0;
    m_pSoundManager = 0;
    m_pMyJobManager = 0;
#if MYFW_BLACKBERRY
    m_pMediaPlayer = 0;
#endif

#if MYFW_IOS || MYFW_ANDROID
    g_pIAPManager = MyNew IAPManager;
#endif

    m_HasFocus = true;
    m_Settled = false;

    m_KeyboardOpenRequested = false;
    m_KeyboardCloseRequested = false;

    m_MouseLockRequested = false;

    m_LastInputMethodUsed = InputMethod_Touch;

    for( int i=0; i<GCBI_NumButtons; i++ )
        m_ButtonsHeld[i] = false;

    for( int i=0; i<255; i++ )
    {
        m_KeysHeld[i] = false;
        m_KeyMappingToButtons[i] = GCBI_NumButtons;
    }

#if MYFW_EDITOR
    m_pCommandStack = 0;
#endif
}

GameCore::~GameCore()
{
    LOGInfo( LOGTag, "GameCore::~GameCore()\n" );

    SAFE_DELETE( m_pMyJobManager );

    g_pFontManager->FreeAllFonts();

    SAFE_DELETE( g_pGamepadManager );
    SAFE_DELETE( g_pMaterialManager );
    SAFE_DELETE( g_pTextureManager );
    SAFE_DELETE( g_pFontManager );
    SAFE_DELETE( g_pBufferManager );
    SAFE_DELETE( g_pMeshManager );
    SAFE_DELETE( g_pLightManager );
    SAFE_DELETE( g_pShaderManager );
    SAFE_DELETE( g_pShaderGroupManager );
    SAFE_DELETE( m_pSoundManager );
    SAFE_DELETE( m_pSoundPlayer );
    SAFE_DELETE( g_pGameServiceManager );
    SAFE_DELETE( g_pVertexFormatManager );

    SAFE_DELETE( g_pFileManager ); // Will assert if all files aren't free, so delete last.

    SAFE_DELETE( g_pEventManager );
    SAFE_DELETE( g_pEventTypeManager );

#if MYFW_BLACKBERRY
    SAFE_DELETE( m_pMediaPlayer );
#endif

#if MYFW_IOS
    SAFE_DELETE( g_pIAPManager );
#endif

    delete m_pRenderer;
}

bool GameCore::IsGLSurfaceIsValid()
{
    return m_pRenderer->IsValid();
}

uint32 GameCore::GetWindowWidth()
{
    return m_pRenderer->GetWindowWidth();
}

uint32 GameCore::GetWindowHeight()
{
    return m_pRenderer->GetWindowHeight();
}

void GameCore::InitializeManagers()
{
    if( g_pEventTypeManager == 0 )
        g_pEventTypeManager = MyNew EventTypeManager;
    if( g_pEventManager == 0 )
        g_pEventManager = MyNew EventManager;
    if( g_pFileManager == 0 )
        g_pFileManager = MyNew FileManager;
    if( g_pTextureManager == 0 )
        g_pTextureManager = MyNew TextureManager;
    if( g_pFontManager == 0 )
        g_pFontManager = MyNew FontManager;
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
#if MYFW_WINDOWS
    if( g_pGamepadManager == 0 )
        g_pGamepadManager = MyNew GamepadManagerXInput;
#endif
    if( g_pGamepadManager )
        g_pGamepadManager->Initialize();
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
    m_pSoundManager = MyNew SoundManager;
    m_pMyJobManager = MyNew MyJobManager;

#if MYFW_BLACKBERRY
    m_pMediaPlayer = MyNew MediaPlayer;
#endif

#if MYFW_IOS || MYFW_ANDROID
    g_pIAPManager->Initialize();
#endif

    m_TimeSinceGameStarted = 0;
}

void GameCore::OnPrepareToDie()
{
}

bool GameCore::IsReadyToRender()
{
    if( IsGLSurfaceIsValid() == false )
        return false;

    return true;
}

float GameCore::Tick(float deltaTime)
{
    m_TimePassedUnpausedLastFrame = deltaTime;
    m_TimeSinceGameStarted += deltaTime;

    GenerateKeyHeldMessages();

#if MYFW_WINDOWS
    size_t numbytesallocated = MyMemory_GetNumberOfBytesAllocated();
#endif

    g_pEventManager->Tick( deltaTime );
    g_pFileManager->Tick();
    g_pTextureManager->Tick();
    g_pMaterialManager->Tick();
    m_pSoundManager->Tick();
    g_pBufferManager->Tick();
    g_pFontManager->Tick();
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

    if( g_pGamepadManager )
        g_pGamepadManager->Tick( deltaTime );

#if MYFW_USING_WX
    g_pPanelWatch->Tick( deltaTime );
#endif

    // Return how much time we advanced the game state.
    return deltaTime;
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
    m_pRenderer->OnSurfaceCreated();
}

void GameCore::OnSurfaceLost()
{
    m_pRenderer->OnSurfaceCreated();

    // These calls don't clean out opengl allocations,
    //     The surface was already lost along with all allocs.
    if( g_pShaderManager )
        g_pShaderManager->InvalidateAllShaders( false );
    if( g_pTextureManager )
        g_pTextureManager->InvalidateAllTextures( false );
    if( g_pBufferManager )
        g_pBufferManager->InvalidateAllBuffers( false );
}

void GameCore::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    m_pRenderer->OnSurfaceChanged( startx, starty, width, height );
}

void GameCore::OnDrawFrameStart(unsigned int canvasid)
{
    g_GLStats.NewFrame();
}

void GameCore::OnDrawFrame(unsigned int canvasid)
{
    //LOGInfo( LOGTag, "OnDrawFrame()\n" );
}

void GameCore::OnDrawFrameDone()
{
    g_GLStats.EndFrame();
    //LOGInfo( LOGTag, "OnDrawFrame()\n" );

#if MYFW_EDITOR
    m_pCommandStack->IncrementFrameCount();
#endif
}

void GameCore::OnFileRenamed(const char* fullpathbefore, const char* fullpathafter)
{
}

void GameCore::SetMouseLock(bool lock)
{
    m_MouseLockRequested = lock;
}

bool GameCore::WasMouseLockRequested()
{
    return m_MouseLockRequested;
}

bool GameCore::IsMouseLocked()
{
    return PlatformSpecific_IsMouseLocked();
}

bool GameCore::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    m_LastInputMethodUsed = InputMethod_Touch;

    //LOGInfo( LOGTag, "GameCore: OnTouch (%d %d)(%f,%f)(%f %f)\n", action, id, x, y, pressure, size);
    return false;
}

bool GameCore::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    m_LastInputMethodUsed = InputMethod_Pad;

    MyAssert( id >= 0 && id < GCBI_NumButtons );

    if( action == GCBA_Down )
        m_ButtonsHeld[id] = true;
    else if( action == GCBA_Up )
        m_ButtonsHeld[id] = false;

    return false;
}

bool GameCore::OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 )
#endif
    {
        // if the key is mapped to a button, then call the button handler.
        if( m_KeyMappingToButtons[keycode] != GCBI_NumButtons && keycode < 255 )
        {
            return OnButtons( action, m_KeyMappingToButtons[keycode] );
        }
        else
        {
            m_LastInputMethodUsed = InputMethod_Keyboard;
        }
    }

    return false;
}

bool GameCore::OnChar(unsigned int c)
{
    return false;
}

bool GameCore::OnKeyDown(int keycode, int unicodechar)
{
    // TODO: don't ignore the unicode characters.
#if MYFW_WINDOWS || MYFW_OSX
    if( keycode >= 'A' && keycode <= 'Z' && m_KeysHeld[MYKEYCODE_LSHIFT] == 0 && m_KeysHeld[MYKEYCODE_RSHIFT] == 0 )
    {
        return OnKeys( GCBA_Down, keycode, keycode+32 );
    }
    else
#endif
    {
        return OnKeys( GCBA_Down, keycode, keycode );
    }

    return false;
}

bool GameCore::OnKeyUp(int keycode, int unicodechar)
{
    // TODO: don't ignore the unicode characters.

    if( keycode >= 0 && keycode < 255 )
        m_KeysHeld[keycode] = false;

#if MYFW_WINDOWS || MYFW_OSX
    if( keycode >= 'A' && keycode <= 'Z' && m_KeysHeld[MYKEYCODE_LSHIFT] == 0 && m_KeysHeld[MYKEYCODE_RSHIFT] == 0 )
    {
        return OnKeys( GCBA_Up, keycode, keycode+32 );
    }
    else
#endif
    {
        return OnKeys( GCBA_Up, keycode, keycode );
    }

    return false;
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

void GameCore::ForceKeyRelease(int keycode)
{
    if( keycode >= 0 && keycode < 255 )
        m_KeysHeld[keycode] = false;
}

void GameCore::GenerateKeyHeldMessages()
{
    // generate held messages for keys and buttons.
#if MYFW_USING_IMGUI
    for( int i=0; i<255; i++ )
    {
        if( m_KeysHeld[i] )
        {
#if MYFW_WINDOWS || MYFW_OSX
            if( i >= 'A' && i <= 'Z' && m_KeysHeld[MYKEYCODE_LSHIFT] == 0 && m_KeysHeld[MYKEYCODE_RSHIFT] == 0 )
                g_pGameCore->OnKeys( GCBA_Held, i, i+32 );
            else
#endif
                g_pGameCore->OnKeys( GCBA_Held, i, i );
        }
    }
#endif

    for( int i=0; i<GCBI_NumButtons; i++ )
    {
        if( m_ButtonsHeld[i] == true )
            OnButtons( GCBA_Held, (GameCoreButtonIDs)i );
    }
}
