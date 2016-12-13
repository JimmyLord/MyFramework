//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
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
#include "wx/combo.h"

#define MAX_PanelWatch_VARIABLES        400
#define WXSlider_Float_Multiplier       10000.0f

enum PanelWatchControlTypes
{
    PanelWatchControlType_StaticText,
    PanelWatchControlType_TextCtrl,
    PanelWatchControlType_Slider,
    PanelWatchControlType_Button,
    PanelWatchControlType_ColourPicker,
    PanelWatchControlType_ChoiceBox,
    PanelWatchControlType_ComboCtrl,
    PanelWatchControlType_CheckBox,
    PanelWatchControlType_NumTypes,
};

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
    int comboctrlwidth;
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
    PanelWatchType_ColorByte,
    PanelWatchType_PointerWithDesc,
    PanelWatchType_Enum,
    PanelWatchType_Flags,
    PanelWatchType_SpaceWithLabel,
    PanelWatchType_Button,
    PanelWatchType_String,

    //ADDING_NEW_WatchVariableType

    PanelWatchType_Unknown,
    PanelWatchType_NumTypes,
};

extern PanelWatchControlInfo g_PanelWatchControlInfo[PanelWatchType_NumTypes];

class PanelWatch;

extern PanelWatch* g_pPanelWatch;

typedef void (*PanelWatchCallback)(void*);
typedef void (*PanelWatchCallbackDropTarget)(void* pObjectPtr, int controlid, wxCoord x, wxCoord y);
typedef void (*PanelWatchCallbackValueChanged)(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue);
typedef void (*PanelWatchCallbackRightClick)(void* pObjectPtr, int controlid);
typedef void (*PanelWatchCallbackButtonPressed)(void* pObjectPtr, int buttonid);

class PanelWatchDropTarget : public wxDropTarget
{
public:
    void* m_pCallbackObj;
    PanelWatchCallbackDropTarget m_pCallbackFunc;
    int m_ControlIndex;

public:
    PanelWatchDropTarget();

    //virtual wxDragResult OnDragEnter(wxCoord x, wxCoord y, wxDragResult defResult);
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);
};

struct VariableProperties
{
    wxStaticText* m_Handle_StaticText;
    wxStaticText* m_Handle_StaticTextExtraLabel;
    wxTextCtrl* m_Handle_TextCtrl;
    wxSlider* m_Handle_Slider;
    wxButton* m_Handle_Button;
    wxColourPickerCtrl* m_Handle_ColourPicker;
    wxChoice* m_Handle_ChoiceBox;
    wxComboCtrl* m_Handle_ComboCtrl;
    wxCheckBox* m_Handle_CheckBox;

    void* m_Pointer;
    Vector2 m_Range;
    const char* m_Description;
    int m_NumEnumTypes;
    int m_ButtonID;
    wxString* m_pEnumStrings;
    PanelWatch_Types m_Type;
    void* m_pCallbackObj;
    PanelWatchCallbackDropTarget m_pOnDropCallbackFunc;
    PanelWatchCallbackButtonPressed m_pOnButtonPressedCallbackFunc;
    PanelWatchCallbackValueChanged m_pOnValueChangedCallbackFunc;
    PanelWatchCallbackRightClick m_pRightClickCallbackFunc;

    Vector4Int m_Rect_XYWH;

    // using with right mouse now, used by text controls to change numbers.
    double m_ValueOnLeftMouseDown;
    bool m_ValueChangedSinceMouseWasPressed;
    wxPoint m_StartMousePosition;
    wxPoint m_LastMousePosition;
    bool m_CapturedMouse;

    void Reset();
};

class PanelWatch : public wxScrolledWindow
{
protected:
    void* m_pObjectBeingWatched;

    wxTimer* m_pTimer;

    bool m_NeedsRefresh; // needed to avoid corruption if deleting controls while in that control's callback.
    void* m_RefreshCallbackObject;
    PanelWatchCallback m_RefreshCallbackFunc;

    bool m_AllowWindowToBeUpdated;

    int m_NumVariables;
    int m_SliderBeingDragged;

    VariableProperties* m_pVariables;
    CommandStack* m_pCommandStack;

    std::vector<wxControl*> m_Controls[PanelWatchControlType_NumTypes];

protected:
    wxControl* GetControlOfType(PanelWatchControlTypes type);
    int SetupStaticTextControl(wxStaticText* pStaticText, const char* name, int variablenum, float PosX, float PosY, float LabelWidth, float LabelHeight, wxString variablename, PanelWatchControlInfo* pInfo);
    void AddControlsForVariable(const char* name, int variablenum, int component, const char* componentname, Vector2Int offset = Vector2Int(0,0), Vector2Int size = Vector2Int(-1,-1));

public:
    void UpdatePanel(int controltoupdate = -1);

