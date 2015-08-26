//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MYFWWinMainWx.h"
#include "Screenshot.h"

#include "../../Framework/MyFramework/SourceWidgets/EditorCommands.h"
#include "../../Framework/MyFramework/SourceWidgets/CommandStack.h"

bool g_EscapeButtonWillQuit;
bool g_CloseProgramRequested;

unsigned int g_GLCanvasIDActive = 0;
wxDataFormat* g_pMyDataFormat = 0;

GLViewTypes g_CurrentGLViewType;

MainFrame::MainFrame(wxWindow* parent)
: wxFrame( parent, -1, "wxWindow Title", wxPoint( -1, -1 ), wxSize( 1, 1 ), wxDEFAULT_FRAME_STYLE )
{
    m_MenuBar = 0;
    m_File = 0;
    m_Edit = 0;
    m_View = 0;
    m_Aspect = 0;

    for( int i=0; i<GLView_NumTypes; i++ )
        m_AspectMenuItems[i] = 0;

    m_pGLCanvas = 0;
    m_pCommandStack = 0;

    m_WindowX = -1;
    m_WindowY = -1;
    m_ClientWidth = 0;
    m_ClientHeight = 0;
    m_Maximized = false;

    g_pMyDataFormat = MyNew wxDataFormat( "MyFormat" );
}

MainFrame::~MainFrame()
{
    SAFE_DELETE( m_pGLCanvas );

    SAFE_DELETE( m_pCommandStack );

    SAFE_DELETE( g_pPanelWatch );
    SAFE_DELETE( g_pPanelMemory );
    SAFE_DELETE( g_pPanelObjectList );

    SAFE_DELETE( g_pMyDataFormat );

    // deinitialize the frame manager
    m_AUIManager.UnInit();
}

void MainFrame::InitFrame()
{
    int width, height;

    WinMain_GetClientSize( &width, &height, &g_CurrentGLViewType );

    if( m_ClientWidth == 0 )
        m_ClientWidth = width;
    if( m_ClientHeight == 0 )
        m_ClientHeight = height;

    // Create the menu bar
    {
        m_File = MyNew wxMenu;
        m_File->Append( wxID_EXIT, wxT("&Quit") );

        m_Edit = MyNew wxMenu;
        m_Edit->Append( myID_Undo, wxT("&Undo\tCtrl-Z") );
        m_Edit->Append( myID_Redo, wxT("&Redo\tCtrl-Y") );

        m_View = MyNew wxMenu;
        m_View->Append( myID_SavePerspective, wxT("&Save window layout") );
        m_View->Append( myID_LoadPerspective, wxT("&Load window layout") );
        m_View->Append( myID_ResetPerspective, wxT("&Reset window layout") );

        m_Aspect = MyNew wxMenu;
        m_AspectMenuItems[GLView_Full] = m_Aspect->AppendCheckItem( myID_GLViewType_Fill, wxT("&Fill\tAlt-1") );
        m_AspectMenuItems[GLView_Tall] = m_Aspect->AppendCheckItem( myID_GLViewType_Tall, wxT("&Tall\tAlt-2") );
        m_AspectMenuItems[GLView_Square] = m_Aspect->AppendCheckItem( myID_GLViewType_Square, wxT("&Square\tAlt-3") );
        m_AspectMenuItems[GLView_Wide] = m_Aspect->AppendCheckItem( myID_GLViewType_Wide, wxT("&Wide\tAlt-4") );

        m_MenuBar = MyNew wxMenuBar;
        m_MenuBar->Append( m_File, wxT("&File") );
        m_MenuBar->Append( m_Edit, wxT("&Edit") );
        m_MenuBar->Append( m_View, wxT("&View") );
        m_MenuBar->Append( m_Aspect, wxT("&Aspect") );
        SetMenuBar( m_MenuBar );

        // set the window position and client size after attaching the main menu.
        SetPosition( wxPoint( m_WindowX, m_WindowY ) );
        SetClientSize( m_ClientWidth, m_ClientHeight );
        Maximize( m_Maximized );

        Connect( wxEVT_MOVE, wxMoveEventHandler(MainFrame::OnMove) );
        Connect( wxEVT_SIZE, wxSizeEventHandler(MainFrame::OnSize) );
        Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MainFrame::OnCloseWindow) );

        Connect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnQuit) );
        
        Connect( myID_Undo, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_Redo, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );

        Connect( myID_SavePerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_LoadPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_ResetPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );

        Connect( myID_GLViewType_Fill, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Tall, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Square, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Wide, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
    }

    UpdateMenuItemStates();

    // notify wxAUI which frame to use
    m_AUIManager.SetManagedWindow( this );
}

