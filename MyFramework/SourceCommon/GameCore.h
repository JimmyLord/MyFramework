//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameCore_H__
#define __GameCore_H__

class GameCore;
class GameLevel;
class BMFont;
class SoundPlayer;

extern GameCore* g_pGameCore;

enum GameCoreInputMethods
{
    InputMethod_Touch,
    InputMethod_Pad,
    InputMethod_Keyboard,
};

enum GameCoreButtonActions // following android finger keys
{
    GCBA_Down,
    GCBA_Up,
    GCBA_Held,
};

enum GameCoreButtonIDs
{
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
public:
    bool m_OneTimeInitWasCalled; // HACK: in android, onetimeinit *was* called in tick, this probably isn't needed anymore, but why not.

    bool m_GLSurfaceIsValid;

    float m_WindowStartX;
    float m_WindowStartY;
    float m_WindowWidth;
    float m_WindowHeight;
    bool m_HasFocus;
    bool m_Settled;

    // not happy about these, but MenuInputBox opens/closes keyboard in key/button/touch callback,
    //     but we want to wait until next tick for JNI to be set up.
    bool m_KeyboardOpenRequested;
    bool m_KeyboardCloseRequested; 

    int m_LastInputMethodUsed;

    bool m_ButtonsHeld[GCBI_NumButtons];
    
    float m_TimeSinceGameStarted;

public:
    SoundPlayer* m_pSoundPlayer;
#if MYFW_BLACKBERRY
    MediaPlayer* m_pMediaPlayer;
#endif

public:
    GameCore();
    virtual ~GameCore();

    virtual void InitializeManagers();

    virtual void OneTimeInit();
    virtual void OnPrepareToDie();
    virtual bool IsReadyToRender();

    virtual void Tick(double TimePassed);
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnSurfaceCreated();
    virtual void OnSurfaceLost();
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrame();
    virtual void OnDrawFrameDone();

    virtual void OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual void OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    virtual void OnKeyDown(int keycode, int unicodechar);
    virtual void OnKeyUp(int keycode, int unicodechar);

    virtual void OnPurchaseComplete(const char* id, const char* sku, IAPErrorCodes errorcode, bool newpurchase) { }

    virtual const char* GetMatchmakingGameName() { return 0; }

};

#endif //__GameCore_H__
