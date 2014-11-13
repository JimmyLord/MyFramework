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

#ifndef __PanelWatch_H__
#define __PanelWatch_H__

#define MAX_PanelWatch_VARIABLES        40
#define WXSlider_Float_Multiplier       10000.0f

enum PanelWatch_Types
{
    PanelWatchType_Int,
    PanelWatchType_Char,
    PanelWatchType_UnsignedChar,
    PanelWatchType_Bool,
    PanelWatchType_Float,
    PanelWatchType_Double,
    PanelWatchType_PointerWithDesc,
    PanelWatchType_Unknown,
};

class PanelWatch;

extern PanelWatch* g_pPanelWatch;

typedef void (*PanelWatchCallback)(void*);

class PanelWatchDropTarget : public wxDropTarget
{
public:
    void* m_pCallbackObj;
    PanelWatchCallback m_pCallbackFunc;

public:
    PanelWatchDropTarget();

    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);
};

class PanelWatch : public wxPanel
{
public:
    wxTimer* m_pTimer;
    wxStaticText** m_Handles_StaticText;
    wxTextCtrl** m_Handles_TextCtrl;
    wxSlider** m_Handles_Slider;

    bool m_AllowWindowToBeUpdated;

    int m_NumVariables;
    int m_SliderBeingDragged;

    void** m_pVariablePointers;
    Vector2* m_pVariableRanges;
    char** m_pVariableDescriptions;
    PanelWatch_Types* m_pVariableTypes;
    void** m_pVariableCallbackObjs;
    PanelWatchCallback* m_pVariableCallbackFuncs;

protected:
    void AddControlsForVariable(const char* name);

public:
    void UpdatePanel(int controltoupdate = -1);

    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnTextCtrlChanged(wxCommandEvent& event);
    void OnSliderChanged(wxScrollEvent& event);

public:
    PanelWatch(wxFrame* parentframe);
    ~PanelWatch();

    void ClearAllVariables();

    void AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, float min, float max);
    void AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, char* pDescription, void* pCallbackObj, PanelWatchCallback pCallBackFunc);

    void AddInt(const char* name, int* pInt, float min, float max);
    void AddChar(const char* name, char* pChar, float min, float max);
    void AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max);
    void AddBool(const char* name, bool* pBool, float min, float max);
    void AddFloat(const char* name, float* pFloat, float min, float max);
    void AddDouble(const char* name, double* pDouble, float min, float max);
    void AddPointerWithDescription(const char* name, void* pPointer, char* pDescription, void* pCallbackObj = 0, PanelWatchCallback pCallBackFunc = 0);
    void AddSpace();
};

#endif // __PanelWatch_H__