void MainFrame::AddPanes()
{
    // create the opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvas = MyNew MainGLCanvas( (wxFrame*)this, args, 0, true );
    m_pGLCanvas->SetSize( 600, 600 );

    // Create the undo/redo command stack
    if( m_pCommandStack == 0 ) // created in EngineMainFrame?
        m_pCommandStack = MyNew CommandStack;

    // create all the panels we need
    g_pPanelWatch = MyNew PanelWatch( this, m_pCommandStack );
    g_pPanelMemory = MyNew PanelMemory( this );
    g_pPanelObjectList = MyNew PanelObjectList( this );

    // add the panes to the manager
    m_AUIManager.AddPane( m_pGLCanvas, wxAuiPaneInfo().Name("GLCanvas").Top().Caption("Game") );
    m_AUIManager.AddPane( g_pPanelWatch, wxAuiPaneInfo().Name("PanelWatch").Right().Caption("Watch").Layer(1) );
    m_AUIManager.AddPane( g_pPanelMemory, wxAuiPaneInfo().Name("PanelMemory").Right().Caption("Memory").Layer(1) );
    m_AUIManager.AddPane( g_pPanelObjectList, wxAuiPaneInfo().Name("PanelObjectList").Left().Caption("Objects").Layer(1) );
}

bool MainFrame::UpdateAUIManagerAndLoadPerspective()
{
    // tell the manager to "commit" all the changes just made
    m_AUIManager.Update();

    m_DefaultPerspectiveString = m_AUIManager.SavePerspective();
    
    FILE* file = 0;
#if MYFW_WINDOWS
    fopen_s( &file, "Layout.ini", "rb" );
#else
    file = fopen( "Layout.ini", "rb" );
#endif
    if( file )
    {
        char* string = MyNew char[10000];
        size_t len = fread( string, 1, 10000, file );
        string[len] = 0;
        fclose( file );
        m_SavedPerspectiveString = wxString( string );
        m_AUIManager.LoadPerspective( m_SavedPerspectiveString );
        delete[] string;

        return true;
    }

    return false;
}

bool MainFrame::OnClose()
{
    g_pPanelWatch->m_pTimer->Stop();
    g_pPanelObjectList->SelectObject( 0 );

    return true;
}

void MainFrame::OnMove(wxMoveEvent& event)
{
    if( this->IsMaximized() )
    {
        m_Maximized = true;
    }
    else
    {
        m_Maximized = false;

        m_WindowX = this->GetPosition().x;
        m_WindowY = this->GetPosition().y;
        m_ClientWidth = this->GetClientRect().width;
        m_ClientHeight = this->GetClientRect().height;
    }
}

void MainFrame::OnSize(wxSizeEvent& event)
{
    if( this->IsMaximized() )
    {
        m_Maximized = true;
    }
    else
    {
        m_Maximized = false;
        m_ClientWidth = this->GetClientRect().width;
        m_ClientHeight = this->GetClientRect().height;
    }
}