    void OnButtonPressed(wxCommandEvent& event);
    void OnChoiceBoxChanged(wxCommandEvent& event);
    void OnComboCtrlChanged(wxCommandEvent& event);
    void OnCheckBoxChanged(wxCommandEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnEditBoxKillFocus(wxFocusEvent& event);

    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseLeftControl(wxMouseEvent& event);

    void OnClickStaticText(wxMouseEvent& event);
    void OnRightClickVariable(wxMouseEvent& event);

    void Tick(double TimePassed);
    void OnTimer(wxTimerEvent& event);
    void OnTextCtrlEnter(wxCommandEvent& event);
    void OnTextCtrlChanged(int controlid);
    void OnSliderChanged(wxScrollEvent& event);
    void OnSliderChanged(int controlid, int value, bool addundocommand);//wxScrollEvent& event);
    void OnColourPickerChanged(wxColourPickerEvent& event);

    void StopTimer() { m_pTimer->Stop(); }

public:
    PanelWatch(wxFrame* parentframe, CommandStack* pCommandStack);
    ~PanelWatch();

    void* GetObjectBeingWatched() { return m_pObjectBeingWatched; }
    void SetObjectBeingWatched(void* obj) { m_pObjectBeingWatched = obj; }

    VariableProperties* GetVariableProperties(unsigned int index) { return &m_pVariables[index]; }

    void SetNeedsRefresh();
    void SetRefreshCallback(void* pCallbackObj, PanelWatchCallback pCallbackFunc);
    void ClearAllVariables();

    bool GetTextCtrlValueAsDouble(int controlid, double* valuenew, double* valueold); // returns true if blank
    void SetControlValueFromDouble(int controlid, double valuenew, double valueold, bool finishedchanging);

    int AddVariableOfTypeRange(PanelWatch_Types type, const char* name, void* pVar, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols);
    int AddVariableOfTypeDesc(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallbackDropTarget pOnDropCallBackFunc, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackButtonPressed pOnButtonPressedCallbackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc);
    int AddVariableOfTypeEnum(PanelWatch_Types type, const char* name, void* pVar, int min, int max, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols);
    int AddVariableOfTypeFlags(PanelWatch_Types type, const char* name, void* pVar, int min, int max, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols);

    int AddInt(const char* name, int* pInt, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddUnsignedInt(const char* name, unsigned int* pInt, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddChar(const char* name, char* pChar, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddBool(const char* name, bool* pBool, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddFloat(const char* name, float* pFloat, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddDouble(const char* name, double* pDouble, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int Add2Floats(const char* name, const char* label1, const char* label2, float* pFloat1, float* pFloat2, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddVector2(const char* name, Vector2* pVector2, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddVector3(const char* name, Vector3* pVector3, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddVector2Int(const char* name, Vector2Int* pVector2Int, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddVector3Int(const char* name, Vector3Int* pVector3Int, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddVector4(const char* name, Vector4* pVector4, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddColorFloat(const char* name, ColorFloat* pColorFloat, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddColorByte(const char* name, ColorByte* pColorByte, float min, float max, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj = 0, PanelWatchCallbackDropTarget pOnDropCallBackFunc = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddEnum(const char* name, int* pInt, int numtypes, const char** ppStrings, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddFlags(const char* name, unsigned int* pUInt, int numtypes, const char** ppStrings, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddSpace(const char* name, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);
    int AddButton(const char* label, void* pCallbackObj, int buttonid, PanelWatchCallbackButtonPressed pOnButtonPressedCallBackFunc, Vector2Int offset = Vector2Int(0,0), Vector2Int size = Vector2Int(-1,-1));
    int AddString(const char* name, const char* pString, int maxlength, void* pCallbackObj = 0, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc = 0, PanelWatchCallbackRightClick pRightClickCallbackFunc = 0);

    void SetRightClickFunction(int controlid, PanelWatchCallbackRightClick pRightClickCallbackFunc);

    void ChangeDescriptionForPointerWithDescription(int controlid, const char* pDescription);
    void ChangeStaticTextFontStyle(int controlid, wxFontStyle style, wxFontWeight weight);
    void ChangeStaticTextBGColor(int controlid, wxColour colour);

    wxString GetFlagsAsString(int variablenum);
};

#endif // __PanelWatch_H__
