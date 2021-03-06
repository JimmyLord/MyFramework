//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include "MYFWMainWx.h"

#include "../SourceEditor/EditorCommands.h"
#include "../SourceEditor/CommandStack.h"

#define TESTING_FRAGCOORDISSUE 0

bool g_EscapeButtonWillQuit;
bool g_CloseProgramRequested;

unsigned int g_GLCanvasIDActive = 0;
wxDataFormat* g_pMyDataFormat = 0;

GLViewTypes g_CurrentGLViewType;

const char* g_DefaultEditorWindowTypeMenuLabels[EditorWindow_NumTypes] =
{
    "&Game View",
    "&Object List Panel",
    "&Watch Panel",
    "&Files Panel",
};

void SetMouseLock(bool lock)
{
    if( lock )
    {
        // Request a lock, mouse will actually lock the next time the window is left-clicked.
        g_pMainApp->m_pMainFrame->m_pGLCanvas->RequestMouseLock();
    }
    else
    {
        g_pMainApp->m_pMainFrame->m_pGLCanvas->ReleaseMouseLock();
    }
}

bool IsMouseLocked()
{
    return g_pMainApp->m_pMainFrame->m_pGLCanvas->IsMouseLocked();
}

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
        m_View->Append( myID_View_SavePerspective, wxT("&Save window layout") );
        m_View->Append( myID_View_LoadPerspective, wxT("&Load window layout") );
        m_View->Append( myID_View_ResetPerspective, wxT("&Reset window layout") );
        m_EditorWindows = MyNew wxMenu;
        for( int i=0; i<EditorWindow_NumTypes; i++ )
        {
            m_EditorWindowOptions[i] = m_EditorWindows->Append( myID_EditorWindow_FirstWindow + i, g_DefaultEditorWindowTypeMenuLabels[i], wxEmptyString );
            Connect( myID_EditorWindow_FirstWindow + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        }
        m_View->Append( myID_View_EditorWindows, "Editor Windows", m_EditorWindows );

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

        Connect( myID_View_SavePerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_View_LoadPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_View_ResetPerspective, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );

        Connect( myID_GLViewType_Fill, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Tall, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Square, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
        Connect( myID_GLViewType_Wide, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OnMenu) );
    }

    UpdateMenuItemStates();

    // notify wxAUI which frame to use
    m_AUIManager.SetManagedWindow( this );
    m_AUIManager.SetFlags( m_AUIManager.GetFlags() | wxAUI_MGR_ALLOW_ACTIVE_PANE );
}

void MainFrame::ProcessAllGLCanvasInputEventQueues()
{
    m_pGLCanvas->ProcessInputEventQueue();
}

void MainFrame::AddPanes()
{
    // create the opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvas = MyNew MainGLCanvas( (wxFrame*)this, args, 0, true );
#if MYFW_WINDOWS
    m_pGLCanvas->RequestRawMouseAccess();
#endif
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
    g_pPanelWatch->StopTimer();
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
        if( m_pCommandStack->GetUndoStackSize() > 0 )
            m_pCommandStack->Undo( 1 );
        break;

    case myID_Redo:
        if( m_pCommandStack->GetRedoStackSize() > 0 )
            m_pCommandStack->Redo( 1 );
        break;

    case myID_View_SavePerspective:
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

    case myID_View_LoadPerspective:
        m_AUIManager.LoadPerspective( m_SavedPerspectiveString );
        break;

    case myID_View_ResetPerspective:
        m_AUIManager.LoadPerspective( m_DefaultPerspectiveString );
        break;

    case myID_EditorWindow_Game:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( m_pGLCanvas );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
        break;

    case myID_EditorWindow_PanelWatch:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( g_pPanelWatch );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
        break;

    case myID_EditorWindow_PanelMemory:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( g_pPanelMemory );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
        break;

    case myID_EditorWindow_PanelObjectList:
        {
            wxAuiPaneInfo& paneinfo = m_AUIManager.GetPane( g_pPanelObjectList );
            paneinfo.Show( !paneinfo.IsShown() );
            m_AUIManager.Update();
        }
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

//void MainFrame::OnKeyPressed(wxKeyEvent& event)
//{
//    LOGInfo( "wxEvents", "MainFrame::OnKeyPressed() was hit" );
//}
//
//void MainFrame::OnKeyReleased(wxKeyEvent& event)
//{
//    LOGInfo( "wxEvents", "MainFrame::OnKeyReleased() was hit" );
//}

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
    MyMemory_MarkAllExistingAllocationsAsStatic();

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

MainApp::MainApp()
{
    m_pMainFrame = 0;
    m_HasFocus = false;
}

MainApp::~MainApp()
{
}

bool MainApp::OnInit()
{
#if _DEBUG && MYFW_WINDOWS
    OverrideJSONMallocFree();
#endif

    g_pMainApp = this;

    m_pMainFrame = WinMain_CreateMainFrame();
    m_pMainFrame->InitFrame();
    m_pMainFrame->AddPanes();
    m_pMainFrame->UpdateAUIManagerAndLoadPerspective();
    m_pMainFrame->ResizeViewport();
    m_pMainFrame->Show();

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created
    m_pMainFrame->m_pGLCanvas->MakeContextCurrent();

#if !MYFW_OSX
    OpenGL_InitExtensions();
#endif

#if MYFW_WINDOWS
    WGL_InitExtensions();
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
#if !TESTING_FRAGCOORDISSUE
    WinMain_CreateGameCore();
    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, size.x, size.y );
    g_pGameCore->OneTimeInit();
    g_pGameCore->SetCommandStack( g_pMainApp->m_pMainFrame->m_pCommandStack );
#endif

    m_pMainFrame->ResizeViewport();
    m_pMainFrame->OnPostInit();

    return true;
} 

