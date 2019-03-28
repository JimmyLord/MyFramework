//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "GameCore.h"
#include "ResourceManagers.h"
#include "../DataTypes/MyTypes.h"
#include "../Events/EventManager.h"
#include "../Events/EventTypeManager.h"
#include "../Fonts/FontManager.h"
#include "../Helpers/ExternalTools.h"
#include "../Helpers/FileManager.h"
#include "../Helpers/MyTime.h"
#include "../Meshes/BufferManager.h"
#include "../Meshes/LightManager.h"
#include "../Meshes/MeshManager.h"
#include "../Multithreading/MyJobManager.h"
#include "../Networking/GameServiceManager.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Renderers/Old/OpenGLWrapper.h" // TODO: Fix this dependency.
#include "../Renderers/OpenGL/Renderer_OpenGL.h"
#include "../Shaders/ShaderManager.h"
#include "../Shaders/VertexFormatManager.h"
#include "../Sound/SoundManager.h"
#include "../SourceCommon/Input/GamepadManager.h"
#include "../SourceWindows/GamepadManagerXInput.h"
#include "../Textures/MaterialManager.h"
#include "../Textures/TextureManager.h"

#include "../../SourceWindows/SoundPlayerXAudio.h" // TODO: Fix this dependency.

#if MYFW_EDITOR
#include "../SourceEditor/CommandStack.h"
#endif

const char* g_GameCoreButtonActionStrings[GCBA_NumActions] =
{
    "Down",
    "Up",
    "Held",
    "Wheel",
    "RelativeMovement",
};

const char* g_GameCoreButtonActionLuaStrings[GCBA_NumActions] =
{
    "BUTTONACTION_Down",
    "BUTTONACTION_Up",
    "BUTTONACTION_Held",
    "BUTTONACTION_Wheel",
    "BUTTONACTION_RelativeMovement",
};

GameCore* g_pGameCore = nullptr;

GameCore::GameCore(Renderer_Base* pRenderer, bool createAndOwnGlobalManagers)
{
    g_pGameCore = this;

    m_ThisClassOwnsTheGlobalManagers = createAndOwnGlobalManagers;

    TestMyTypeSizes();

    m_OneTimeInitWasCalled = false;
    m_GameConfirmedCloseIsOkay = false;

    m_OwnsRenderer = false;

    if( pRenderer )
    {
        m_pRenderer = pRenderer;
    }
    else
    {
        m_pRenderer = MyNew Renderer_OpenGL( this );
        m_OwnsRenderer = true;
    }

    m_MainViewport.Set( 0, 0, 0, 0 );

    m_pSoundPlayer = nullptr;
#if MYFW_BLACKBERRY
    m_pMediaPlayer = nullptr;
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
    m_pCommandStack = nullptr;
#endif
}

GameCore::~GameCore()
{
    LOGInfo( LOGTag, "GameCore::~GameCore()\n" );

    SAFE_DELETE( m_Managers.m_pMyJobManager );
    SAFE_DELETE( m_Managers.m_pGamepadManager );

    if( m_Managers.m_pFontManager )
    {
        m_Managers.m_pFontManager->FreeAllFonts();
    }

    SAFE_DELETE( m_Managers.m_pMaterialManager );
    SAFE_DELETE( m_Managers.m_pTextureManager );
    SAFE_DELETE( m_Managers.m_pFontManager );
    SAFE_DELETE( m_Managers.m_pBufferManager );
    SAFE_DELETE( m_Managers.m_pMeshManager );
    SAFE_DELETE( m_Managers.m_pLightManager );
    SAFE_DELETE( m_Managers.m_pShaderManager );
    SAFE_DELETE( m_Managers.m_pShaderGroupManager );
    SAFE_DELETE( m_Managers.m_pSoundManager );
    SAFE_DELETE( m_pSoundPlayer );
    SAFE_DELETE( m_Managers.m_pGameServiceManager );
    SAFE_DELETE( m_Managers.m_pVertexFormatManager );
    SAFE_DELETE( m_Managers.m_pFileManager ); // Will assert if all files aren't free, so delete last.
    SAFE_DELETE( m_Managers.m_pEventManager );
    SAFE_DELETE( m_Managers.m_pEventTypeManager );

#if MYFW_BLACKBERRY
    SAFE_DELETE( m_pMediaPlayer );
#endif

#if MYFW_IOS
    SAFE_DELETE( g_pIAPManager );
#endif

    if( m_OwnsRenderer )
    {
        if( g_pRenderer == m_pRenderer )
            g_pRenderer = nullptr;

        delete m_pRenderer;
    }
}

bool GameCore::IsGLSurfaceIsValid()
{
    return m_pRenderer->IsValid();
}

uint32 GameCore::GetWindowWidth()
{
    return m_MainViewport.GetWidth();
}

uint32 GameCore::GetWindowHeight()
{
    return m_MainViewport.GetHeight();
}

