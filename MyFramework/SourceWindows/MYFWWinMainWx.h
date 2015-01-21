//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __WinMainWx_H__
#define __WinMainWx_H__

#include "wx/wx.h"
#include "wx/glcanvas.h"

class MainFrame;
class MainGLCanvas;

enum GLViewTypes
{
    GLView_Full,
    GLView_Tall,
    GLView_Square,
    GLView_Wide,
};

enum MenuIDs
{
    myID_SavePerspective = wxID_HIGHEST + 1,
    myID_LoadPerspective,
    myID_ResetPerspective,
    myID_GLViewType_Fill,
    myID_GLViewType_Tall,
    myID_GLViewType_Square,
    myID_GLViewType_Wide,
    myID_NumIDs,
};

bool MYFW_GetKey(int value);
void WinMain_CreateGameCore();
MainFrame* WinMain_CreateMainFrame();
void WinMain_GetClientSize(int* width, int* height, GLViewTypes* viewtype);

extern unsigned int g_GLCanvasIDActive;

// Main application class
class MainApp : public wxApp
{
public:
    MainFrame* m_pMainFrame;

public:
    virtual bool OnInit();
    virtual int FilterEvent(wxEvent& event);
};

// Main frame class
class MainFrame : public wxFrame
{
public:
    wxMenuBar* m_MenuBar;
    wxMenu* m_File;
    wxMenu* m_View;
    wxMenu* m_Aspect;

    wxAuiManager m_AUIManager;
    MainGLCanvas* m_pGLCanvas;

    wxString m_DefaultPerspectiveString;
    wxString m_SavedPerspectiveString;

public:
    MainFrame(wxWindow* parent);
    ~MainFrame();

    virtual void AddPanes();
    void UpdateAUIManagerAndLoadPerspective();
    virtual void OnPostInit() {}
    virtual void OnClose() {}
    
    void OnQuit(wxCommandEvent& event);
    void OnMenu(wxCommandEvent& event);
    void OnKeyPressed(wxKeyEvent& event);
    void OnKeyReleased(wxKeyEvent& event);

    virtual void ResizeViewport();
};

extern int m_GLContextRefCount;
extern wxGLContext* m_GLContext;

// Main gl canvas class
class MainGLCanvas : public wxGLCanvas
{
public:
    unsigned int m_GLCanvasID;

    int m_MouseCaptured_ButtonsHeld;
    bool m_MouseDown;
    double m_LastTimeTicked;

    bool m_TickGameCore;
 
    int m_CurrentGLViewWidth;
    int m_CurrentGLViewHeight;

public:
    MainGLCanvas(wxWindow* parent, int* args, unsigned int ID, bool tickgamecore);
    virtual ~MainGLCanvas();

    void MakeContextCurrent();
 
    void Resized(wxSizeEvent& evt);
    void ResizeViewport(bool clearhack = true);
 
    void Idle(wxIdleEvent& evt);
    void Render(wxPaintEvent& evt);
    void Draw();

    // events
    DECLARE_EVENT_TABLE()
    void MouseMoved(wxMouseEvent& event);
    void MouseWheelMoved(wxMouseEvent& event);
    void MouseLeftDown(wxMouseEvent& event);
    void MouseLeftUp(wxMouseEvent& event);
    void MouseRightDown(wxMouseEvent& event);
    void MouseRightUp(wxMouseEvent& event);
    void MouseMiddleDown(wxMouseEvent& event);
    void MouseMiddleUp(wxMouseEvent& event);
    void MouseLeftWindow(wxMouseEvent& event);
    void KeyPressed(wxKeyEvent& event);
    void KeyReleased(wxKeyEvent& event);
 };

#endif //__WinMainWx_H__