int MainApp::FilterEvent(wxEvent& event)
{
    int ret = wxApp::FilterEvent( event );

    if( event.GetEventType() == wxEVT_SHOW )
    {
        if( m_pMainFrame && event.GetEventObject() == m_pMainFrame->m_pGLCanvas )
        {
            m_pMainFrame->OnGLCanvasShownOrHidden( false );
        }
    }

    if( event.GetEventType() == wxEVT_DESTROY )
    {
        wxWindowDestroyEvent& destroyevent = (wxWindowDestroyEvent&)event;
        if( destroyevent.GetWindow() == m_pMainFrame )
            m_pMainFrame = 0;
    }

    if( ret != Event_Processed )
    {
        // only pass events to the frame if a textctrl isn't in focus.
        if( m_pMainFrame )
        {
            wxWindow* pWindowInFocus = m_pMainFrame->FindFocus();
            if( dynamic_cast<wxTextCtrl*>(pWindowInFocus) == 0 )//|| dynamic_cast<wxStyledTextCtrl*>(pWindowInFocus) == 0 )
            {
                if( m_pMainFrame->FilterGlobalEvents( event ) )
                    return Event_Processed;
            }
        }
    }

    return ret;
}

BEGIN_EVENT_TABLE(MainGLCanvas, wxGLCanvas)
    EVT_MOTION(MainGLCanvas::MouseMoved)
    EVT_MOUSEWHEEL(MainGLCanvas::MouseWheelMoved)
    EVT_LEFT_DOWN(MainGLCanvas::MouseLeftDown)
    EVT_LEFT_DCLICK(MainGLCanvas::MouseLeftDoubleClick)
    EVT_LEFT_UP(MainGLCanvas::MouseLeftUp)
    EVT_RIGHT_DOWN(MainGLCanvas::MouseRightDown)
    EVT_RIGHT_UP(MainGLCanvas::MouseRightUp)
    EVT_RIGHT_DCLICK(MainGLCanvas::MouseRightDoubleClick)
    EVT_MIDDLE_DOWN(MainGLCanvas::MouseMiddleDown)
    EVT_MIDDLE_UP(MainGLCanvas::MouseMiddleUp)
    EVT_MIDDLE_DCLICK(MainGLCanvas::MouseMiddleDoubleClick)
    EVT_LEAVE_WINDOW(MainGLCanvas::MouseLeftWindow)
    EVT_MOUSE_CAPTURE_LOST(MainGLCanvas::MouseCaptureLost)
    EVT_SIZE(MainGLCanvas::Resized)
    EVT_KEY_DOWN(MainGLCanvas::KeyPressed)
    EVT_KEY_UP(MainGLCanvas::KeyReleased)
    EVT_CHAR(MainGLCanvas::KeyChar)
    EVT_PAINT(MainGLCanvas::Render)
    EVT_IDLE(MainGLCanvas::Idle)
