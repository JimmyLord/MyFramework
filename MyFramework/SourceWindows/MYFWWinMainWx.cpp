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

#include "CommonHeader.h"
#include "MYFWWinMainWx.h"
#include "Screenshot.h"

bool g_EscapeButtonWillQuit;
bool g_CloseProgramRequested;

bool m_KeysDown[512];

bool MYFW_GetKey(int value)
{
    assert( value >= 0 && value < 512 );
    return m_KeysDown[value];
}

GLViewTypes g_CurrentGLViewType;
int g_CurrentGLViewWidth;
int g_CurrentGLViewHeight;

MainFrame::MainFrame(wxWindow* parent)
: wxFrame( parent, -1, _("wxWindow Title"), wxPoint( 0, 0 ), wxSize( 1, 1 ), wxDEFAULT_FRAME_STYLE )
{
    int width, height;
    WinMain_GetClientSize( &width, &height, &g_CurrentGLViewType );

    SetClientSize( width, height );

    for( int i=0; i<512; i++ )
    {
        m_KeysDown[i] = 0;
    }

    // Create the menu bar
    {
        m_File = MyNew wxMenu;
        m_File->Append( wxID_EXIT, wxT("&Quit") );

        m_View = MyNew wxMenu;
        m_View->Append( myID_SavePerspective, wxT("&Save window layout") );
        m_View->Append( myID_LoadPerspective, wxT("&Load window layout") );
        m_View->Append( myID_ResetPerspective, wxT("&Reset window layout") );

        m_Aspect = MyNew wxMenu;
        m_Aspect->Append( myID_GLViewType_Fill, wxT("&Fill") );
        m_Aspect->Append( myID_GLViewType_Tall, wxT("&Tall") );
        m_Aspect->Append( myID_GLViewType_Square, wxT("&Square") );
        m_Aspect->Append( myID_GLViewType_Wide, wxT("&Wide") );

        m_MenuBar = MyNew wxMenuBar;
        m_MenuBar->Append( m_File, wxT("&File") );
        m_MenuBar->Append( m_View, wxT("&View") );
        m_MenuBar->Append( m_Aspect, wxT("&Aspect") );
        SetMenuBar( m_MenuBar );

        Connect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnQuit) );
        
        Connect( myID_SavePerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_LoadPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_ResetPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );

        Connect( myID_GLViewType_Fill, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Tall, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Square, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Wide, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
    }

    // create the opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvas = MyNew MainGLCanvas( (wxFrame*)this, args );

    // create all the panels we need
    g_pPanelWatch = MyNew PanelWatch( this );
    g_pPanelMemory = MyNew PanelMemory( this );
    g_pPanelObjectList = MyNew PanelObjectList( this );

    // notify wxAUI which frame to use
    m_AUIManager.SetManagedWindow( this );

    // add the panes to the manager
    m_AUIManager.AddPane( m_pGLCanvas, wxAuiPaneInfo().Name("GLCanvas").Centre().Caption("GLCanvas").CaptionVisible(false) );
    m_AUIManager.AddPane( g_pPanelWatch, wxAuiPaneInfo().Name("PanelWatch").Right().Caption("Watch") );
    m_AUIManager.AddPane( g_pPanelMemory, wxAuiPaneInfo().Name("PanelMemory").Right().Caption("Memory") );
    m_AUIManager.AddPane( g_pPanelObjectList, wxAuiPaneInfo().Name("PanelObjectList").Left().Caption("Objects") );

    // tell the manager to "commit" all the changes just made
    m_AUIManager.Update();

    m_DefaultPerspectiveString = m_AUIManager.SavePerspective();
#pragma warning (disable : 4996)
    FILE* file = fopen( "Layout.ini", "rb" );
#pragma warning (default : 4996)
    if( file )
    {
        char* string = MyNew char[10000];
        int len = fread( string, 1, 10000, file );
        string[len] = 0;
        fclose( file );
        m_SavedPerspectiveString = wxString( string );
        m_AUIManager.LoadPerspective( m_SavedPerspectiveString );
        delete[] string;
    }
}

MainFrame::~MainFrame()
{
    SAFE_DELETE( g_pPanelWatch );
    SAFE_DELETE( g_pPanelMemory );
    SAFE_DELETE( g_pPanelObjectList );

    // deinitialize the frame manager
    m_AUIManager.UnInit();
}