void MainFrame::OnCloseWindow(wxCloseEvent& event)
{
    if( OnClose() )
        event.Skip();
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
    case myID_Undo:
        if( m_pCommandStack->m_UndoStack.size() > 0 )
            m_pCommandStack->Undo( 1 );
        break;

    case myID_Redo:
        if( m_pCommandStack->m_RedoStack.size() > 0 )
            m_pCommandStack->Redo( 1 );
        break;

    case myID_SavePerspective:
        {
            m_SavedPerspectiveString = m_AUIManager.SavePerspective();

            FILE* file = 0;
#if MYFW_WINDOWS
      	    fopen_s( &file, "Layout.ini", "wb" );
#else
        	file = fopen( "Layout.ini", "wb" );
#endif
            if( file )
            {
                fprintf( file, "%s", (const char*)m_SavedPerspectiveString );
                fclose( file );
            }
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
        UpdateMenuItemStates();
        break;

    case myID_GLViewType_Tall:
        g_CurrentGLViewType = GLView_Tall;
        m_pGLCanvas->ResizeViewport();
        UpdateMenuItemStates();
        break;

    case myID_GLViewType_Square:
        g_CurrentGLViewType = GLView_Square;
        m_pGLCanvas->ResizeViewport();
        UpdateMenuItemStates();
        break;

    case myID_GLViewType_Wide:
        g_CurrentGLViewType = GLView_Wide;
        m_pGLCanvas->ResizeViewport();
        UpdateMenuItemStates();
        break;
    }
}

void MainFrame::OnKeyPressed(wxKeyEvent& event)
{
    int keycode = (int)event.m_keyCode;

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

    g_pGameCore->OnKeyDown( keycode, keycode );
}

void MainFrame::OnKeyReleased(wxKeyEvent& event)
{
    int keycode = (int)event.m_keyCode;

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

    //if( keycode < 512 )
    //    m_KeysDown[keycode] = false;
}

void MainFrame::ResizeViewport()
{
    m_pGLCanvas->ResizeViewport();
}

void MainFrame::UpdateMenuItemStates()
{
    for( int i=0; i<GLView_NumTypes; i++ )
    {
        if( m_AspectMenuItems[i] )
            m_AspectMenuItems[i]->Check( false );
    }

    if( m_AspectMenuItems[g_CurrentGLViewType] )
        m_AspectMenuItems[g_CurrentGLViewType]->Check( true );
}

#if MYFW_WINDOWS
IMPLEMENT_APP_NO_MAIN( MainApp );

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MarkAllExistingAllocationsAsStatic();

    wxDISABLE_DEBUG_SUPPORT();

    int ret = wxEntry( hInstance, hPrevInstance, lpCmdLine, nCmdShow ); //argc, argv);
    wxEntryCleanup();

    //_CrtDumpMemoryLeaks();
    //ValidateAllocations( true );

    return ret;
}
#else
IMPLEMENT_APP( MainApp );
#endif

MainApp* g_pMainApp = 0;

MainApp::~MainApp()
{
}

bool MainApp::OnInit()
{
#if _DEBUG && MYFW_WINDOWS
    OverrideJSONMallocFree();
#endif

    g_pMainApp = this;

    m_pMainFrame = WinMain_CreateMainFrame(); //MyNew MainFrame( 0 );
    m_pMainFrame->InitFrame();
    m_pMainFrame->AddPanes();
    m_pMainFrame->UpdateAUIManagerAndLoadPerspective();
    m_pMainFrame->ResizeViewport();
    m_pMainFrame->Show();

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created
    m_pMainFrame->m_pGLCanvas->MakeContextCurrent();

#if MYFW_WINDOWS
    OpenGL_InitExtensions();
#endif

    // Initialize sockets
#if MYFW_WINDOWS
    WSAData wsaData;
    int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if( code != 0 )
    {
        LOGError( LOGTag, "WSAStartup error:%d\n",code );
        return 0;
    }
#endif

    wxSize size = m_pMainFrame->m_pGLCanvas->GetSize();

    // Create and initialize our Game object.
    WinMain_CreateGameCore();
    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, size.x, size.y );
    g_pGameCore->OneTimeInit();

    m_pMainFrame->ResizeViewport();
    m_pMainFrame->OnPostInit();

    return true;
} 

int MainApp::FilterEvent(wxEvent& event)
{
    return wxApp::FilterEvent( event );
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

int g_GLContextRefCount = 0;
wxGLContext* m_GLContext = 0;

MainGLCanvas::MainGLCanvas(wxWindow* parent, int* args, unsigned int ID, bool tickgamecore)
: wxGLCanvas( parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
    if( m_GLContext == 0 )
        m_GLContext = MyNew wxGLContext( this );

    m_GLCanvasID = ID;
    m_TickGameCore = tickgamecore;

    g_GLContextRefCount++;

    m_MouseCaptured_ButtonsHeld = 0;
    m_MouseDown = false;

    // To avoid flashing on MSW
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );

    m_LastTimeTicked = MyTime_GetRunningTime();

    for( int i=0; i<512; i++ )
    {
        m_KeysDown[i] = 0;
    }
}

