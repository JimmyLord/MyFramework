//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __PanelWatch_H__
#define __PanelWatch_H__

class CommandStack;

#include "wx/stattext.h"
#include "wx/slider.h"
#include "wx/choice.h"

#define MAX_PanelWatch_VARIABLES        400
#define WXSlider_Float_Multiplier       10000.0f

struct PanelWatchControlInfo
{
    int height;
    int labelfontheight;
    int labelwidth;
    int labelpaddingbottom;
    int labelstyle;
    int sliderwidth;
    int editboxwidth;
    int colorpickerwidth;
    int choiceboxwidth;
};

enum PanelWatch_Types
{
    PanelWatchType_Int,
    PanelWatchType_UnsignedInt,
    PanelWatchType_Char,
    PanelWatchType_UnsignedChar,
    PanelWatchType_Bool,
    PanelWatchType_Float,
    PanelWatchType_Double,
    //PanelWatchType_Vector3,
    PanelWatchType_ColorFloat,
    PanelWatchType_PointerWithDesc,
    PanelWatchType_Enum,
    PanelWatchType_SpaceWithLabel,
    PanelWatchType_Button,

    //ADDING_NEW_WatchVariableType

    PanelWatchType_Unknown,
    PanelWatchType_NumTypes,
};

extern PanelWatchControlInfo g_PanelWatchControlInfo[PanelWatchType_NumTypes];

class PanelWatch;

extern PanelWatch* g_pPanelWatch;

typedef void (*PanelWatchCallback)(void*);
typedef void (*PanelWatchCallbackWithID)(void* pObjectPtr, int controlid, bool finishedchanging);

class PanelWatchDropTarget : public wxDropTarget
{
public:
    void* m_pCallbackObj;
    PanelWatchCallback m_pCallbackFunc;
    int m_ControlIndex;

public:
    PanelWatchDropTarget();

    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);
};

struct VariableProperties
{
    wxStaticText* m_Handle_StaticText;
    wxTextCtrl* m_Handle_TextCtrl;
    wxSlider* m_Handle_Slider;
    wxButton* m_Handle_Button;
    wxColourPickerCtrl* m_Handle_ColourPicker;
    wxChoice* m_Handle_ChoiceBox;

    void* m_Pointer;
    Vector2 m_Range;
    const char* m_Description;
    int m_NumEnumTypes;
    wxString* m_pEnumStrings;
    PanelWatch_Types m_Type;
    void* m_pCallbackObj;
    PanelWatchCallback m_pOnDropCallbackFunc;
    PanelWatchCallback m_pOnButtonPressedCallbackFunc;
    PanelWatchCallbackWithID m_pOnValueChangedCallbackFunc;

    Vector4Int m_Rect_XYWH;

    // using with right mouse now, used by text controls to change numbers.
    double m_ValueOnLeftMouseDown;
    wxPoint m_StartMousePosition;
    wxPoint m_LastMousePosition;
    bool m_LeftMouseIsDown;
};

class PanelWatch : public wxScrolledWindow
{
public:
    wxTimer* m_pTimer;

    bool m_NeedsRefresh; // needed to avoid corruption if deleting controls while in that control's callback.
    void* m_RefreshCallbackObject;
    PanelWatchCallback m_RefreshCallbackFunc;

    bool m_AllowWindowToBeUpdated;

    int m_NumVariables;
    int m_SliderBeingDragged;

    VariableProperties* m_pVariables;
    CommandStack* m_pCommandStack;

protected:
    void AddControlsForVariable(const char* name, int variablenum, int component, const char* componentname);

public:
    void UpdatePanel(int controltoupdate = -1);

    void OnButtonPressed(wxCommandEvent& event);
    void OnChoiceBoxChanged(wxCommandEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnEditBoxKillFocus(wxFocusEvent& event);

    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    void OnClickStaticText(wxMouseEvent& event);

    void OnTimer(wxTimerEvent& event);
    void OnTextCtrlEnter(wxCommandEvent& event);
    void OnTextCtrlChanged(int controlid);
    void OnSliderChanged(wxScrollEvent& event);
    void OnSliderChanged(int controlid, int value, bool addundocommand);//wxScrollEvent& event);
    void OnColourPickerChanged(wxColourPickerEvent& event);

public:
    PanelWatch(wxFrame* parentframe, CommandStack* pCommandStack);
    ~PanelWatch();

    void SetRefreshCallback(void* pCallbackObj, PanelWatchCallback pCallbackFunc);
    void ClearAllVariables();

    bool GetTextCtrlValueAsDouble(int controlid, double* valuenew, double* valueold); // returns true if blank
    void SetControlValueFromDouble(int controlid, double valuenew, double valueold, bool finishedchanging);

    int AddVariableOfTypeRange(PanelWatch_Types type, const char* name, void* pVar, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc, bool addcontrols);
    int AddVariableOfTypeDesc(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallback pOnDropCallBackFunc, PanelWatchCallbackWithID pOnValueChangedCallBackFunc, PanelWatchCallback pOnButtonPressedCallbackFunc);
    int AddVariableOfTypeEnum(PanelWatch_Types type, const char* name, void* pVar, float min, float max, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc, bool addcontrols);

    int AddInt(const char* name, int* pInt, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddUnsignedInt(const char* name, unsigned int* pInt, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddChar(const char* name, char* pChar, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddBool(const char* name, bool* pBool, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddFloat(const char* name, float* pFloat, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddDouble(const char* name, double* pDouble, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddVector2(const char* name, Vector2* pVector2, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddVector3(const char* name, Vector3* pVector3, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddVector4(const char* name, Vector4* pVector4, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddColorFloat(const char* name, ColorFloat* pColorFloat, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj = 0, PanelWatchCallback pOnDropCallBackFunc = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddEnum(const char* name, int* pInt, float numtypes, const char** ppStrings, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddSpace(const char* name, void* pCallbackObj = 0, PanelWatchCallbackWithID pOnValueChangedCallBackFunc = 0);
    int AddButton(const char* label, void* pCallbackObj, PanelWatchCallback pOnButtonPressedCallBackFunc);
};

#endif // __PanelWatch_H__