void MainFrame::OnQuit(wxCommandEvent& event)
{
    Close( true );
}

void MainFrame::OnMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myID_SavePerspective:
        {
            m_SavedPerspectiveString = m_AUIManager.SavePerspective();
#pragma warning (disable : 4996)
            FILE* file = fopen( "Layout.ini", "wb" );
#pragma warning (default : 4996)
            fprintf( file, m_SavedPerspectiveString );
            fclose( file );
        }
        break;

    case myID_LoadPerspective:
        m_AUIManager.LoadPerspective( m_SavedPerspectiveString );
        break;

    case myID_ResetPerspective:
        m_AUIManager.LoadPerspective( m_DefaultPerspectiveString );
        break;

    case myID_GLViewType_Fill:
        g_CurrentGLViewType = GLView_Full;
        m_pGLCanvas->ResizeViewport();
        break;

    case myID_GLViewType_Tall:
        g_CurrentGLViewType = GLView_Tall;
        m_pGLCanvas->ResizeViewport();
        break;

    case myID_GLViewType_Square:
        g_CurrentGLViewType = GLView_Square;
        m_pGLCanvas->ResizeViewport();
        break;

    case myID_GLViewType_Wide:
        g_CurrentGLViewType = GLView_Wide;
        m_pGLCanvas->ResizeViewport();
        break;
    }
}

void MainFrame::OnKeyPressed(wxKeyEvent& event)
{
    int keycode = event.m_keyCode;

    if( keycode == '1' )
    {
        g_CurrentGLViewType = GLView_Full;
        m_pGLCanvas->ResizeViewport();
    }
    if( keycode == '2' )
    {
        g_CurrentGLViewType = GLView_Tall;
        m_pGLCanvas->ResizeViewport();
    }
    if( keycode == '3' )
    {
        g_CurrentGLViewType = GLView_Square;
        m_pGLCanvas->ResizeViewport();
    }
    if( keycode == '4' )
    {
        g_CurrentGLViewType = GLView_Wide;
        m_pGLCanvas->ResizeViewport();
    }

    if( keycode == 8 )
        keycode = MYKEYCODE_BACKSPACE;
    if( keycode == 314 )
        keycode = MYKEYCODE_LEFT;
    if( keycode == 316 )
        keycode = MYKEYCODE_RIGHT;
    if( keycode == 315 )
        keycode = MYKEYCODE_UP;
    if( keycode == 317 )
        keycode = MYKEYCODE_DOWN;

    //if( g_pGameCore )
    //    g_pGameCore->OnKeyDown( keycode, keycode );

    //if( keycode < 512 )
    //    m_KeysDown[keycode] = true;
}

void MainFrame::OnKeyReleased(wxKeyEvent& event)
{
    int keycode = event.m_keyCode;

    if( keycode == 8 )
        keycode = MYKEYCODE_BACKSPACE;
    if( keycode == 314 )
        keycode = MYKEYCODE_LEFT;
    if( keycode == 316 )
        keycode = MYKEYCODE_RIGHT;
    if( keycode == 315 )
        keycode = MYKEYCODE_UP;
    if( keycode == 317 )
        keycode = MYKEYCODE_DOWN;

    //if( g_pGameCore )
    //    g_pGameCore->OnKeyUp( keycode, keycode );

    //if( keycode < 512 )
    //    m_KeysDown[keycode] = false;
}

IMPLEMENT_APP( MainApp );

bool MainApp::OnInit()
{
    m_pMainFrame = WinMain_CreateMainFrame(); //MyNew MainFrame( 0 );
    m_pMainFrame->Show();

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created
    m_pMainFrame->m_pGLCanvas->MakeContextCurrent();

    OpenGL_InitExtensions();

    // Initialize sockets
    WSAData wsaData;
    int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if( code != 0 )
    {
        LOGError( LOGTag, "WSAStartup error:%d\n",code );
        return 0;
    }

    // Create and initialize our Game object.
    WinMain_CreateGameCore();

    wxSize size = m_pMainFrame->m_pGLCanvas->GetSize();

    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, size.x, size.y );
    g_pGameCore->OneTimeInit();

    m_pMainFrame->m_pGLCanvas->ResizeViewport();

    return true;
} 