END_EVENT_TABLE()

int g_GLContextRefCount = 0;
wxGLContext* g_pGLContext = 0;

MainGLCanvas::MainGLCanvas(wxWindow* parent, int* args, unsigned int ID, bool tickgamecore)
: wxGLCanvas( parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
    if( g_pGLContext == 0 )
        g_pGLContext = MyNew wxGLContext( this );

    m_GLCanvasID = ID;
    m_TickGameCore = tickgamecore;

    g_GLContextRefCount++;

    // To avoid flashing on MSW
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );

    m_LastTimeTicked = MyTime_GetRunningTime();

    for( int i=0; i<512; i++ )
    {
        m_KeysDown[i] = 0;
    }
    m_MouseCaptured_ButtonsHeld = 0;
    m_MouseButtonStates = 0;

    // Vars for mouse lock.
    m_RawMouseInputInitialized = false;

    m_GameWantsLockedMouse = false;
    m_SystemMouseIsLocked = false;

    m_MouseXPositionWhenLocked = -1;
    m_MouseYPositionWhenLocked = -1;
}

MainGLCanvas::~MainGLCanvas()
{
    SAFE_DELETE( g_pGameCore );

    g_GLContextRefCount--;
    if( g_GLContextRefCount == 0 )
        delete g_pGLContext;
}

//WXLRESULT MainGLCanvas::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
//{
//    int bp = 1;
//
//    return wx
//}

#if MYFW_WINDOWS
void MainGLCanvas::RequestRawMouseAccess()
{
    // Only attempt this once.
    if( m_RawMouseInputInitialized == false )
    {
        // Request raw mouse access on Windows.
        RAWINPUTDEVICE device;

        device.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        device.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
        device.dwFlags = 0;
        device.hwndTarget = 0; // All windows in this app?

        BOOL ret = RegisterRawInputDevices( &device, 1, sizeof(RAWINPUTDEVICE) );
        if( ret == false )
        {
            // Registration failed. Call GetLastError for the cause of the error
            LOGError( LOGTag, "Failed to grab raw mouse device: %d\n", GetLastError() );
        }
        else
        {
            m_RawMouseInputInitialized = true;
        }
    }
}

bool MainGLCanvas::MSWHandleMessage(WXLRESULT* result, WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if( message == WM_INPUT )
    {
        //LOGInfo( LOGTag, "WM_INPUT\n" );

        unsigned int size = sizeof( RAWINPUT );
        RAWINPUT rawinput;

#if _DEBUG
        unsigned int sizewanted;
        GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, 0, &sizewanted, sizeof(RAWINPUTHEADER) );
        MyAssert( size == sizewanted );
#endif

        GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, &rawinput, &size, sizeof(RAWINPUTHEADER) );

        // Pass mouse position diffs to event queue.
        if( rawinput.header.dwType == RIM_TYPEMOUSE )
        {
            // Only send mouse movement messages (position diffs) if the system mouse is locked
            if( m_SystemMouseIsLocked )
            {
                //LOGInfo( "RawMouse", "%f (%d, %d)\n", MyTime_GetSystemTime(), rawinput.data.mouse.lLastX, rawinput.data.mouse.lLastY );

                float xdiff = (float)rawinput.data.mouse.lLastX;
                float ydiff = (float)rawinput.data.mouse.lLastY;
                WarpPointer( m_MouseXPositionWhenLocked, m_MouseYPositionWhenLocked );

                if( xdiff != 0 || ydiff != 0 )
                {
                    if( m_MouseButtonStates & 1 << 0 )
                        m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 0, xdiff, ydiff, m_MousePosition.z ) );
                    if( m_MouseButtonStates & 1 << 1 )
                        m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 1, xdiff, ydiff, m_MousePosition.z ) );
                    if( m_MouseButtonStates & 1 << 2 )
                        m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 2, xdiff, ydiff, m_MousePosition.z ) );
                }
            }
        }

        return true;
    }

    return wxWindowMSW::MSWHandleMessage( result, message, wParam, lParam );
}
#endif

void MainGLCanvas::MakeContextCurrent()
{
    SetCurrent( *g_pGLContext );
}

