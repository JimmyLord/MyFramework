//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
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
#include "ppapi/lib/gl/gles2/gl2ext_ppapi.h"

MainInstance* g_pInstance = 0;

static int currwidth = -1;
static int currheight = -1;

MainInstance::MainInstance(PP_Instance instance)
: pp::Instance(instance)
, pp::MouseLock(this)
, m_CallbackFactory(this)
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
    m_GameWantsLockedMouse = false;
    m_SystemMouseIsLocked = false;
}

MainInstance::~MainInstance()
{
    //m_OpenGLContext.MakeContextCurrent(this);
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

            if( m_GameWantsLockedMouse == true && m_SystemMouseIsLocked == false )
            {
                SetMouseLock( true );
            }

            if( m_GameWantsLockedMouse == false || m_SystemMouseIsLocked == true )
            {
                int id = mouseevent.GetButton();                
                if( id == PP_INPUTEVENT_MOUSEBUTTON_LEFT ) id = 0;
                else if( id == PP_INPUTEVENT_MOUSEBUTTON_RIGHT ) id = 1;
                else if( id == PP_INPUTEVENT_MOUSEBUTTON_MIDDLE ) id = 1;

                float x = (float)mouseevent.GetPosition().x();
                float y = (float)mouseevent.GetPosition().y();

                if( m_GameWantsLockedMouse && m_SystemMouseIsLocked )
                {
                    x = currwidth/2;
                    y = currheight/2;
                }

                g_pGameCore->OnTouch( GCBA_Down, id, x, y, 0, 0 ); // new press
            }
        }
        //GotMouseEvent(pp::MouseInputEvent(event), "Down");
        break;

    case PP_INPUTEVENT_TYPE_MOUSEUP:
        {
            pp::MouseInputEvent mouseevent = pp::MouseInputEvent(event);

            int id = mouseevent.GetButton();
            if( id == PP_INPUTEVENT_MOUSEBUTTON_LEFT ) id = 0;
            else if( id == PP_INPUTEVENT_MOUSEBUTTON_RIGHT ) id = 1;
            else if( id == PP_INPUTEVENT_MOUSEBUTTON_MIDDLE ) id = 1;

            float x = (float)mouseevent.GetPosition().x();
            float y = (float)mouseevent.GetPosition().y();

            if( m_GameWantsLockedMouse && m_SystemMouseIsLocked )
            {
                x = currwidth/2;
                y = currheight/2;
            }

            g_pGameCore->OnTouch( GCBA_Up, id, x, y, 0, 0 ); // new release
        }
        //GotMouseEvent(pp::MouseInputEvent(event), "Up");
        break;

    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
        {
            pp::MouseInputEvent mouseevent = pp::MouseInputEvent(event);

            float x;
            float y;

            if( m_GameWantsLockedMouse )
            {
                // Only send mouse movement messages (position diffs) if the system mouse is locked
                if( m_SystemMouseIsLocked )
                {
                    x = (float)mouseevent.GetMovement().x();
                    y = (float)mouseevent.GetMovement().y();
                    
                    g_pGameCore->OnTouch( GCBA_RelativeMovement, 0, x, y, 0, 0 );
                }
            }
            else
            {
                // Only send mouse positions if system mouse isn't locked
                if( m_SystemMouseIsLocked == false )
                {
                    x = (float)mouseevent.GetPosition().x();
                    y = (float)mouseevent.GetPosition().y();

                    g_pGameCore->OnTouch( GCBA_Held, 0, x, y, 0, 0 );
                }
            }
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
            //if( keycode >= 'A' && keycode <= 'Z' && m_ShiftsHeld == 0 )
            //    g_pGameCore->OnKeyDown( keycode+32, keycode+32 );
            //else
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
            //if( keycode >= 'A' && keycode <= 'Z' && m_ShiftsHeld == 0 )
            //    g_pGameCore->OnKeyUp( keycode+32, keycode+32 );
            //else
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

bool MainInstance::InitGL(int32_t new_width, int32_t new_height)
{
    if( !glInitializePPAPI( pp::Module::Get()->get_browser_interface() ) )
    {
        //fprintf( stderr, "Unable to initialize GL PPAPI!\n" );
        return false;
    }

    const int32_t attrib_list[] =
    {
        PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
        PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
        PP_GRAPHICS3DATTRIB_WIDTH, new_width,
        PP_GRAPHICS3DATTRIB_HEIGHT, new_height,
        PP_GRAPHICS3DATTRIB_NONE
    };

    m_OpenGLContext = pp::Graphics3D( this, attrib_list );
    if( !BindGraphics( m_OpenGLContext ) )
    {
        //fprintf( stderr, "Unable to bind 3d context!\n" );
        m_OpenGLContext = pp::Graphics3D();
        glSetCurrentContextPPAPI(0);
        return false;
    }

    glSetCurrentContextPPAPI( m_OpenGLContext.pp_resource() );
    return true;
}

void MainInstance::DidChangeView(const pp::View& view)
{
    LOGInfo( LOGTag, "DidChangeView\n" );

    int32_t new_width = view.GetRect().width() * view.GetDeviceScale();
    int32_t new_height = view.GetRect().height() * view.GetDeviceScale();

    if( new_width == currwidth && new_height == currheight )
        return; // Size didn't change, no need to update anything.

    LOGInfo( LOGTag, "DidChangeView - something changed\n" );

    currwidth = new_width;
    currheight = new_height;

    //if( m_OpenGLContext == 0 )
    //    m_OpenGLContext.reset(new OpenGLContext(this));
    //m_OpenGLContext->InvalidateContext(this);
    //m_OpenGLContext->ResizeContext(position.size());

    //if( !m_OpenGLContext->MakeContextCurrent(this) )
    //    return;
    
    if( m_OpenGLContext.is_null() )
    {
        if( !InitGL( new_width, new_height ) )
        {
            return; // failed
        }
        //InitShaders();
        //InitBuffers();
        //InitTexture();
        //MainLoop(0);
    }
    else
    {
        // Resize the buffers to the new size of the module.
        int32_t result = m_OpenGLContext.ResizeBuffers( new_width, new_height );
        if( result < 0 )
        {
            return; // Unable to resize buffers
        }
    }

    if( g_pGameCore )
    {
        g_pGameCore->OnSurfaceCreated();
        g_pGameCore->OnSurfaceChanged( 0, 0, currwidth, currheight );

        // now the opengl is initialized, let's call OneTimeInit() if it hasn't been.
        if( g_pGameCore->m_OneTimeInitWasCalled == false )
            g_pGameCore->OneTimeInit();
    }

    DrawSelf( 0 );
}

void MainInstance::DrawSelf(int32_t somevaluethecallbackfactorywants)
{
    //if( m_OpenGLContext == 0 )
    //    return;

    //m_OpenGLContext->MakeContextCurrent(this);

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

        g_pGameCore->OnDrawFrameStart( 0 );
        g_UnpausedTime += g_pGameCore->Tick( timepassed );
        g_pGameCore->OnDrawFrame( 0 );
        g_pGameCore->OnDrawFrameDone();

        m_OpenGLContext.SwapBuffers( m_CallbackFactory.NewCallback( &MainInstance::DrawSelf ) );
        //m_OpenGLContext->FlushContext();
    }
}

void MainInstance::SetMouseLock(bool lock)
{
    if( lock )
    {
        LOGInfo( LOGTag, "SetMouseLock( true );\n" );

        m_GameWantsLockedMouse = true;
        LockMouse( m_CallbackFactory.NewCallback( &MainInstance::MouseLocked ) );
    }
    else
    {
        LOGInfo( LOGTag, "SetMouseLock( false );\n" );

        m_GameWantsLockedMouse = false;
        UnlockMouse();
    }
}

bool MainInstance::IsMouseLocked()
{
    return m_SystemMouseIsLocked;
}

void MainInstance::MouseLocked(int32_t somevaluethecallbackfactorywants)
{
    LOGInfo( LOGTag, "Mouse locked\n" );

    m_SystemMouseIsLocked = true;
}

void MainInstance::MouseLockLost()
{
    LOGInfo( LOGTag, "Mouse unlocked\n" );

    m_SystemMouseIsLocked = false;
}