void GameCore::InitializeManagers()
{
    if( m_Managers.m_pEventTypeManager == nullptr )     m_Managers.m_pEventTypeManager = MyNew EventTypeManager;
    if( m_Managers.m_pEventManager == nullptr )         m_Managers.m_pEventManager = MyNew EventManager( m_Managers.GetEventTypeManager() );
    if( m_Managers.m_pVertexFormatManager == nullptr )  m_Managers.m_pVertexFormatManager = MyNew VertexFormatManager;
    if( m_Managers.m_pFileManager == nullptr )          m_Managers.m_pFileManager = MyNew FileManager( this );
    if( m_Managers.m_pTextureManager == nullptr )       m_Managers.m_pTextureManager = MyNew TextureManager( this );
    if( m_Managers.m_pFontManager == nullptr )          m_Managers.m_pFontManager = MyNew FontManager( this );
    if( m_Managers.m_pMaterialManager == nullptr )      m_Managers.m_pMaterialManager = MyNew MaterialManager( this );
    if( m_Managers.m_pBufferManager == nullptr )        m_Managers.m_pBufferManager = MyNew BufferManager();
    if( m_Managers.m_pMeshManager == nullptr )          m_Managers.m_pMeshManager = MyNew MeshManager( this );
    if( m_Managers.m_pLightManager == nullptr )         m_Managers.m_pLightManager = MyNew LightManager();
    if( m_Managers.m_pShaderManager == nullptr )        m_Managers.m_pShaderManager = MyNew ShaderManager( this );
    if( m_Managers.m_pShaderGroupManager == nullptr )   m_Managers.m_pShaderGroupManager = MyNew ShaderGroupManager;
    if( m_Managers.m_pGameServiceManager == nullptr )   m_Managers.m_pGameServiceManager = MyNew GameServiceManager;
#if MYFW_WINDOWS
    if( m_Managers.m_pGamepadManager == nullptr )       m_Managers.m_pGamepadManager = MyNew GamepadManagerXInput;
#endif
    
    if( m_Managers.m_pGamepadManager )
        m_Managers.m_pGamepadManager->Initialize();

    m_Managers.m_pSoundManager = MyNew SoundManager( this );
    m_Managers.m_pMyJobManager = MyNew MyJobManager;
}

void GameCore::OneTimeInit()
{
    if( m_OneTimeInitWasCalled )
        return;

    m_OneTimeInitWasCalled = true;

    // Base seed on realtime if available.  Don't fmod on emscripten, nanosecs aren't accurate enough.
#if MYFW_EMSCRIPTEN
    double time = MyTime_GetSystemTime( true );
    unsigned int seed = (unsigned int)time;
#else
    double time = MyTime_GetSystemTime( true ) * 10000000;
    unsigned int seed = (uint32)fmod( time, 100000 );
#endif
    LOGInfo( LOGTag, "Seeding random: %d\n", seed );
    srand( seed );

    InitializeManagers();

    m_pSoundPlayer = MyNew SoundPlayer;

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

    m_Managers.m_pEventManager->Tick( deltaTime );
    m_Managers.m_pFileManager->Tick();
    m_Managers.m_pTextureManager->Tick();
    m_Managers.m_pMaterialManager->Tick();
    m_Managers.m_pSoundManager->Tick();
    m_Managers.m_pBufferManager->Tick();
    m_Managers.m_pFontManager->Tick();
#if MYFW_BLACKBERRY10
    if( m_WindowWidth == m_WindowHeight )
        m_Managers.m_pGameServiceManager->Tick( "Q10" );
    else
        m_Managers.m_pGameServiceManager->Tick( "Z10" );
#else
    m_Managers.m_pGameServiceManager->Tick();
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

    if( m_Managers.m_pGamepadManager )
        m_Managers.m_pGamepadManager->Tick( deltaTime );

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

void GameCore::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
    m_MainViewport.Set( x, y, width, height );

    m_pRenderer->OnSurfaceChanged( x, y, width, height );
}

void GameCore::OnSurfaceLost()
{
    m_pRenderer->OnSurfaceCreated();

    // These calls don't clean out opengl allocations,
    //     the surface was already lost along with all allocs.
    if( m_Managers.m_pShaderManager )
        m_Managers.m_pShaderManager->InvalidateAllShaders( false );
    if( m_Managers.m_pTextureManager )
        m_Managers.m_pTextureManager->InvalidateAllTextures( false );
    if( m_Managers.m_pBufferManager )
        m_Managers.m_pBufferManager->InvalidateAllBuffers( false );
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
        // If the key is mapped to a button, then call the button handler.
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
    // TODO: Don't ignore the unicode characters.
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
    // TODO: Don't ignore the unicode characters.

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
    // If the key is mapped to a button, then the key isn't held.
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
    // Generate held messages for keys and buttons.
#if MYFW_USING_IMGUI
    for( int i=0; i<255; i++ )
    {
        if( m_KeysHeld[i] )
        {
#if MYFW_WINDOWS || MYFW_OSX
            if( i >= 'A' && i <= 'Z' && m_KeysHeld[MYKEYCODE_LSHIFT] == 0 && m_KeysHeld[MYKEYCODE_RSHIFT] == 0 )
                OnKeys( GCBA_Held, i, i+32 );
            else
#endif
                OnKeys( GCBA_Held, i, i );
        }
    }
#endif

    for( int i=0; i<GCBI_NumButtons; i++ )
    {
        if( m_ButtonsHeld[i] == true )
            OnButtons( GCBA_Held, (GameCoreButtonIDs)i );
    }
}