void MainGLCanvas::MouseMoved(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseMoved Event, %d, %d\n", event.m_x, event.m_y );

    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );

    // Game window wants mouse locked.
    if( m_GameWantsLockedMouse || m_SystemMouseIsLocked )
    {
        // This case should be handled by WM_INPUT nearby.
        // This is fallback if raw mouse input fails to initialize and for non-windows builds.

        // Only send mouse movement messages (position diffs) if the system mouse is locked.
        if( m_RawMouseInputInitialized == false && m_SystemMouseIsLocked )
        {
#if MYFW_WINDOWS
            MyAssert( false ); // Windows builds should use raw mouse input.
#endif

            float xdiff = (float)event.m_x - m_MouseXPositionWhenLocked;
            float ydiff = (float)event.m_y - m_MouseYPositionWhenLocked;
            WarpPointer( m_MouseXPositionWhenLocked, m_MouseYPositionWhenLocked );

            if( xdiff != 0 || ydiff != 0 )
            {
                if( m_MouseButtonStates == 0 )
                    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, -1, xdiff, ydiff, (float)event.m_wheelRotation ) );
                if( m_MouseButtonStates & 1 << 0 )
                    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 0, xdiff, ydiff, (float)event.m_wheelRotation ) );
                if( m_MouseButtonStates & 1 << 1 )
                    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 1, xdiff, ydiff, (float)event.m_wheelRotation ) );
                if( m_MouseButtonStates & 1 << 2 )
                    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_RelativeMovement, 2, xdiff, ydiff, (float)event.m_wheelRotation ) );
            }
        }
    }
    else
    {
        // Send mouse positions if this window's mouse cursor isn't locked
        if( m_SystemMouseIsLocked == false )
        {
            if( m_MouseButtonStates == 0 )
                m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Held, -1, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
            if( m_MouseButtonStates & 1 << 0 )
                m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Held, 0, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
            if( m_MouseButtonStates & 1 << 1 )
                m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Held, 1, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
            if( m_MouseButtonStates & 1 << 2 )
                m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Held, 2, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
        }
    }
}

void MainGLCanvas::MouseLeftDown(wxMouseEvent& event)
{
    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseLeftDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    // Lock the mouse if the game requested it.
    if( m_GameWantsLockedMouse )
    {
        LockMouse( true );

        // Overwrite the x,y in the event to avoid a pop in mouse values in GCBA_Down message below.
        event.m_x = m_MouseXPositionWhenLocked;
        event.m_y = m_MouseYPositionWhenLocked;
    }

    if( g_GLCanvasIDActive != 0 || (this->HasFocus() == true && (m_GameWantsLockedMouse == false || m_SystemMouseIsLocked == true)) )
    {
        m_MouseButtonStates |= (1 << 0);
        m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
        m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Down, 0, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
    }

    this->SetFocus();
}

void MainGLCanvas::MouseLeftDoubleClick(wxMouseEvent& event)
{
    MouseLeftDown( event );
}

void MainGLCanvas::MouseLeftUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseLeftUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseButtonStates &= ~(1 << 0);
    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Up, 0, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::MouseLeftWindow(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    LockMouse( false );
}

void MainGLCanvas::MouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    LockMouse( false );
    m_MouseCaptured_ButtonsHeld = 0;
    m_MouseButtonStates = 0;
}