MainGLCanvas::~MainGLCanvas()
{
    SAFE_DELETE( g_pGameCore );

    g_GLContextRefCount--;
    if( g_GLContextRefCount == 0 )
        delete m_GLContext;
}

void MainGLCanvas::MakeContextCurrent()
{
    SetCurrent( *m_GLContext );
}

void MainGLCanvas::MouseMoved(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseMoved Event, %d, %d\n", event.m_x, event.m_y );

    //if( g_pGameCore )
    //    g_pGameCore->OnTouch( GCBA_Held, m_MouseDown?0:-1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftDown(wxMouseEvent& event)
{
    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseLeftDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    this->SetFocus();

    m_MouseDown = true;
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseLeftUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseDown = false;
    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseLeftWindow(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    //if( m_MouseDown )
    //{
    //    m_MouseDown = false;
    //    if( g_pGameCore )
    //        g_pGameCore->OnTouch( GCBA_Up, 0, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
    //}
}

void MainGLCanvas::MouseRightDown(wxMouseEvent& event)
{
    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseRightDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseRightUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseRightUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 1, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseMiddleDown(wxMouseEvent& event)
{
    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;
    
    //LOGInfo( LOGTag, "MainGLCanvas::MouseMiddleDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Down, 2, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseMiddleUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseMiddleUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Up, 2, (float)event.m_x, (float)event.m_y, 0, 0 ); // new press
}

void MainGLCanvas::MouseWheelMoved(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore )
        g_pGameCore->OnTouch( GCBA_Wheel, 0, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation, 0 ); // new press
}

void MainGLCanvas::KeyPressed(wxKeyEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    int keycode = (int)event.m_keyCode;

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
    {
        if( keycode >= 'A' && keycode <= 'Z' && m_KeysDown[MYKEYCODE_LSHIFT] == 0 && m_KeysDown[MYKEYCODE_RSHIFT] == 0 )
            g_pGameCore->OnKeyDown( keycode+32, keycode+32 );
        else
            g_pGameCore->OnKeyDown( keycode, keycode );
    }

    if( keycode < 512 )
        m_KeysDown[keycode] = true;
}

void MainGLCanvas::KeyReleased(wxKeyEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    int keycode = (int)event.m_keyCode;
    
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
    {
        if( keycode >= 'A' && keycode <= 'Z' && m_KeysDown[MYKEYCODE_LSHIFT] == 0 && m_KeysDown[MYKEYCODE_RSHIFT] == 0 )
            g_pGameCore->OnKeyUp( keycode+32, keycode+32 );
        else
            g_pGameCore->OnKeyUp( keycode, keycode );
    }

    if( keycode < 512 )
        m_KeysDown[keycode] = false;
}

void MainGLCanvas::Resized(wxSizeEvent& evt)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    //wxGLCanvas::OnSize(evt);
 
    //if( g_pGameCore )
    //    g_pGameCore->OnSurfaceChanged( 0, 0, evt.GetSize().x, evt.GetSize().y );

    m_CurrentGLViewWidth = evt.GetSize().x;
    m_CurrentGLViewHeight = evt.GetSize().y;

    ResizeViewport();

    Refresh();
}

void MainGLCanvas::ResizeViewport(bool clearhack)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    if( g_pGameCore == 0 )
        return;

    // bit of a hack, but since we might be only using part of the screen, clear both buffers to black
    if( clearhack )
    {
        for( int i=0; i<2; i++ )
        {
            glDisable( GL_SCISSOR_TEST );
            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            SwapBuffers();
        }
    }

    if( m_GLCanvasID == 1 )
    {
        // GameCore should pass this along to Editor_OnSurfaceChanged for any editor canvas (canvas 1 ATM).
        g_pGameCore->OnSurfaceChanged( 0, 0, m_CurrentGLViewWidth, m_CurrentGLViewHeight );
        return;
    }

    if( g_CurrentGLViewType == GLView_Full )
    {
        g_pGameCore->OnSurfaceChanged( 0, 0, m_CurrentGLViewWidth, m_CurrentGLViewHeight );
    }
    else if( g_CurrentGLViewType == GLView_Tall )
    {
        if( m_CurrentGLViewWidth-m_CurrentGLViewHeight/1.5f >= 0 )
        {
            g_pGameCore->OnSurfaceChanged( (unsigned int)((m_CurrentGLViewWidth-m_CurrentGLViewHeight/1.5f)/2), 0,
                (unsigned int)(m_CurrentGLViewHeight/1.5f), (unsigned int)(m_CurrentGLViewHeight) );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( 0, 0, m_CurrentGLViewWidth, m_CurrentGLViewHeight );
        }
    }
    else if( g_CurrentGLViewType == GLView_Square )
    {
        if( m_CurrentGLViewWidth < m_CurrentGLViewHeight )
        {
            g_pGameCore->OnSurfaceChanged( 0, (m_CurrentGLViewHeight-m_CurrentGLViewWidth)/2, m_CurrentGLViewWidth, m_CurrentGLViewWidth );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( (m_CurrentGLViewWidth-m_CurrentGLViewHeight)/2, 0, m_CurrentGLViewHeight, m_CurrentGLViewHeight );
        }
    }
    else if( g_CurrentGLViewType == GLView_Wide )
    {
        if( (m_CurrentGLViewHeight-m_CurrentGLViewWidth/1.5f)/2 >= 0 )
        {
            g_pGameCore->OnSurfaceChanged( 0, (unsigned int)((m_CurrentGLViewHeight-m_CurrentGLViewWidth/1.5f)/2),
                (unsigned int)(m_CurrentGLViewWidth), (unsigned int)(m_CurrentGLViewWidth/1.5f) );
        }
        else
        {
            g_pGameCore->OnSurfaceChanged( 0, 0, m_CurrentGLViewWidth, m_CurrentGLViewHeight );
        }
    }
}

