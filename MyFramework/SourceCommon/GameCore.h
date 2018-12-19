//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameCore_H__
#define __GameCore_H__

class GameCore;
class GameLevel;
class BMFont;
class SoundPlayer;
class SoundManager;
class Renderer_Base;
#if MYFW_EDITOR
class CommandStack;
#endif

extern GameCore* g_pGameCore;

enum GameCoreInputMethods
{
    InputMethod_Touch,
    InputMethod_Pad,
    InputMethod_Keyboard,
};

enum GameCoreButtonActions // following android finger keys
{ // If changed here, change in LuaGameState.cpp and snippets.json as well.
    GCBA_Down,
    GCBA_Up,
    GCBA_Held,
    GCBA_Wheel,
    GCBA_RelativeMovement,
};

enum GameCoreButtonIDs
{ // If changed here, change in LuaGameState.cpp and snippets.json as well.
    GCBI_Back,
    GCBI_Left,
    GCBI_Right,
    GCBI_Up,
    GCBI_Down,
    GCBI_ButtonA,
    GCBI_ButtonB,
    GCBI_ButtonC,
    GCBI_ButtonD,
    GCBI_NumButtons,
};

enum IAPErrorCodes
{
    IAPErrorCode_Success,
    IAPErrorCode_UnknownError,
    IAPErrorCode_UserCancelled,
    IAPErrorCode_PaymentSystemBusy,
    IAPErrorCode_PaymentError,
    IAPErrorCode_NoNetworkConnectivity,
};

class GameCore
{
protected:
    bool m_OneTimeInitWasCalled; // HACK: NaCl and Android builds call OneTimeInit too often, so this will ensure it doesn't get called multiple times.
    bool m_GameConfirmedCloseIsOkay; // Setting this flag should cause the main game window to close.

    Renderer_Base* m_pRenderer;

    bool m_HasFocus;
    bool m_Settled;

    // not happy about these, but MenuInputBox opens/closes keyboard in key/button/touch callback,
    //     but we want to wait until next tick for JNI to be set up.
    bool m_KeyboardOpenRequested;
    bool m_KeyboardCloseRequested; 

    bool m_MouseLockRequested;

    int m_LastInputMethodUsed;

    bool m_ButtonsHeld[GCBI_NumButtons];
    bool m_KeysHeld[512];
    GameCoreButtonIDs m_KeyMappingToButtons[512];
    
    float m_TimeSinceGameStarted;
    float m_TimePassedUnpausedLastFrame;

    MyJobManager* m_pMyJobManager;

    SoundPlayer* m_pSoundPlayer;
    SoundManager* m_pSoundManager;
#if MYFW_BLACKBERRY
    MediaPlayer* m_pMediaPlayer;
#endif

public:
    GameCore(Renderer_Base* pRenderer = 0);
    virtual ~GameCore();

    // Getters.
    bool HasOneTimeInitBeenCalled() { return m_OneTimeInitWasCalled; }
    bool IsGLSurfaceIsValid();

    float GetWindowWidth();
    float GetWindowHeight();
    bool HasFocus() { return m_HasFocus; }
    bool IsSettled() { return m_Settled; }

    int GetLastInputMethodUsed() { return m_LastInputMethodUsed; }

    float GetTimePassedUnpausedLastFrame() { return m_TimePassedUnpausedLastFrame; }

    SoundPlayer* GetSoundPlayer() { return m_pSoundPlayer; }
    SoundManager* GetSoundManager() { return m_pSoundManager; }
#if MYFW_BLACKBERRY
    MediaPlayer* GetMediaPlayer() { return m_pMediaPlayer; }
#endif

    // Setters.
    void SetIsNotSettled() { m_Settled = false; }

    void RequestKeyboardOpen() { m_KeyboardOpenRequested = true; }
    void RequestKeyboardClose() { m_KeyboardCloseRequested = true; }

    // GameCore Methods
    virtual void InitializeManagers();

    virtual void OneTimeInit();
    virtual void OnPrepareToDie();
    virtual bool IsReadyToRender();

    virtual void RequestClose() { m_GameConfirmedCloseIsOkay = true; } // Always close when requested.
    virtual bool HasGameConfirmedCloseIsOkay() { return m_GameConfirmedCloseIsOkay; }
    virtual void SetGameConfirmedCloseIsOkay() { m_GameConfirmedCloseIsOkay = true; }

    virtual float Tick(float deltaTime); // returns time used... i.e. unpaused time.
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnSurfaceCreated();
    virtual void OnSurfaceLost();
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrameStart(unsigned int canvasid);
    virtual void OnDrawFrame(unsigned int canvasid);
    virtual void OnDrawFrameDone();
    virtual void OnFileRenamed(const char* fullpathbefore, const char* fullpathafter);

    virtual void OnDropFile(const char* filename) {}

    virtual void SetMouseLock(bool lock);
    virtual bool WasMouseLockRequested();
    virtual bool IsMouseLocked();
    virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    virtual bool OnKeys(GameCoreButtonActions action, int keycode, int unicodechar);
    virtual bool OnChar(unsigned int c);
    virtual void GenerateKeyHeldMessages();

    // OnKeyDown and OnKeyUp are used by platform-specific code, games should override OnKeys();
    bool OnKeyDown(int keycode, int unicodechar);
    bool OnKeyUp(int keycode, int unicodechar);

    virtual bool IsKeyHeld(int keycode);
    virtual void ForceKeyRelease(int keycode);

    virtual void OnPurchaseComplete(const char* id, const char* sku, IAPErrorCodes errorcode, bool newpurchase) { }

    virtual const char* GetMatchmakingGameName() { return 0; }

#if MYFW_EDITOR
protected:
    CommandStack* m_pCommandStack;

public:
    void SetCommandStack(CommandStack* pCommandStack) { MyAssert( m_pCommandStack == 0 ); m_pCommandStack = pCommandStack; }
    CommandStack* GetCommandStack() { return m_pCommandStack; }
#endif
};

#endif //__GameCore_H__