void MainGLCanvas::MouseRightDown(wxMouseEvent& event)
{
    SetFocus();

    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;

    //LOGInfo( LOGTag, "MainGLCanvas::MouseRightDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseButtonStates |= (1 << 1);
    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Down, 1, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::MouseRightDoubleClick(wxMouseEvent& event)
{
    MouseRightDown( event );
}

void MainGLCanvas::MouseRightUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseRightUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseButtonStates &= ~(1 << 1);
    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Up, 1, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::MouseMiddleDown(wxMouseEvent& event)
{
    SetFocus();

    if( m_MouseCaptured_ButtonsHeld == 0 )
        CaptureMouse();
    m_MouseCaptured_ButtonsHeld++;
    
    //LOGInfo( LOGTag, "MainGLCanvas::MouseMiddleDown Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseButtonStates |= (1 << 2);
    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Down, 2, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::MouseMiddleDoubleClick(wxMouseEvent& event)
{
    MouseMiddleDown( event );
}

void MainGLCanvas::MouseMiddleUp(wxMouseEvent& event)
{
    m_MouseCaptured_ButtonsHeld--;
    if( m_MouseCaptured_ButtonsHeld == 0 )
        ReleaseMouse();

    //LOGInfo( LOGTag, "MainGLCanvas::MouseMiddleUp Event, %d\n", m_MouseCaptured_ButtonsHeld );

    g_GLCanvasIDActive = m_GLCanvasID;

    m_MouseButtonStates &= ~(1 << 2);
    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Up, 2, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::MouseWheelMoved(wxMouseEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    m_MousePosition.Set( (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation );
    m_InputEventQueue.push_back( EditorInputEvent( -1, -1, -1, GCBA_Wheel, m_MouseButtonStates, (float)event.m_x, (float)event.m_y, (float)event.m_wheelRotation ) );
}

void MainGLCanvas::KeyPressed(wxKeyEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    int keycode = (int)event.m_keyCode;

    if( m_KeysDown[keycode] == true )
        return;

    m_InputEventQueue.push_back( EditorInputEvent( -1, GCBA_Down, keycode, -1, -1, -1, -1, -1 ) );

    ////LOGInfo( LOGTag, "KeyPressed: %d\n", keycode );

    //if( g_pGameCore )
    //{
    //    //if( keycode >= 'A' && keycode <= 'Z' && m_KeysDown[MYKEYCODE_LSHIFT] == 0 && m_KeysDown[MYKEYCODE_RSHIFT] == 0 )
    //    //    g_pGameCore->OnKeyDown( keycode+32, keycode+32 );
    //    //else
    //        g_pGameCore->OnKeyDown( keycode, keycode );
    //}

    if( keycode < 512 )
        m_KeysDown[keycode] = true;

    event.Skip();
}

void MainGLCanvas::KeyReleased(wxKeyEvent& event)
{
    g_GLCanvasIDActive = m_GLCanvasID;

    int keycode = (int)event.m_keyCode;
    
    //LOGInfo( LOGTag, "KeyReleased: %d\n", keycode );

    m_InputEventQueue.push_back( EditorInputEvent( -1, GCBA_Up, keycode, -1, -1, -1, -1, -1 ) );

    //if( g_pGameCore )
    //{
    //    //if( keycode >= 'A' && keycode <= 'Z' && m_KeysDown[MYKEYCODE_LSHIFT] == 0 && m_KeysDown[MYKEYCODE_RSHIFT] == 0 )
    //    //    g_pGameCore->OnKeyUp( keycode+32, keycode+32 );
    //    //else
    //        g_pGameCore->OnKeyUp( keycode, keycode );
    //}

    if( keycode < 512 )
        m_KeysDown[keycode] = false;

    event.Skip();
}

void MainGLCanvas::KeyChar(wxKeyEvent& event)
{
    int c = event.GetKeyCode();
    int mods = event.GetModifiers();

    if( (mods == wxMOD_CONTROL) && (c <= 26) ) // WXK_CONTROL_A to WXK_CONTROL_Z
    {
        // ignore
    }
    else
    {
        if( c < 255 )
            m_InputEventQueue.push_back( EditorInputEvent( c, -1, -1, -1, -1, -1, -1, -1 ) );
    }
    
    event.Skip();
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

void MainGLCanvas::ProcessInputEventQueue()
{
    g_GLCanvasIDActive = m_GLCanvasID;

    for( unsigned int i=0; i<m_InputEventQueue.size(); i++ )
    {
        EditorInputEvent* ev = &m_InputEventQueue[i];

        if( ev->keychar != -1 )
        {
            bool used = g_pGameCore->OnChar( ev->keychar );

            if( used == false )
            {
                if( ev->keychar == MYKEYCODE_ESC )
                {
                    LockMouse( false );
                }
            }
        }
        else if( ev->keyaction == -1 )
        {
            // Mouse event
            MyAssert( ev->keycode == -1 );

            // normalize mouse wheel
            if( ev->pressure != 0 )
                ev->pressure = ev->pressure / fabs( ev->pressure );

            if( g_pGameCore )
                g_pGameCore->OnTouch( ev->mouseaction, ev->mousebuttonid, (float)ev->x, (float)ev->y, ev->pressure, 0 );

            //if( ev->mouseaction == GCBA_Held )
            //    LOGInfo( "Input", "Input event mouse: %d, %d, %d, %0.0f, %0.0f\n", m_GLCanvasID, ev->mouseaction, ev->mousebuttonid, (float)ev->x, (float)ev->y );
        }
        else
        {
            // Keyboard event
            MyAssert( ev->mouseaction == -1 );
            MyAssert( ev->mousebuttonid == -1 );
            MyAssert( ev->x == -1 );
            MyAssert( ev->y == -1 );
            MyAssert( ev->pressure == -1 );

            if( g_pGameCore )
            {
                if( ev->keyaction == GCBA_Down )
                    g_pGameCore->OnKeyDown( ev->keycode, ev->keycode );
                else if( ev->keyaction == GCBA_Up )
                    g_pGameCore->OnKeyUp( ev->keycode, ev->keycode );
                else
                    g_pGameCore->OnKeys( (GameCoreButtonActions)ev->keyaction, ev->keycode, ev->keycode );
            }
        }
    }

    // if any mouse button is held, send out a "held" event.
    if( m_MouseButtonStates != 0 )
    {
        if( g_pGameCore )
        {
            // normalize mouse wheel
            if( m_MousePosition.z != 0 )
                m_MousePosition.z = m_MousePosition.z / fabs( m_MousePosition.z );

            if( m_SystemMouseIsLocked )
            {
                // if( m_MouseButtonStates & 1 << 0 )
                //     g_pGameCore->OnTouch( GCBA_RelativeMovement, 0, 0, 0, 0, 0 );
                // if( m_MouseButtonStates & 1 << 1 )
                //     g_pGameCore->OnTouch( GCBA_RelativeMovement, 1, 0, 0, 0, 0 );
                // if( m_MouseButtonStates & 1 << 2 )
                //     g_pGameCore->OnTouch( GCBA_RelativeMovement, 2, 0, 0, 0, 0 );
            }
            else
            {
                if( m_MouseButtonStates & 1 << 0 )
                    g_pGameCore->OnTouch( GCBA_Held, 0, m_MousePosition.x, m_MousePosition.y, m_MousePosition.z, 0 );
                if( m_MouseButtonStates & 1 << 1 )
                    g_pGameCore->OnTouch( GCBA_Held, 1, m_MousePosition.x, m_MousePosition.y, m_MousePosition.z, 0 );
                if( m_MouseButtonStates & 1 << 2 )
                    g_pGameCore->OnTouch( GCBA_Held, 2, m_MousePosition.x, m_MousePosition.y, m_MousePosition.z, 0 );
            }
        }
    }

    // send key held messages for any key held
    for( int i=0; i<512; i++ )
    {
        if( m_KeysDown[i] )
            g_pGameCore->OnKeys( GCBA_Held, i, i );
    }

    m_InputEventQueue.clear();
}

void MainGLCanvas::Draw()
{
    g_GLCanvasIDActive = m_GLCanvasID;

    g_pGLContext->SetCurrent( *this );
    //SetCurrent( *g_pGLContext );
    //wxPaintDC( this );

#if TESTING_FRAGCOORDISSUE
    void FragCoordTest(MainGLCanvas* pGLCanvas);
    
    FragCoordTest( this );
    SwapBuffers();

    return;
#endif

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

                for( int i=0; i<512; i++ )
                    m_KeysDown[i] = false;

                m_MouseCaptured_ButtonsHeld = 0;
                m_MouseButtonStates = 0;
            }

            g_pMainApp->m_HasFocus = false;
        }

        g_pGameCore->OnDrawFrameStart( g_GLCanvasIDActive );

        // deal with queued up input messages
        if( m_TickGameCore )
        {
            //LOGInfo( "ImGui", "ProcessAllGLCanvasInputEventQueues()\n" );
            // process events for all MainGLCanvas objects before we tick the game.
            g_pMainApp->m_pMainFrame->ProcessAllGLCanvasInputEventQueues();
            g_GLCanvasIDActive = m_GLCanvasID;

            double currentTime = MyTime_GetRunningTime();
            float deltaTime = (float)(currentTime - m_LastTimeTicked);
            m_LastTimeTicked = currentTime;

            g_UnpausedTime += g_pGameCore->Tick( deltaTime );

            // Hacks on top of hacks, popping up a wx popup menu inside g_pGameCore->Tick is blocking
            //   other event messages will still change the value of g_GLCanvasIDActive which will trip assert before OnDrawFrame below
            if( g_GLCanvasIDActive != m_GLCanvasID )
                return;
        }
        else
        {
            //g_pGameCore->GenerateKeyHeldMessages();
        }

        int currentframebuffer;
        glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentframebuffer );
        MyAssert( currentframebuffer == 0 );

        //if( m_GLCanvasID == 0 )
        //if( g_pMainApp->m_HasFocus )
        {
            MyAssert( g_GLCanvasIDActive == m_GLCanvasID );

            g_pGameCore->OnDrawFrame( m_GLCanvasID );
            g_pGameCore->OnDrawFrameDone();
        }
    }

    SwapBuffers();
}

void MainGLCanvas::LockMouse(bool lock)
{
    if( lock )
    {
        if( m_SystemMouseIsLocked == false )
        {
            m_MouseXPositionWhenLocked = m_CurrentGLViewWidth/2;
            m_MouseYPositionWhenLocked = m_CurrentGLViewHeight/2;
            WarpPointer( m_MouseXPositionWhenLocked, m_MouseYPositionWhenLocked );

            m_SystemMouseIsLocked = true;

#if MYFW_WINDOWS
            ShowCursor( false );
#else
            SetCursor( wxCURSOR_BLANK );
#endif
        }
    }
    else
    {
        if( m_SystemMouseIsLocked == true )
        {
            //LOGInfo( LOGTag, "LockMouse( false );\n" );
            m_SystemMouseIsLocked = false;

#if MYFW_WINDOWS
            ShowCursor( true );
#else
            SetCursor( *wxSTANDARD_CURSOR );
            //SetCursor( wxCURSOR_DEFAULT );
#endif
        }
    }
}

void MainGLCanvas::RequestMouseLock()
{
    m_GameWantsLockedMouse = true;
}

void MainGLCanvas::ReleaseMouseLock()
{
    m_GameWantsLockedMouse = false;

    g_pMainApp->m_pMainFrame->m_pGLCanvas->LockMouse( false );
}

bool MainGLCanvas::IsMouseLocked()
{
    return m_SystemMouseIsLocked;
}

#if TESTING_FRAGCOORDISSUE

void FragCoordTest(MainGLCanvas* pGLCanvas)
{
    static bool loadedassets = false;
    static unsigned int proghandle = 0;

    if( loadedassets == false )
    {
        loadedassets = true;

        FILE* pFile;
        fopen_s( &pFile, "Data/Shaders/TestFragCoord.glsl", "rb" );

        char buffer[10000];
        int size = fread( buffer, 1, 10000, pFile );
        buffer[size] = 0;
        unsigned int vert;
        unsigned int frag;

        proghandle = createProgram( size, buffer, size, buffer, &vert, &frag );

        fclose( pFile );
    }

    MyBindBuffer( GL_ARRAY_BUFFER, 0 );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    MyAssert( proghandle != 0 );
    if( proghandle != 0 )
    {
        float verts[] =
        {
            -0.95f, -0.95f, // bl
             0.95f,  0.95f, // tr
            -0.95f,  0.95f, // tl
             0.95f,  0.95f, // tr
            -0.95f, -0.95f, // bl
             0.95f, -0.95f, // br
        };

        float time = (float)MyTime_GetRunningTime();
        for( int i=0; i<12; i++ )
        {
            verts[i] += sin( time ) * 0.02f;
        }

        if( pGLCanvas->m_GLCanvasID == 0 )
            glClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
        else
            glClearColor( 0.0f, 0.5f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        g_pRenderer->SetDepthTestEnabled( false );

        glViewport( 0, 0, pGLCanvas->m_CurrentGLViewWidth, pGLCanvas->m_CurrentGLViewHeight );

        MyUseProgram( proghandle );

        int apos = glGetAttribLocation( proghandle, "a_Position" );

        glUniform2f( 0, (float)pGLCanvas->m_CurrentGLViewWidth, (float)pGLCanvas->m_CurrentGLViewHeight );

        glVertexAttribPointer( apos, 2, GL_FLOAT, 0, 8, verts );
        glEnableVertexAttribArray( apos );

        glDrawArrays( GL_TRIANGLES, 0, 6 );

        //glPointSize( 10 );
        //glDrawArrays( GL_POINTS, 0, 6 );
    }
}

#endif //TESTING_FRAGCOORDISSUE