int MainApp::FilterEvent(wxEvent& event)
{
    //return wxApp::FilterEvent( event );
    int ret = wxApp::FilterEvent( event );

    if( ret == 1 )
        return 1;

    if( event.GetEventType() == wxEVT_KEY_DOWN )
    {
        if( ((wxKeyEvent&)event).GetModifiers() & (wxMOD_CONTROL|wxMOD_ALT) )
        {
            m_pMainFrame->OnKeyPressed( (wxKeyEvent&)event );
            return 1;
        }
    }
 
    if( event.GetEventType() == wxEVT_KEY_UP )
    {
        if( ((wxKeyEvent&)event).GetModifiers() & (wxMOD_CONTROL|wxMOD_ALT) )
        {
            m_pMainFrame->OnKeyReleased( (wxKeyEvent&)event );
            return 1;
        }
    }

    return -1;
}

BEGIN_EVENT_TABLE(MainGLCanvas, wxGLCanvas)
    EVT_MOTION(MainGLCanvas::MouseMoved)
    EVT_LEFT_DOWN(MainGLCanvas::MouseLeftDown)
    EVT_LEFT_UP(MainGLCanvas::MouseLeftUp)
    EVT_RIGHT_DOWN(MainGLCanvas::MouseRightDown)
    EVT_RIGHT_UP(MainGLCanvas::MouseRightUp)
    EVT_MIDDLE_DOWN(MainGLCanvas::MouseMiddleDown)
    EVT_MIDDLE_UP(MainGLCanvas::MouseMiddleUp)
    EVT_LEAVE_WINDOW(MainGLCanvas::MouseLeftWindow)
    EVT_SIZE(MainGLCanvas::Resized)
    EVT_KEY_DOWN(MainGLCanvas::KeyPressed)
    EVT_KEY_UP(MainGLCanvas::KeyReleased)
    EVT_MOUSEWHEEL(MainGLCanvas::MouseWheelMoved)
    EVT_PAINT(MainGLCanvas::Render)
    EVT_IDLE(MainGLCanvas::Idle)
END_EVENT_TABLE()

MainGLCanvas::MainGLCanvas(wxWindow* parent, int* args) :
wxGLCanvas( parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
    m_GLContext = MyNew wxGLContext( this );
    m_MouseDown = false;

    // To avoid flashing on MSW
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}

MainGLCanvas::~MainGLCanvas()
{
    SAFE_DELETE( g_pGameCore );
    delete m_GLContext;
}

void MainGLCanvas::MakeContextCurrent()
{
    SetCurrent( *m_GLContext );
}

