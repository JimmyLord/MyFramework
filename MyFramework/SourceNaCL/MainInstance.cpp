//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"
#include "../SourceCommon/GameCore.h"

#include "MainInstance.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "OpenGLContext.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/input_event.h"

MainInstance* g_pInstance = 0;

static int currwidth = -1;
static int currheight = -1;

MainInstance::MainInstance(PP_Instance instance)
: pp::Instance(instance)
{
    g_pInstance = this;

    LOGInfo( LOGTag, "Constructor\n" );

    //RequestInputEvents( PP_INPUTEVENT_CLASS_KEYBOARD );
    RequestInputEvents( PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_WHEEL | PP_INPUTEVENT_CLASS_KEYBOARD );
    //RequestFilteringInputEvents( PP_INPUTEVENT_CLASS_KEYBOARD );

    for( int i=0; i<GCBI_NumButtons; i++ )
    {
        m_ButtonsDown[i] = false;
    }

    m_ShiftsHeld = 0;
}

MainInstance::~MainInstance()
{
    m_OpenGLContext->MakeContextCurrent(this);
}

// Handle an incoming input event by switching on type and dispatching
// to the appropriate subtype handler.
bool MainInstance::HandleInputEvent(const pp::InputEvent& event)
{
    if( g_pGameCore == 0 )
        return false;

    //LOGInfo( LOGTag, "Key HandleInputEvent\n" );
    switch( event.GetType() )
    {
    case PP_INPUTEVENT_TYPE_UNDEFINED:
        break;

    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
        {
            pp::MouseInputEvent mouseevent = pp::MouseInputEvent(event);

            float x = (float)mouseevent.GetPosition().x();
            float y = (float)mouseevent.GetPosition().y();

            g_pGameCore->OnTouch( GCBA_Down, 0, x, y, 0, 0 ); // new press
        }
        //GotMouseEvent(pp::MouseInputEvent(event), "Down");
        break;

    case PP_INPUTEVENT_TYPE_MOUSEUP:
        {
            pp::MouseInputEvent mouseevent = pp::MouseInputEvent(event);

            float x = (float)mouseevent.GetPosition().x();
            float y = (float)mouseevent.GetPosition().y();

            g_pGameCore->OnTouch( GCBA_Up, 0, x, y, 0, 0 ); // new release
        }
        //GotMouseEvent(pp::MouseInputEvent(event), "Up");
        break;

    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
        {
            pp::MouseInputEvent mouseevent = pp::MouseInputEvent(event);

            float x = (float)mouseevent.GetPosition().x();
            float y = (float)mouseevent.GetPosition().y();

            g_pGameCore->OnTouch( GCBA_Held, 0, x, y, 0, 0 );
        }
        //GotMouseEvent(pp::MouseInputEvent(event), "Move");
        break;

    case PP_INPUTEVENT_TYPE_MOUSEENTER:
        //GotMouseEvent(pp::MouseInputEvent(event), "Enter");
        break;

    case PP_INPUTEVENT_TYPE_MOUSELEAVE:
        //GotMouseEvent(pp::MouseInputEvent(event), "Leave");
        break;

    case PP_INPUTEVENT_TYPE_WHEEL:
        //GotWheelEvent(pp::WheelInputEvent(event));
        break;

    case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
        LOGInfo( LOGTag, "Key PP_INPUTEVENT_TYPE_RAWKEYDOWN\n" );
        //GotKeyEvent(pp::KeyboardInputEvent(event), "RawKeyDown");
        break;

    case PP_INPUTEVENT_TYPE_KEYDOWN:
        {
            pp::KeyboardInputEvent keyevent = pp::KeyboardInputEvent(event);

            if( keyevent.GetKeyCode() == 16 ) // either shift
                m_ShiftsHeld = 1;

            int keycode = keyevent.GetKeyCode();
            if( keycode >= 'A' && keycode <= 'Z' && m_ShiftsHeld == 0 )
                g_pGameCore->OnKeyDown( keycode+32, keycode+32 );
            else
                g_pGameCore->OnKeyDown( keycode, keycode );

            //LOGInfo( LOGTag, "Key Down\n" );
            //GotKeyEvent(pp::KeyboardInputEvent(event), "Down");
        }
        break;

    case PP_INPUTEVENT_TYPE_KEYUP:
        {
            pp::KeyboardInputEvent keyevent = pp::KeyboardInputEvent(event);

            if( keyevent.GetKeyCode() == 16 ) // either shift
                m_ShiftsHeld = 0;

            int keycode = keyevent.GetKeyCode();
            if( keycode >= 'A' && keycode <= 'Z' && m_ShiftsHeld == 0 )
                g_pGameCore->OnKeyUp( keycode+32, keycode+32 );
            else
                g_pGameCore->OnKeyUp( keycode, keycode );

            //LOGInfo( LOGTag, "Key Up\n" );
            //GotKeyEvent(, "Up");
        }
        break;

    case PP_INPUTEVENT_TYPE_CHAR:
        //LOGInfo( LOGTag, "Key PP_INPUTEVENT_TYPE_CHAR\n" );
        //GotKeyEvent(pp::KeyboardInputEvent(event), "Character");
        break;

    default:
        MyAssert(false);
        return false;
    }

    return true;
}

bool MainInstance::Init(uint32_t argc, const char* argn[], const char* argv[])
{
    LOGInfo( LOGTag, "Init\n" );
    return true;
}

void MainInstance::HandleMessage(const pp::Var& message)
{
    if( !message.is_string() )
        return;

    //m_ScriptingBridge.InvokeMethod( message.AsString() );
}

void MainInstance::DidChangeView(const pp::Rect& position, const pp::Rect& clip)
{
    LOGInfo( LOGTag, "DidChangeView\n" );

    if( position.size().width() == currwidth && position.size().height() == currheight )
        return; // Size didn't change, no need to update anything.

    LOGInfo( LOGTag, "DidChangeView - something changed\n" );

    currwidth = position.size().width();
    currheight = position.size().height();

    if( m_OpenGLContext == 0 )
        m_OpenGLContext.reset(new OpenGLContext(this));
    m_OpenGLContext->InvalidateContext(this);
    m_OpenGLContext->ResizeContext(position.size());

    if( !m_OpenGLContext->MakeContextCurrent(this) )
        return;
    
    if( g_pGameCore )
    {
        g_pGameCore->OnSurfaceCreated();
        g_pGameCore->OnSurfaceChanged( 0, 0, currwidth, currheight );

        // now the opengl is initialized, let's call OneTimeInit() if it hasn't been.
        if( g_pGameCore->m_OneTimeInitWasCalled == false )
            g_pGameCore->OneTimeInit();
    }

    DrawSelf();
}

void MainInstance::DrawSelf()
{
    if( m_OpenGLContext == 0 )
        return;

    m_OpenGLContext->MakeContextCurrent(this);

    static double lasttime = MyTime_GetSystemTime();

    if( g_pGameCore )
    {
        // send held messages every frame for buttons that are held.
        for( int i=0; i<GCBI_NumButtons; i++ )
        {
            if( m_ButtonsDown[i] )
            {
                g_pGameCore->OnButtons( GCBA_Held, (GameCoreButtonIDs)i );
            }
        }

        double currtime = MyTime_GetSystemTime();
        double timepassed = currtime - lasttime;
        lasttime = currtime;        

        g_UnpausedTime += g_pGameCore->Tick( timepassed );
        g_pGameCore->OnDrawFrame();
        g_pGameCore->OnDrawFrameDone();
        m_OpenGLContext->FlushContext();
    }
}