void MainGLCanvas::Idle(wxIdleEvent& evt)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    Refresh();
}

void MainGLCanvas::Render(wxPaintEvent& evt)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    Draw();
}

void MainGLCanvas::Draw()
{
    g_GLCanvasIDActive = m_GLCanvasID;

    m_GLContext->SetCurrent( *this );
    //SetCurrent( *m_GLContext );
    wxPaintDC( this );

    if( g_pGameCore )
    {
        // Generate keyheld messages:
        //for( int i=0; i<GCBI_NumButtons; i++ )
        //{
        //    if( m_ButtonsHeld[i] )
        //        g_pGameCore->OnButtons( GCBA_Held, (GameCoreButtonIDs)i );
        //}

        // deal with losing and gaining focus;
        if( g_pMainApp->m_pMainFrame->IsActive() )
        {
            if( g_pMainApp->m_HasFocus == false )
            {
                //LOGInfo( LOGTag, "g_pMainApp gained focus\n" );
                g_pGameCore->OnFocusGained();
            }

            g_pMainApp->m_HasFocus = true;
        }
        else
        {
            if( g_pMainApp->m_HasFocus == true )
            {
                //LOGInfo( LOGTag, "g_pMainApp lost focus\n" );
                g_pGameCore->OnFocusLost();
            }

            g_pMainApp->m_HasFocus = false;
        }

        if( m_TickGameCore )
        {
            double currtime = MyTime_GetRunningTime();
            double timepassed = currtime - m_LastTimeTicked;
            m_LastTimeTicked = currtime;

            g_UnpausedTime += g_pGameCore->Tick( timepassed );
        }

        int currentframebuffer;
        glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentframebuffer );
        MyAssert( currentframebuffer == 0 );

        //if( m_GLCanvasID == 0 )
        {
            g_pGameCore->OnDrawFrame( m_GLCanvasID );
            g_pGameCore->OnDrawFrameDone();
        }
    }

    SwapBuffers();
}