void MainGLCanvas::MouseMoved(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Held, -1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftDown(wxMouseEvent& event)
{
    this->SetFocus();

    //m_MouseDown = true;
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftUp(wxMouseEvent& event)
{
    //m_MouseDown = false;
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftWindow(wxMouseEvent& event)
{
    //if( m_MouseDown )
    //{
    //    m_MouseDown = false;
    //    if( g_pGameCore )
    //        g_pGameCore->OnTouch( GCBA_Up, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
    //}
}

void MainGLCanvas::MouseRightDown(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseRightUp(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseMiddleDown(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 2, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseMiddleUp(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 2, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseWheelMoved(wxMouseEvent& event)
{
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Wheel, 0, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation, 0 ); // new press
}

void MainGLCanvas::KeyPressed(wxKeyEvent& event)
{
    int keycode = event.m_keyCode;

    //if( keycode == '1' )
    //{
    //    g_CurrentGLViewType = GLView_Full;
    //    ResizeViewport();
    //}
    //if( keycode == '2' )
    //{
    //    g_CurrentGLViewType = GLView_Tall;
    //    ResizeViewport();
    //}
    //if( keycode == '3' )
    //{
    //    g_CurrentGLViewType = GLView_Square;
    //    ResizeViewport();
    //}
    //if( keycode == '4' )
    //{
    //    g_CurrentGLViewType = GLView_Wide;
    //    ResizeViewport();
    //}

    if( keycode == 8 )
        keycode = MYKEYCODE_BACKSPACE;
    if( keycode == 314 )
        keycode = MYKEYCODE_LEFT;
    if( keycode == 316 )
        keycode = MYKEYCODE_RIGHT;
    if( keycode == 315 )
        keycode = MYKEYCODE_UP;
    if( keycode == 317 )
        keycode = MYKEYCODE_DOWN;

    if( g_pGameCore )
        g_pGameCore->OnKeyDown( keycode, keycode );

    if( keycode < 512 )
        m_KeysDown[keycode] = true;
}

void MainGLCanvas::KeyReleased(wxKeyEvent& event)
{
    int keycode = event.m_keyCode;
    
    if( keycode == 8 )
        keycode = MYKEYCODE_BACKSPACE;
    if( keycode == 314 )
        keycode = MYKEYCODE_LEFT;
    if( keycode == 316 )
        keycode = MYKEYCODE_RIGHT;
    if( keycode == 315 )
        keycode = MYKEYCODE_UP;
    if( keycode == 317 )
        keycode = MYKEYCODE_DOWN;

    if( g_pGameCore )
        g_pGameCore->OnKeyUp( keycode, keycode );

    if( keycode < 512 )
        m_KeysDown[keycode] = false;
}

void MainGLCanvas::Resized(wxSizeEvent& evt)
{
//    wxGLCanvas::OnSize(evt);
 
    if( g_pGameCore )
        g_pGameCore->OnSurfaceChanged( 0, 0, evt.GetSize().x, evt.GetSize().y );

    g_CurrentGLViewWidth = evt.GetSize().x;
    g_CurrentGLViewHeight = evt.GetSize().y;

    ResizeViewport();

    Refresh();
}

void MainGLCanvas::ResizeViewport()
{
    if( g_pGameCore == 0 )
        return;

    // bit of a hack, but since we might be only using part of the screen, clear both buffers to black
    for( int i=0; i<2; i++ )
    {
        glDisable( GL_SCISSOR_TEST );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        SwapBuffers();
    }

    if( g_CurrentGLViewType == GLView_Full )
    {
        g_pGameCore->OnSurfaceChanged( 0, 0, g_CurrentGLViewWidth, g_CurrentGLViewHeight );
    }
    else if( g_CurrentGLViewType == GLView_Tall )
    {
        if( g_CurrentGLViewWidth-g_CurrentGLViewHeight/1.5f >= 0 )
        {
            g_pGameCore->OnSurfaceChanged( (g_CurrentGLViewWidth-g_CurrentGLViewHeight/1.5f)/2, 0,
                g_CurrentGLViewHeight/1.5f, g_CurrentGLViewHeight );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( 0, 0, g_CurrentGLViewWidth, g_CurrentGLViewHeight );
        }
    }
    else if( g_CurrentGLViewType == GLView_Square )
    {
        if( g_CurrentGLViewWidth < g_CurrentGLViewHeight )
        {
            g_pGameCore->OnSurfaceChanged( 0, (g_CurrentGLViewHeight-g_CurrentGLViewWidth)/2, g_CurrentGLViewWidth, g_CurrentGLViewWidth );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( (g_CurrentGLViewWidth-g_CurrentGLViewHeight)/2, 0, g_CurrentGLViewHeight, g_CurrentGLViewHeight );
        }
    }
    else if( g_CurrentGLViewType == GLView_Wide )
    {
        if( (g_CurrentGLViewHeight-g_CurrentGLViewWidth/1.5f)/2 >= 0 )
        {
            g_pGameCore->OnSurfaceChanged( 0, (g_CurrentGLViewHeight-g_CurrentGLViewWidth/1.5f)/2,
                g_CurrentGLViewWidth, g_CurrentGLViewWidth/1.5f );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( 0, 0, g_CurrentGLViewWidth, g_CurrentGLViewHeight );
        }
    }
}

void MainGLCanvas::Idle(wxIdleEvent& evt)
{
    Refresh();
}

void MainGLCanvas::Render(wxPaintEvent& evt)
{
    Draw();
}

void MainGLCanvas::Draw()
{
    wxGLCanvas::SetCurrent( *m_GLContext );
    wxPaintDC( this );

    static double lasttime = MyTime_GetSystemTime();

    if( g_pGameCore )
    {
        double currtime = MyTime_GetSystemTime();
        double timepassed = currtime - lasttime;
        lasttime = currtime;

        g_pGameCore->Tick( timepassed );
        g_pGameCore->OnDrawFrame();
        g_pGameCore->OnDrawFrameDone();

        SwapBuffers();
    }
}
