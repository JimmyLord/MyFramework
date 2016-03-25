//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "PanelWatch.h"

#include "../SourceWidgets/EditorCommands.h"
#include "../SourceWidgets/CommandStack.h"
#include "expr_eval.h"

PanelWatch* g_pPanelWatch = 0;

class wxCheckListComboPopup
: public wxCheckListBox
, public wxComboPopup
{
public:

    // Initialize member variables
    virtual void Init()
    {
    }

    // Create popup control
    virtual bool Create(wxWindow* parent)
    {
        return wxCheckListBox::Create( parent, wxID_ANY, wxPoint(0,0), wxDefaultSize );
    }

    // Return pointer to the created control
    virtual wxWindow *GetControl() { return this; }

    // Translate string into a list selection
    virtual void SetStringValue(const wxString& s)
    {
    }

    // Get list selection as a string
    virtual wxString GetStringValue() const
    {
        return wxEmptyString;
    }

    // Do mouse hot-tracking (which is typical in list popups)
    void OnMouseMove(wxMouseEvent& event)
    {
        // Move selection to cursor
        int value = HitTest( event.GetPosition() );
        if( value >= 0 )
            wxListBox::Select( value );

        event.Skip();
    }

    // On mouse left up, set the value and close the popup
    void OnMouseClick(wxMouseEvent& event)
    {
        int value = HitTest( event.GetPosition() );

        if( wxCheckListBox::IsChecked( value ) )
            wxCheckListBox::Check( value, false );
        else
            wxCheckListBox::Check( value, true );

        this->SendEvent( value );

        event.Skip();
    }

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxCheckListComboPopup, wxListBox)
    EVT_MOTION(wxCheckListComboPopup::OnMouseMove)
    EVT_LEFT_UP(wxCheckListComboPopup::OnMouseClick)
END_EVENT_TABLE()

PanelWatchControlInfo g_PanelWatchControlInfo[PanelWatchType_NumTypes] = // ADDING_NEW_WatchVariableType
{ // control    label                                          widths
  // height, font,wdt,pdg, style                   slider, editbox, colorpicker, choicebox, combobox,
  //          hgt     bot,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_Int,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_UnsignedInt,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_Char,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_UnsignedChar,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,       0,           0,         0,        0, }, //PanelWatchType_Bool,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_Float,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,      45,           0,         0,        0, }, //PanelWatchType_Double,
  //{    20,   8, 100,  0, wxALIGN_LEFT,              120,      45,           0,         0,        0, }, //PanelWatchType_Vector3,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,       0,          85,         0,        0, }, //PanelWatchType_ColorFloat,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,       0,          85,         0,        0, }, //PanelWatchType_ColorByte,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,     170,           0,         0,        0, }, //PanelWatchType_PointerWithDesc,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,       0,           0,        75,        0, }, //PanelWatchType_Enum,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,       0,           0,         0,      125, }, //PanelWatchType_Flags,
    {     8,   6, 300,  2, wxALIGN_CENTRE_HORIZONTAL,   0,       0,           0,         0,        0, }, //PanelWatchType_SpaceWithLabel,
    {    20,   8, 150,  0, wxALIGN_CENTRE_HORIZONTAL,   0,       0,           0,         0,        0, }, //PanelWatchType_Button,
    {    20,   8, 100,  0, wxALIGN_LEFT,                0,     170,           0,         0,        0, }, //PanelWatchType_String
    {    -1,  -1,  -1, -1, -1,                          0,       0,           0,         0,        0, }, //PanelWatchType_Unknown,
};

void VariableProperties::Reset()
{
    m_Handle_StaticText = 0;
    m_Handle_StaticTextExtraLabel = 0;
    m_Handle_TextCtrl = 0;
    m_Handle_Slider = 0;
    m_Handle_Button = 0;
    m_Handle_ColourPicker = 0;
    m_Handle_ChoiceBox = 0;
    m_Handle_ComboBox = 0;
    m_Handle_CheckBox = 0;

    m_Pointer = 0;
    m_Range.Set( 0, 0 );
    m_Description = 0;
    m_NumEnumTypes = 0;
    m_pEnumStrings = 0;
    m_Type = PanelWatchType_Unknown;
    m_pCallbackObj = 0;
    m_pOnDropCallbackFunc = 0;
    m_pOnButtonPressedCallbackFunc = 0;
    m_pOnValueChangedCallbackFunc = 0;
    m_ValueOnLeftMouseDown = 0;
    m_ValueChangedSinceMouseWasPressed = false;
    m_CapturedMouse = 0;
    m_StartMousePosition = wxPoint(0,0);
    m_LastMousePosition = wxPoint(0,0);

    m_pOnDropCallbackFunc = 0;
    m_pOnButtonPressedCallbackFunc = 0;
    m_pOnValueChangedCallbackFunc = 0;
    m_pRightClickCallbackFunc = 0;
}

PanelWatch::PanelWatch(wxFrame* parentframe, CommandStack* pCommandStack)
: wxScrolledWindow( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Watch" )
{
    m_NeedsRefresh = false;
    m_RefreshCallbackObject = 0;
    m_RefreshCallbackFunc = 0;

    m_AllowWindowToBeUpdated = true;

    m_pCommandStack = pCommandStack;

    m_NumVariables = 0;
    m_SliderBeingDragged = -1;

    m_pVariables = MyNew VariableProperties[MAX_PanelWatch_VARIABLES];

    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        m_pVariables[i].Reset();
    }

    m_pTimer = MyNew wxTimer( this, wxID_ANY );
    m_pTimer->Start( 100 );

    //Connect( wxEVT_CHILD_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus) );
    //Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus) );
    //Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    //Connect( wxEVT_SLIDER, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(PanelWatch::OnTextCtrlEnter) );
    Connect( wxEVT_COLOURPICKER_CHANGED, wxColourPickerEventHandler(PanelWatch::OnColourPickerChanged) );    
    Connect( wxEVT_TIMER, wxTimerEventHandler(PanelWatch::OnTimer) );
}

PanelWatch::~PanelWatch()
{
    ClearAllVariables();

    for( int type=0; type<PanelWatchControlType_NumTypes; type++ )
    {
        for( unsigned int i=0; i<m_Controls[type].size(); i++ )
        {
            this->RemoveChild( m_Controls[type][i] );
            SAFE_DELETE( m_Controls[type][i] );
        }
    }

    SAFE_DELETE_ARRAY( m_pVariables );

    SAFE_DELETE( m_pTimer );
}

void PanelWatch::SetRefreshCallback(void* pCallbackObj, PanelWatchCallback pCallbackFunc)
{
    m_RefreshCallbackObject = pCallbackObj;
    m_RefreshCallbackFunc = pCallbackFunc;
}

void PanelWatch::ClearAllVariables()
{
    m_pObjectBeingWatched = 0;

    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        if( m_pVariables[i].m_Handle_StaticText != 0 )
        {
            m_pVariables[i].m_Handle_StaticText->Show( false );
            //m_pVariables[i].m_Handle_StaticText->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler(PanelWatch::OnClickStaticText) );
        }
        if( m_pVariables[i].m_Handle_StaticTextExtraLabel != 0 )
        {
            m_pVariables[i].m_Handle_StaticTextExtraLabel->Show( false );
            //m_pVariables[i].m_Handle_StaticTextExtraLabel->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler(PanelWatch::OnClickStaticText) );
        }
        if( m_pVariables[i].m_Handle_TextCtrl != 0 )
        {
            m_pVariables[i].m_Handle_TextCtrl->Show( false );
        }
        if( m_pVariables[i].m_Handle_Slider != 0 )
        {
            m_pVariables[i].m_Handle_Slider->Show( false );
        }
        if( m_pVariables[i].m_Handle_Button != 0 )
        {
            m_pVariables[i].m_Handle_Button->Show( false );
        }
        if( m_pVariables[i].m_Handle_ColourPicker != 0 )
        {
            m_pVariables[i].m_Handle_ColourPicker->Show( false );
        }
        if( m_pVariables[i].m_Handle_ChoiceBox != 0 )
        {
            m_pVariables[i].m_Handle_ChoiceBox->Show( false );
        }
        if( m_pVariables[i].m_Handle_ComboBox != 0 )
        {
            m_pVariables[i].m_Handle_ComboBox->Show( false );
        }
        if( m_pVariables[i].m_Handle_CheckBox != 0 )
        {
            m_pVariables[i].m_Handle_CheckBox->Show( false );
        }

        SAFE_DELETE_ARRAY( m_pVariables[i].m_pEnumStrings );

        m_pVariables[i].Reset();
    }

    m_NumVariables = 0;
}

int PanelWatch::AddVariableOfTypeRange(PanelWatch_Types type, const char* name, void* pVar, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( min, max );
    m_pVariables[m_NumVariables].m_Description = 0;
    m_pVariables[m_NumVariables].m_NumEnumTypes = 0;
    m_pVariables[m_NumVariables].m_pEnumStrings = 0;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnButtonPressedCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pRightClickCallbackFunc = pRightClickCallbackFunc;

    if( addcontrols )
    {
        AddControlsForVariable( name, m_NumVariables, -1, 0 );
        UpdatePanel();
    }

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddVariableOfTypeDesc(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallbackDropTarget pOnDropCallBackFunc, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallback pOnButtonPressedCallbackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( 0, 0 );
    m_pVariables[m_NumVariables].m_Description = pDescription;
    m_pVariables[m_NumVariables].m_NumEnumTypes = 0;
    m_pVariables[m_NumVariables].m_pEnumStrings = 0;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = pOnDropCallBackFunc;
    m_pVariables[m_NumVariables].m_pOnButtonPressedCallbackFunc = pOnButtonPressedCallbackFunc;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pRightClickCallbackFunc = pRightClickCallbackFunc;

    AddControlsForVariable( name, m_NumVariables, -1, 0 );

    m_NumVariables++;

    UpdatePanel();

    return m_NumVariables-1;
}

int PanelWatch::AddVariableOfTypeEnum(PanelWatch_Types type, const char* name, void* pVar, int min, int max, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    int numtypes = (int)max;

    wxString* wxstrings = MyNew wxString[numtypes];
    for( int i=0; i<numtypes; i++ )
    {
        wxstrings[i] = ppStrings[i];
    }

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( (float)min, (float)max ); // turning our ints to floats, not ideal.
    m_pVariables[m_NumVariables].m_Description = 0;
    m_pVariables[m_NumVariables].m_NumEnumTypes = numtypes;
    m_pVariables[m_NumVariables].m_pEnumStrings = wxstrings;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnButtonPressedCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pRightClickCallbackFunc = pRightClickCallbackFunc;

    if( addcontrols )
    {
        AddControlsForVariable( name, m_NumVariables, -1, 0 );
        UpdatePanel();
    }

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddVariableOfTypeFlags(PanelWatch_Types type, const char* name, void* pVar, int min, int max, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc, bool addcontrols)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    int numtypes = (int)max;

    wxString* wxstrings = MyNew wxString[numtypes];
    for( int i=0; i<numtypes; i++ )
    {
        wxstrings[i] = ppStrings[i];
    }

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( (float)min, (float)max ); // turning our ints to floats, not ideal.
    m_pVariables[m_NumVariables].m_Description = 0;
    m_pVariables[m_NumVariables].m_NumEnumTypes = numtypes;
    m_pVariables[m_NumVariables].m_pEnumStrings = wxstrings;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnButtonPressedCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pRightClickCallbackFunc = pRightClickCallbackFunc;

    if( addcontrols )
    {
        AddControlsForVariable( name, m_NumVariables, -1, 0 );
        UpdatePanel();
    }

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddInt(const char* name, int* pInt, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_Int, name, pInt, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddUnsignedInt(const char* name, unsigned int* pInt, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_UnsignedInt, name, pInt, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddChar(const char* name, char* pChar, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_Char, name, pChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_UnsignedChar, name, pUChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddBool(const char* name, bool* pBool, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_Bool, name, pBool, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddFloat(const char* name, float* pFloat, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_Float, name, pFloat, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddDouble(const char* name, double* pDouble, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeRange( PanelWatchType_Double, name, pDouble, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::Add2Floats(const char* name, const char* label1, const char* label2, float* pFloat1, float* pFloat2, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    int first;
    first = AddVariableOfTypeRange( PanelWatchType_Float, "x", pFloat1, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "y", pFloat2, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );

    AddControlsForVariable( name, first+0, 0, label1 );
    AddControlsForVariable( name, first+1, 1, label2 );

    return first;
}

int PanelWatch::AddVector2(const char* name, Vector2* pVector2, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    int first;
    first = AddVariableOfTypeRange( PanelWatchType_Float, "x", &pVector2->x, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "y", &pVector2->y, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );

    AddControlsForVariable( name, first+0, 0, "x" );
    AddControlsForVariable( name, first+1, 1, "y" );

    return first;
}

int PanelWatch::AddVector3(const char* name, Vector3* pVector3, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    int first;
    first = AddVariableOfTypeRange( PanelWatchType_Float, "x", &pVector3->x, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "y", &pVector3->y, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "z", &pVector3->z, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );

    AddControlsForVariable( name, first+0, 0, "x" );
    AddControlsForVariable( name, first+1, 1, "y" );
    AddControlsForVariable( name, first+2, 2, "z" );

    return first;
}

int PanelWatch::AddVector4(const char* name, Vector4* pVector4, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    int first;
    first = AddVariableOfTypeRange( PanelWatchType_Float, "x", &pVector4->x, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "y", &pVector4->y, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "z", &pVector4->z, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddVariableOfTypeRange( PanelWatchType_Float, "w", &pVector4->w, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );

    AddControlsForVariable( name, first+0, 0, "x" );
    AddControlsForVariable( name, first+1, 1, "y" );
    AddControlsForVariable( name, first+2, 2, "z" );
    AddControlsForVariable( name, first+3, 3, "w" );

    return first;
}

int PanelWatch::AddColorFloat(const char* name, ColorFloat* pColorFloat, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    // TODO: maybe? change this to a color picker that supports alpha.
    int first;
    first = AddVariableOfTypeDesc( PanelWatchType_ColorFloat, name, pColorFloat, 0, 0, 0, 0, 0, pRightClickCallbackFunc );

    // add alpha as a float
    AddVariableOfTypeRange( PanelWatchType_Float, "a", &pColorFloat->a, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddControlsForVariable( name, first+1, 1, "a" );

    return first;
}

int PanelWatch::AddColorByte(const char* name, ColorByte* pColorByte, float min, float max, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    // TODO: maybe? change this to a color picker that supports alpha.
    int first;
    first = AddVariableOfTypeDesc( PanelWatchType_ColorByte, name, pColorByte, 0, pCallbackObj, 0, pOnValueChangedCallBackFunc, 0, pRightClickCallbackFunc );

    // add alpha as a unsigned char
    AddVariableOfTypeRange( PanelWatchType_UnsignedChar, "a", &pColorByte->a, min, max, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, false );
    AddControlsForVariable( name, first+1, 1, "a" );

    return first;
}

int PanelWatch::AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj, PanelWatchCallbackDropTarget pOnDropCallBackFunc, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeDesc( PanelWatchType_PointerWithDesc, name, pPointer, pDescription, pCallbackObj, pOnDropCallBackFunc, pOnValueChangedCallBackFunc, 0, pRightClickCallbackFunc );
}

int PanelWatch::AddEnum(const char* name, int* pInt, int numtypes, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeEnum( PanelWatchType_Enum, name, pInt, 0, numtypes, ppStrings, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddFlags(const char* name, unsigned int* pUInt, int numtypes, const char** ppStrings, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    return AddVariableOfTypeFlags( PanelWatchType_Flags, name, pUInt, 0, numtypes, ppStrings, pCallbackObj, pOnValueChangedCallBackFunc, pRightClickCallbackFunc, true );
}

int PanelWatch::AddSpace(const char* name, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Type = PanelWatchType_SpaceWithLabel;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;

    AddControlsForVariable( name, m_NumVariables, -1, 0 );

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddButton(const char* label, void* pCallbackObj, PanelWatchCallback pOnButtonPressedCallBackFunc)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Type = PanelWatchType_Button;
    m_pVariables[m_NumVariables].m_pOnButtonPressedCallbackFunc = pOnButtonPressedCallBackFunc;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;

    AddControlsForVariable( label, m_NumVariables, -1, 0 );

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddString(const char* name, const char* pString, int maxlength, void* pCallbackObj, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    MyAssert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Type = PanelWatchType_String;
    m_pVariables[m_NumVariables].m_Pointer = (void*)pString;
    m_pVariables[m_NumVariables].m_Range.Set( 0, (float)maxlength );
    m_pVariables[m_NumVariables].m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;

    AddControlsForVariable( name, m_NumVariables, -1, 0 );

    m_NumVariables++;

    return m_NumVariables-1;
}

void PanelWatch::SetRightClickFunction(int controlid, PanelWatchCallbackRightClick pRightClickCallbackFunc)
{
    m_pVariables[controlid].m_pRightClickCallbackFunc = pRightClickCallbackFunc;
}

void PanelWatch::ChangeDescriptionForPointerWithDescription(int controlid, const char* pDescription)
{
    m_pVariables[controlid].m_Handle_TextCtrl->SetValue( pDescription );
    m_pVariables[controlid].m_Description = pDescription;
}

void PanelWatch::ChangeStaticTextFontStyle(int controlid, wxFontStyle style, wxFontWeight weight)
{
    //pStaticText->SetFont( wxFont(pInfo->labelfontheight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
    wxFont font = m_pVariables[controlid].m_Handle_StaticText->GetFont();
    font.SetStyle( style );
    font.SetWeight( weight );

    if( m_pVariables[controlid].m_Handle_StaticText )
        m_pVariables[controlid].m_Handle_StaticText->SetFont( font );

    if( m_pVariables[controlid].m_Handle_StaticTextExtraLabel )
        m_pVariables[controlid].m_Handle_StaticTextExtraLabel->SetFont( font );
}

void PanelWatch::ChangeStaticTextBGColor(int controlid, wxColour colour)
{
    if( m_pVariables[controlid].m_Handle_StaticText )
        m_pVariables[controlid].m_Handle_StaticText->SetBackgroundColour( colour );

    if( m_pVariables[controlid].m_Handle_StaticTextExtraLabel )
        m_pVariables[controlid].m_Handle_StaticTextExtraLabel->SetBackgroundColour( colour );
}

wxControl* PanelWatch::GetControlOfType(PanelWatchControlTypes type)
{
    // look for an existing control of this type that isn't visible and return it.
    for( unsigned int i=0; i<m_Controls[type].size(); i++ )
    {
        if( m_Controls[type][i]->IsShown() == false )
        {
            m_Controls[type][i]->Show( true );
            return m_Controls[type][i];
        }
    }

    wxControl* pControlHandle = 0;

    // if none found, create a new one
    switch( type )
    {
    case PanelWatchControlType_StaticText:
        {
            pControlHandle = MyNew wxStaticText( this, 0, wxEmptyString );
            
            pControlHandle->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler(PanelWatch::OnClickStaticText), 0, this );
            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
            //pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            //pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );
        }
        break;

    case PanelWatchControlType_TextCtrl:
        {
            pControlHandle = MyNew wxTextCtrl( this, 0 );

            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnEditBoxKillFocus), 0, this );

            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnMouseDown), 0, this );
            pControlHandle->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler(PanelWatch::OnMouseUp), 0, this );
            pControlHandle->Connect( wxEVT_MOTION, wxMouseEventHandler(PanelWatch::OnMouseMove), 0, this );
            pControlHandle->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler(PanelWatch::OnMouseLeftControl), 0, this );
        }
        break;

    case PanelWatchControlType_Slider:
        {
            pControlHandle = MyNew wxSlider( this, 0, 0, 0, 0 );

            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

            //pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
            //pControlHandle->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler(PanelWatch::OnMouseDown), 0, this );
            //pControlHandle->Connect( wxEVT_LEFT_UP, wxMouseEventHandler(PanelWatch::OnMouseUp), 0, this );
            //pControlHandle->Connect( wxEVT_MOTION, wxMouseEventHandler(PanelWatch::OnMouseMove), 0, this );
        }
        break;

    case PanelWatchControlType_Button:
        {
            pControlHandle = MyNew wxButton( this, 0 );

            // setup callback for when button is pressed.
            pControlHandle->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PanelWatch::OnButtonPressed), 0, this );
        }
        break;

    case PanelWatchControlType_ColourPicker:
        {
            pControlHandle = MyNew wxColourPickerCtrl( this, 0 );

            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
        }
        break;

    case PanelWatchControlType_ChoiceBox:
        {
            pControlHandle = MyNew wxChoice( this, 0 );

            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_CHOICE, wxCommandEventHandler(PanelWatch::OnChoiceBoxChanged), 0, this );
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
        }
        break;

    case PanelWatchControlType_ComboBox:
        {
            pControlHandle = MyNew wxComboCtrl( this, wxID_ANY, wxEmptyString );

            wxCheckListComboPopup* pCheckListComboPopup = new wxCheckListComboPopup();
            ((wxComboCtrl*)pControlHandle)->SetPopupControl( pCheckListComboPopup );
    
            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_CHECKLISTBOX, wxCommandEventHandler(PanelWatch::OnComboBoxChanged), 0, this );
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
        }
        break;

    case PanelWatchControlType_CheckBox:
        {
            pControlHandle = MyNew wxCheckBox( this, 0, wxEmptyString );

            // if control gets focus, stop updates.
            pControlHandle->Connect( wxEVT_CHECKBOX, wxCommandEventHandler(PanelWatch::OnCheckBoxChanged), 0, this );
            pControlHandle->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
            pControlHandle->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

            pControlHandle->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(PanelWatch::OnRightClickVariable), 0, this );
        }
        break;

    case PanelWatchControlType_NumTypes:
        break;
    }

    MyAssert( pControlHandle );
    pControlHandle->SetParent( this );
    m_Controls[type].push_back( pControlHandle );

    return m_Controls[type].back();
}

int PanelWatch::SetupStaticTextControl(wxStaticText* pStaticText, const char* name, int variablenum, float PosX, float PosY, float LabelWidth, float LabelHeight, wxString variablename, PanelWatchControlInfo* pInfo)
{
    pStaticText->SetId( variablenum );
    pStaticText->SetPosition( wxPoint((int)PosX, (int)PosY) );
    pStaticText->SetInitialSize( wxSize((int)LabelWidth, (int)LabelHeight) );
    pStaticText->SetWindowStyle( pInfo->labelstyle );
    pStaticText->SetLabel( variablename );
    pStaticText->SetFont( wxFont(pInfo->labelfontheight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );

    if( m_pVariables[variablenum].m_Type == PanelWatchType_SpaceWithLabel )
    {
        pStaticText->SetBackgroundColour( wxColour(0,100,0,255) );
        pStaticText->SetForegroundColour( wxColour(255,255,255,255) );
    }
    else
    {
        pStaticText->SetBackgroundColour( wxNullColour );
        pStaticText->SetForegroundColour( wxNullColour );
    }

    m_pVariables[variablenum].m_Rect_XYWH.w += pInfo->labelpaddingbottom;
    return (int)LabelWidth;
}

void PanelWatch::AddControlsForVariable(const char* name, int variablenum, int component, const char* componentname)
{
    int PaddingTop = 3;
    int PaddingBottom = 3;
    int PaddingLeft = 2;

    int ControlPaddingTop = 3;
    int ControlPaddingBottom = 0;
    int ControlPaddingLeft = 0;

    PanelWatch_Types type = m_pVariables[variablenum].m_Type;
    PanelWatchControlInfo* pInfo = &g_PanelWatchControlInfo[type];

    int ControlHeight = ControlPaddingTop + pInfo->height + ControlPaddingBottom;

    int PosX = PaddingLeft + ControlPaddingLeft;
    int PosY; // = PaddingTop + variablenum*ControlHeight;
    if( variablenum == 0 ) // first variable in list
        PosY = PaddingTop + variablenum*ControlHeight;
    else
        PosY = m_pVariables[variablenum-1].m_Rect_XYWH.y + m_pVariables[variablenum-1].m_Rect_XYWH.w;

    if( component >= 1 )
    {
        PosX = m_pVariables[variablenum-1].m_Rect_XYWH.x + m_pVariables[variablenum-1].m_Rect_XYWH.z;
        PosY = m_pVariables[variablenum-1].m_Rect_XYWH.y;
    }

    //int WindowWidth = 300;

    int LabelHeight = g_PanelWatchControlInfo[type].height;
    int TextCtrlHeight = 20;
    int SliderHeight = 20;

    wxString variablename = name;
    
    int LabelWidth = pInfo->labelwidth;
    int SliderWidth = 120;
    //int TextCtrlWidth = 70;

    if( component >= 0 )
    {
        LabelWidth = 10;
        SliderWidth = 30;
        //TextCtrlWidth = 45;

        if( component == 0 )
        {
            // create a label for the variable before the component label.
            wxStaticText* pStaticText = (wxStaticText*)GetControlOfType( PanelWatchControlType_StaticText );
            m_pVariables[variablenum].m_Handle_StaticTextExtraLabel = pStaticText;
            MyAssert( dynamic_cast<wxStaticText*>( m_pVariables[variablenum].m_Handle_StaticTextExtraLabel ) != 0 );

            PosX += SetupStaticTextControl( pStaticText, name, variablenum, (float)PosX, (float)PosY, 90, (float)LabelHeight, variablename, pInfo );
        }

        variablename = componentname;
    }

    int TextCtrlWidth = g_PanelWatchControlInfo[type].editboxwidth;
    int ColorPickerWidth = g_PanelWatchControlInfo[type].colorpickerwidth;

    m_pVariables[variablenum].m_Rect_XYWH.x = PosX;
    m_pVariables[variablenum].m_Rect_XYWH.y = PosY;
    m_pVariables[variablenum].m_Rect_XYWH.z = LabelWidth + 10 + ColorPickerWidth + /*SliderWidth +*/ TextCtrlWidth;
    m_pVariables[variablenum].m_Rect_XYWH.w = ControlHeight;

    // Text label
    if( m_pVariables[variablenum].m_Type != PanelWatchType_Button )
    {
        wxStaticText* pStaticText = (wxStaticText*)GetControlOfType( PanelWatchControlType_StaticText );
        m_pVariables[variablenum].m_Handle_StaticText = pStaticText;
        MyAssert( dynamic_cast<wxStaticText*>( m_pVariables[variablenum].m_Handle_StaticText ) != 0 );

        PosX += SetupStaticTextControl( pStaticText, name, variablenum, (float)PosX, (float)PosY, (float)LabelWidth, (float)LabelHeight, variablename, pInfo );
    }

    // Slider
    if( pInfo->sliderwidth != 0 )
    {
        float sliderfloatmultiplier = 1;
        if( m_pVariables[variablenum].m_Type == PanelWatchType_Float ||
            m_pVariables[variablenum].m_Type == PanelWatchType_Double ) //|| 
            //m_pVariables[variablenum].m_Type == PanelWatchType_Vector3 )
        {
            sliderfloatmultiplier = WXSlider_Float_Multiplier;
        }

        //wxSlider* pSlider = MyNew wxSlider( this, variablenum, 0,
        //    m_pVariables[variablenum].m_Range.x * sliderfloatmultiplier, // min
        //    m_pVariables[variablenum].m_Range.y * sliderfloatmultiplier, // max
        //    wxPoint(PosX, PosY), wxSize(SliderWidth, SliderHeight) );
        //m_pVariables[variablenum].m_Handle_Slider = pSlider;

        m_pVariables[variablenum].m_Handle_Slider = (wxSlider*)GetControlOfType( PanelWatchControlType_Slider );
        MyAssert( dynamic_cast<wxSlider*>( m_pVariables[variablenum].m_Handle_Slider ) != 0 );

        m_pVariables[variablenum].m_Handle_Slider->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_Slider->SetPosition( wxPoint(PosX, PosY) );
        m_pVariables[variablenum].m_Handle_Slider->SetInitialSize( wxSize(SliderWidth, SliderHeight) );
        m_pVariables[variablenum].m_Handle_Slider->SetRange( (int)(m_pVariables[variablenum].m_Range.x * sliderfloatmultiplier),
                                                             (int)(m_pVariables[variablenum].m_Range.y * sliderfloatmultiplier) );

        PosX += SliderWidth;
    }

    // Edit box
    if( g_PanelWatchControlInfo[type].editboxwidth != 0 )
    {
        //wxTextCtrl* pTextCtrl = MyNew wxTextCtrl( this, variablenum, "",
        //    wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight), wxTE_PROCESS_ENTER );
        //m_pVariables[variablenum].m_Handle_TextCtrl = pTextCtrl;

        m_pVariables[variablenum].m_Handle_TextCtrl = (wxTextCtrl*)GetControlOfType( PanelWatchControlType_TextCtrl );
        MyAssert( dynamic_cast<wxTextCtrl*>( m_pVariables[variablenum].m_Handle_TextCtrl ) != 0 );

        m_pVariables[variablenum].m_Handle_TextCtrl->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_TextCtrl->SetPosition( wxPoint(PosX, PosY) );
        m_pVariables[variablenum].m_Handle_TextCtrl->SetInitialSize( wxSize(TextCtrlWidth, TextCtrlHeight) );
        m_pVariables[variablenum].m_Handle_TextCtrl->SetWindowStyle( wxTE_PROCESS_ENTER );
        m_pVariables[variablenum].m_Handle_TextCtrl->SetValue( variablename );

        PosX += TextCtrlWidth;

        if( m_pVariables[variablenum].m_pOnDropCallbackFunc )
        {
            PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
            pDropTarget->m_pCallbackObj = m_pVariables[variablenum].m_pCallbackObj;
            pDropTarget->m_pCallbackFunc = m_pVariables[variablenum].m_pOnDropCallbackFunc;
            pDropTarget->m_ControlIndex = variablenum;

            m_pVariables[variablenum].m_Handle_TextCtrl->SetDropTarget( pDropTarget );            
        }
    }

    if( type == PanelWatchType_Bool )
    {
        m_pVariables[variablenum].m_Handle_CheckBox = (wxCheckBox*)GetControlOfType( PanelWatchControlType_CheckBox );
        MyAssert( dynamic_cast<wxCheckBox*>( m_pVariables[variablenum].m_Handle_CheckBox ) != 0 );

        m_pVariables[variablenum].m_Handle_CheckBox->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_CheckBox->SetPosition( wxPoint(PosX, PosY) );
        //m_pVariables[variablenum].m_Handle_CheckBox->SetInitialSize( wxSize(TextCtrlWidth, TextCtrlHeight) );
        //m_pVariables[variablenum].m_Handle_CheckBox->SetWindowStyle( wxTE_PROCESS_ENTER );
        m_pVariables[variablenum].m_Handle_CheckBox->Set3StateValue( *(bool*)m_pVariables[variablenum].m_Pointer ? wxCHK_CHECKED : wxCHK_UNCHECKED );

        if( m_pVariables[variablenum].m_pOnDropCallbackFunc )
        {
            PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
            pDropTarget->m_pCallbackObj = m_pVariables[variablenum].m_pCallbackObj;
            pDropTarget->m_pCallbackFunc = m_pVariables[variablenum].m_pOnDropCallbackFunc;
            pDropTarget->m_ControlIndex = variablenum;

            m_pVariables[variablenum].m_Handle_TextCtrl->SetDropTarget( pDropTarget );            
        }
    }

    if( g_PanelWatchControlInfo[type].choiceboxwidth != 0 )
    {
        // add a drop list for enum types.
        //wxChoice* pChoiceBox = MyNew wxChoice( this, variablenum,
        //    wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight), m_pVariables[variablenum].m_NumEnumTypes,
        //    m_pVariables[variablenum].m_pEnumStrings, wxTE_PROCESS_ENTER );
        //m_pVariables[variablenum].m_Handle_ChoiceBox = pChoiceBox;

        m_pVariables[variablenum].m_Handle_ChoiceBox = (wxChoice*)GetControlOfType( PanelWatchControlType_ChoiceBox );
        MyAssert( dynamic_cast<wxChoice*>( m_pVariables[variablenum].m_Handle_ChoiceBox ) != 0 );

        m_pVariables[variablenum].m_Handle_ChoiceBox->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_ChoiceBox->SetPosition( wxPoint(PosX, PosY) );
        m_pVariables[variablenum].m_Handle_ChoiceBox->SetInitialSize( wxSize(TextCtrlWidth, TextCtrlHeight) );
        m_pVariables[variablenum].m_Handle_ChoiceBox->SetWindowStyle( wxTE_PROCESS_ENTER );
        m_pVariables[variablenum].m_Handle_ChoiceBox->Set( m_pVariables[variablenum].m_NumEnumTypes, m_pVariables[variablenum].m_pEnumStrings );

        m_pVariables[variablenum].m_Handle_ChoiceBox->SetSelection( *(int*)m_pVariables[variablenum].m_Pointer );

        PosX += g_PanelWatchControlInfo[type].choiceboxwidth;
    }

    if( g_PanelWatchControlInfo[type].comboboxwidth != 0 )
    {
        // add a checkbox drop list for flag types.
        m_pVariables[variablenum].m_Handle_ComboBox = (wxComboCtrl*)GetControlOfType( PanelWatchControlType_ComboBox );
        wxComboCtrl* pComboCtrl = m_pVariables[variablenum].m_Handle_ComboBox;
        MyAssert( dynamic_cast<wxComboCtrl*>( pComboCtrl ) != 0 );

        pComboCtrl->SetId( variablenum );
        pComboCtrl->SetPosition( wxPoint(PosX, PosY) );
        pComboCtrl->SetInitialSize( wxSize(g_PanelWatchControlInfo[type].comboboxwidth, TextCtrlHeight) );
        pComboCtrl->SetWindowStyle( wxTE_PROCESS_ENTER );

        wxCheckListComboPopup* pCheckListComboPopup = (wxCheckListComboPopup*)pComboCtrl->GetPopupControl();
        MyAssert( dynamic_cast<wxCheckListComboPopup*>( pCheckListComboPopup ) != 0 );

        pCheckListComboPopup->SetId( variablenum );
        pCheckListComboPopup->Set( m_pVariables[variablenum].m_NumEnumTypes, m_pVariables[variablenum].m_pEnumStrings );

        unsigned int flags = *(int*)m_pVariables[variablenum].m_Pointer;
        for( int i=0; i<32; i++ )
        {
            if( flags & (1<<i) )
                pCheckListComboPopup->Check( i, true );
            else
                pCheckListComboPopup->Check( i, false );
        }

        wxString string = GetFlagsAsString( variablenum );
        pComboCtrl->SetText( string );

        PosX += g_PanelWatchControlInfo[type].comboboxwidth;
    }

    // Button
    if( m_pVariables[variablenum].m_Type == PanelWatchType_Button )
    {
        //wxButton* pButton = MyNew wxButton( this, variablenum, name,
        //    wxPoint(PosX, PosY), wxSize(LabelWidth, LabelHeight) );
        //m_pVariables[variablenum].m_Handle_Button = pButton;

        m_pVariables[variablenum].m_Handle_Button = (wxButton*)GetControlOfType( PanelWatchControlType_Button );
        MyAssert( dynamic_cast<wxButton*>( m_pVariables[variablenum].m_Handle_Button ) != 0 );

        m_pVariables[variablenum].m_Handle_Button->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_Button->SetPosition( wxPoint(PosX, PosY) );
        m_pVariables[variablenum].m_Handle_Button->SetInitialSize( wxSize(LabelWidth, LabelHeight) );
        m_pVariables[variablenum].m_Handle_Button->SetLabel( variablename );

        PosX += TextCtrlWidth;
    }

    // Color picker.
    if( m_pVariables[variablenum].m_Type == PanelWatchType_ColorFloat ||
        m_pVariables[variablenum].m_Type == PanelWatchType_ColorByte )
    {
        // TODO: change this to a color picker that supports alpha.
        //wxColourPickerCtrl* pCtrl = MyNew wxColourPickerCtrl( this, variablenum, "",
        //    wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight) );
        //m_pVariables[variablenum].m_Handle_ColourPicker = pCtrl;

        m_pVariables[variablenum].m_Handle_ColourPicker = (wxColourPickerCtrl*)GetControlOfType( PanelWatchControlType_ColourPicker );
        MyAssert( dynamic_cast<wxColourPickerCtrl*>( m_pVariables[variablenum].m_Handle_ColourPicker ) != 0 );

        m_pVariables[variablenum].m_Handle_ColourPicker->SetId( variablenum );
        m_pVariables[variablenum].m_Handle_ColourPicker->SetPosition( wxPoint(PosX, PosY) );
        m_pVariables[variablenum].m_Handle_ColourPicker->SetInitialSize( wxSize(ColorPickerWidth, TextCtrlHeight) );

        PosX += TextCtrlWidth;

        //if( m_pVariables[variablenum].m_pOnDropCallbackFunc )
        //{
        //    PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
        //    pDropTarget->m_pCallbackObj = m_pVariables[variablenum].m_pCallbackObj;
        //    pDropTarget->m_pCallbackFunc = m_pVariables[variablenum].m_pOnDropCallbackFunc;
        //    pDropTarget->m_ControlIndex = variablenum;

        //    pCtrl->SetDropTarget( pDropTarget );            
        //}
    }

    int height = PaddingTop + (variablenum+1)*ControlHeight + PaddingBottom;

    SetScrollbars( 1, 1, PosX+10, height, 0, 0 );
}

PanelWatchDropTarget::PanelWatchDropTarget()
{
    wxCustomDataObject* dataobject = MyNew wxCustomDataObject;
    dataobject->SetFormat( *g_pMyDataFormat );
    SetDataObject( dataobject );
    m_ControlIndex = -1;
}

//wxDragResult PanelWatchDropTarget::OnDragEnter(wxCoord x, wxCoord y, wxDragResult defResult)
//{
//    return wxDragCopy;
//}

wxDragResult PanelWatchDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    return wxDragCopy;
}

wxDragResult PanelWatchDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult defResult)
{
    // figure out which object the stuff was dropped on and let it know.
    MyAssert( m_pCallbackObj && m_pCallbackFunc );

    g_DragAndDropStruct.m_ID = m_ControlIndex;
    m_pCallbackFunc( m_pCallbackObj, m_ControlIndex, x, y );

    return wxDragNone;
}

void PanelWatch::OnButtonPressed(wxCommandEvent& event)
{
    event.Skip();

    int controlid = event.GetId();
    if( m_pVariables[controlid].m_pOnButtonPressedCallbackFunc )
    {
        m_pVariables[controlid].m_pOnButtonPressedCallbackFunc( m_pVariables[controlid].m_pCallbackObj );
    }
}

void PanelWatch::OnChoiceBoxChanged(wxCommandEvent& event)
{
    //LOGInfo( LOGTag, "OnChoiceBoxChanged\n" );

    event.Skip();

    int controlid = event.GetId();

    int valueold = *(int*)m_pVariables[controlid].m_Pointer;
    int valuenew = event.GetSelection();

    // call the parent object to say it's value changed.
    if( valueold != valuenew )
    {
        // add the command to the undo stack and change the value at the same time.
        m_pCommandStack->Do( MyNew EditorCommand_PanelWatchNumberValueChanged(
            valuenew - valueold,
            m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
            m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
    }
}

void PanelWatch::OnComboBoxChanged(wxCommandEvent& event)
{
    //LOGInfo( LOGTag, "OnComboBoxChanged\n" );

    event.Skip();

    int controlid = event.GetId();

    int valueold = *(int*)m_pVariables[controlid].m_Pointer;
    int valuenew = event.GetSelection();

    // call the parent object to say it's value changed.
    {
        MyAssert( m_pVariables[controlid].m_Type == PanelWatchType_Flags );

        if( valueold & 1<<valuenew ) // if this bit was selected, unselect it.
        {
            valuenew = valueold & ~(1<<valuenew);
        }
        else // select it
        {
            valuenew = valueold | 1<<valuenew;
        }

        wxString string = GetFlagsAsString( controlid );
        m_pVariables[controlid].m_Handle_ComboBox->SetText( string );

        // add the command to the undo stack and change the value at the same time.
        m_pCommandStack->Do( MyNew EditorCommand_PanelWatchNumberValueChanged(
            valuenew - valueold,
            m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
            m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
    }
}

void PanelWatch::OnCheckBoxChanged(wxCommandEvent& event)
{
    //LOGInfo( LOGTag, "OnCheckBoxChanged\n" );

    event.Skip();

    int controlid = event.GetId();

    bool valueold = *(bool*)m_pVariables[controlid].m_Pointer;
    bool valuenew = event.IsChecked();

    // call the parent object to say it's value changed.
    if( valueold != valuenew )
    {
        // add the command to the undo stack and change the value at the same time.
        m_pCommandStack->Do( MyNew EditorCommand_PanelWatchNumberValueChanged(
            valuenew - valueold,
            m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
            m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
    }
}

void PanelWatch::OnSetFocus(wxFocusEvent& event)
{
    //LOGInfo( LOGTag, "OnSetFocus\n" );

    m_pTimer->Stop();

    event.Skip();
}

void PanelWatch::OnKillFocus(wxFocusEvent& event)
{
    //LOGInfo( LOGTag, "OnKillFocus\n" );
    m_pTimer->Start();
    event.Skip();
}

void PanelWatch::OnEditBoxKillFocus(wxFocusEvent& event)
{
    int controlid = event.GetId();
    OnTextCtrlChanged( controlid );

    OnKillFocus( event );
}

void PanelWatch::OnMouseDown(wxMouseEvent& event)
{
    //LOGInfo( LOGTag, "OnMouseDown:\n" );
    event.Skip();

    int controlid = event.GetId();
    VariableProperties* pVar = &m_pVariables[controlid];

    wxPoint pos = event.GetPosition();

    if( pVar->m_Handle_Slider )
    {
        int value = pVar->m_Handle_Slider->GetValue();

        MyAssert( false ); // if a slider is used, I'll need to retest all slider code.
        //pVar->m_CapturedMouse = true;

        pVar->m_ValueOnLeftMouseDown = value;
        pVar->m_ValueChangedSinceMouseWasPressed = false;
        pVar->m_StartMousePosition = pos;
        pVar->m_LastMousePosition = pos;
    }
    if( pVar->m_Handle_TextCtrl )
    {
        double newvalue, oldvalue;
        //bool isblank =
        GetTextCtrlValueAsDouble( controlid, &newvalue, &oldvalue );

        pVar->m_ValueOnLeftMouseDown = newvalue;
        pVar->m_ValueChangedSinceMouseWasPressed = false;
        pVar->m_StartMousePosition = pos;
        pVar->m_LastMousePosition = pos;

        pVar->m_Handle_TextCtrl->CaptureMouse();
        pVar->m_CapturedMouse = true;
    }
}

void PanelWatch::OnMouseUp(wxMouseEvent& event)
{
    //LOGInfo( LOGTag, "OnMouseUp:\n" );

    //bool showrightclickmenu = true;

    int controlid = event.GetId();

    if( m_pVariables[controlid].m_Handle_TextCtrl )
    {
        if( m_pVariables[controlid].m_CapturedMouse )
        {
            m_pVariables[controlid].m_Handle_TextCtrl->ReleaseMouse();
            m_pVariables[controlid].m_CapturedMouse = false;
        }

        double newvalue, oldvalue;
        bool isblank = GetTextCtrlValueAsDouble( controlid, &newvalue, &oldvalue );

        if( isblank == false && newvalue != m_pVariables[controlid].m_ValueOnLeftMouseDown )
        {
            SetControlValueFromDouble( controlid, newvalue, m_pVariables[controlid].m_ValueOnLeftMouseDown, true );
            //showrightclickmenu = false;
        }

        // if the value changed, then don't show the right click menu.
        if( m_pVariables[controlid].m_ValueChangedSinceMouseWasPressed )
            return;
    }

    if( m_pVariables[controlid].m_pRightClickCallbackFunc )
    {
        m_pVariables[controlid].m_pRightClickCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid );
        return;
    }

    event.Skip();
}

void PanelWatch::OnMouseMove(wxMouseEvent& event)
{
    int controlid = event.GetId();

    event.Skip();

    if( m_pVariables[controlid].m_CapturedMouse )
    {
        wxPoint pos = event.GetPosition();
        wxPoint lastpos = m_pVariables[controlid].m_LastMousePosition;
        wxPoint startpos = m_pVariables[controlid].m_StartMousePosition;

        if( pos.x != startpos.x )
        {
            if( m_pVariables[controlid].m_Handle_TextCtrl )
            {
                //m_pVariables[controlid].m_Handle_TextCtrl->WarpPointer( m_pVariables[controlid].m_StartMousePosition.x, m_pVariables[controlid].m_StartMousePosition.y );

                double newvalue, oldvalue;
                GetTextCtrlValueAsDouble( controlid, &newvalue, &oldvalue );

                int diff = pos.x - lastpos.x;

                if( m_pVariables[controlid].m_Type == PanelWatchType_Int ||
                    m_pVariables[controlid].m_Type == PanelWatchType_UnsignedInt ||
                    m_pVariables[controlid].m_Type == PanelWatchType_Char ||
                    m_pVariables[controlid].m_Type == PanelWatchType_UnsignedChar ||
                    m_pVariables[controlid].m_Type == PanelWatchType_Bool )
                {
                    // whole numbers are handled in OnMouseLeftControl
                    //newvalue += diff;
                    //m_pVariables[controlid].m_ValueChangedSinceMouseWasPressed = true;
                }
                else
                {
                    newvalue += 0.2f * diff;
                    m_pVariables[controlid].m_ValueChangedSinceMouseWasPressed = true;
                }

                //LOGInfo( LOGTag, "moved %d %d\n", pos.x, lastpos.x );

                if( newvalue != oldvalue )
                {
                    SetControlValueFromDouble( controlid, newvalue, oldvalue, false );
                }
            }
        }

        m_pVariables[controlid].m_LastMousePosition = pos;
    }
}

void PanelWatch::OnMouseLeftControl(wxMouseEvent& event)
{
    int controlid = event.GetId();

    if( m_pVariables[controlid].m_CapturedMouse )
    {
        if( m_pVariables[controlid].m_Handle_TextCtrl )
        {
            // Change all whole numbers by 1 each time the mouse leaves the control window.

            if( m_pVariables[controlid].m_Type == PanelWatchType_Int ||
                m_pVariables[controlid].m_Type == PanelWatchType_UnsignedInt ||
                m_pVariables[controlid].m_Type == PanelWatchType_Char ||
                m_pVariables[controlid].m_Type == PanelWatchType_UnsignedChar ||
                m_pVariables[controlid].m_Type == PanelWatchType_Bool )
            {
                wxPoint pos = event.GetPosition();
                wxPoint ctrlpos = m_pVariables[controlid].m_StartMousePosition;

                int direction = 1;
                if( pos.x < ctrlpos.x )
                    direction = -1;

                double newvalue, oldvalue;
                GetTextCtrlValueAsDouble( controlid, &newvalue, &oldvalue );

                newvalue += direction;
                m_pVariables[controlid].m_ValueChangedSinceMouseWasPressed = true;

                SetControlValueFromDouble( controlid, newvalue, oldvalue, false );
            }

            m_pVariables[controlid].m_Handle_TextCtrl->WarpPointer( m_pVariables[controlid].m_StartMousePosition.x, m_pVariables[controlid].m_StartMousePosition.y );
        }
        return;
    }

    event.Skip();
}

void PanelWatch::OnClickStaticText(wxMouseEvent& event)
{
    //LOGInfo( LOGTag, "OnMouseDown:\n" );
    event.Skip();

    int controlid = event.GetId();

    if( m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
        m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, true, 0 );
}

void PanelWatch::OnRightClickVariable(wxMouseEvent& event)
{
    event.Skip();

    int controlid = event.GetId();

    if( m_pVariables[controlid].m_pRightClickCallbackFunc )
        m_pVariables[controlid].m_pRightClickCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid );
}

void PanelWatch::OnTimer(wxTimerEvent& event)
{
    // moved to gamecore::tick to make it more responsive.
    //if( m_NeedsRefresh )
    //{
    //    m_RefreshCallbackFunc( m_RefreshCallbackObject );
    //    m_NeedsRefresh = false;
    //}

    UpdatePanel();
}

void PanelWatch::OnTextCtrlEnter(wxCommandEvent& event)
{
    int controlid = event.GetId();

    m_pVariables[controlid].m_Handle_TextCtrl->Navigate();
    if( controlid < m_NumVariables && m_pVariables[controlid+1].m_Handle_Slider )
    {
        m_pVariables[controlid+1].m_Handle_Slider->Navigate();
    }
    else
    {
        // TODO: support oldvalue for text controls.
        if( m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
            m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, true, 0 );
    }
}

// returns true if blank
bool PanelWatch::GetTextCtrlValueAsDouble(int controlid, double* valuenew, double* valueold)
{
    wxString wxstr = m_pVariables[controlid].m_Handle_TextCtrl->GetValue();

    bool isblank = false;
    if( wxstr == "" )
    {
        isblank = true;
        return isblank;
    }

    // evaluate wxstr as math op, not just a simple atoi or atof.
    ExprEval eval;
    double evaluatedvalue = eval.Eval( wxstr.char_str() );
    if( eval.GetErr() != EEE_NO_ERROR )
        evaluatedvalue = 0;

    //double valueint;
    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
    case PanelWatchType_Enum:
    case PanelWatchType_Flags:
        //if( isblank == false )
        {
            *valueold = *(int*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxAtoi( wxstr );
            //*((int*)m_pVariables[controlid].m_Pointer) = valueint;
        }
        break;

    case PanelWatchType_UnsignedInt:
        //if( isblank == false )
        {
            *valueold = *(unsigned int*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxAtoi( wxstr );
            //*((unsigned int*)m_pVariables[controlid].m_Pointer) = valueint;
        }
        break;

    case PanelWatchType_Char:
        //if( isblank == false )
        {
            *valueold = *(char*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxAtoi( wxstr );
            //*((char*)m_pVariables[controlid].m_Pointer) = (char)valueint;
        }
        break;

    case PanelWatchType_UnsignedChar:
        //if( isblank == false )
        {
            *valueold = *(unsigned char*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxAtoi( wxstr );
            //*((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)valueint;
        }
        break;

    case PanelWatchType_Bool:
        //if( isblank == false )
        {
            *valueold = *(bool*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxAtoi( wxstr );
            //*((bool*)m_pVariables[controlid].m_Pointer) = valueint > 0 ? true : false;
        }
        break;

    case PanelWatchType_Float:
        //if( isblank == false )
        {
            *valueold = *(float*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxstr.ToDouble( valuenew );
            //*((float*)m_pVariables[controlid].m_Pointer) = (float)valuedouble;
        }
        break;

    case PanelWatchType_Double:
        //if( isblank == false )
        {
            *valueold = *(double*)m_pVariables[controlid].m_Pointer;
            *valuenew = evaluatedvalue; //wxstr.ToDouble( valuenew );
            //*((double*)m_pVariables[controlid].m_Pointer) = valuedouble;
        }
        break;

    //case PanelWatchType_Vector3:
    //    if( isblank == false )
    //    {
    //    }

    case PanelWatchType_ColorFloat:
    case PanelWatchType_ColorByte:
    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_SpaceWithLabel:
    case PanelWatchType_Button:
    case PanelWatchType_String:
        //ADDING_NEW_WatchVariableType
    case PanelWatchType_Unknown:
    case PanelWatchType_NumTypes:
    default:
        break;
    }

    return false;//isblank;
}

void PanelWatch::SetControlValueFromDouble(int controlid, double valuenew, double valueold, bool finishedchanging)
{
    // if a range is specified, clamp the new value to that range.
    if( !fequal( m_pVariables[controlid].m_Range.x, m_pVariables[controlid].m_Range.y ) )
    {
        MyAssert( m_pVariables[controlid].m_Range.x < m_pVariables[controlid].m_Range.y );
        MyClamp( valuenew, (double)m_pVariables[controlid].m_Range.x, (double)m_pVariables[controlid].m_Range.y );
    }

    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
    case PanelWatchType_Enum:
    case PanelWatchType_Flags:
        *((int*)m_pVariables[controlid].m_Pointer) = (int)valuenew;
        break;

    case PanelWatchType_UnsignedInt:
        *((unsigned int*)m_pVariables[controlid].m_Pointer) = (unsigned int)valuenew;
        break;

    case PanelWatchType_Char:
        *((char*)m_pVariables[controlid].m_Pointer) = (char)valuenew;
        break;

    case PanelWatchType_UnsignedChar:
        *((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)valuenew;
        break;

    case PanelWatchType_Bool:
        *((bool*)m_pVariables[controlid].m_Pointer) = valuenew > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        *((float*)m_pVariables[controlid].m_Pointer) = (float)valuenew;
        break;

    case PanelWatchType_Double:
        *((double*)m_pVariables[controlid].m_Pointer) = valuenew;
        break;

    //case PanelWatchType_Vector3:
    //    *((float*)m_pVariables[controlid].m_Pointer) = value / WXSlider_Float_Multiplier;
    //    break;

    case PanelWatchType_ColorFloat:
    case PanelWatchType_ColorByte:
    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_Button:
    case PanelWatchType_String:
    case PanelWatchType_SpaceWithLabel:
    case PanelWatchType_Unknown:
    case PanelWatchType_NumTypes:
        break;
    }

    {
        if( finishedchanging )
        {
            if( valueold != valuenew )
            {
                // add the command to the undo stack and value was already changed.
                m_pCommandStack->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    valuenew - valueold,
                    m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
                    m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
    
                // call the parent object to say it's value changed... finished changing.
                if( m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
                    m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, true, valueold );
            }
        }
        else
        {
            // call the parent object to say it's value changed... slider is still held, so value isn't finished changing.
            if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
                m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, false, valueold );
        }
    }

    UpdatePanel( controlid );
}

void PanelWatch::OnTextCtrlChanged(int controlid)
{
    double valueold = 0;
    double valuenew = 0;
    GetTextCtrlValueAsDouble( controlid, &valuenew, &valueold ); //bool isblank = 

    //LOGInfo( LOGTag, "OnTextCtrlChanged start %f\n", *(float*)m_pVariables[controlid].m_Pointer );

    if( m_pVariables[controlid].m_Handle_Slider )
    {
        float sliderfloatmultiplier = 1;
        if( m_pVariables[controlid].m_Type == PanelWatchType_Float ||
            m_pVariables[controlid].m_Type == PanelWatchType_Double ) //||
            //m_pVariables[controlid].m_Type == PanelWatchType_Vector3 )
        {
            sliderfloatmultiplier = WXSlider_Float_Multiplier;
        }

        if( valuenew < m_pVariables[controlid].m_Range.x )
        {
            m_pVariables[controlid].m_Range.x = (float)valuenew;
        }
        if( valuenew > m_pVariables[controlid].m_Range.y )
        {
            m_pVariables[controlid].m_Range.y = (float)valuenew;
        }

        m_pVariables[controlid].m_Handle_Slider->SetRange( (int)(m_pVariables[controlid].m_Range.x * sliderfloatmultiplier),
                                                           (int)(m_pVariables[controlid].m_Range.y * sliderfloatmultiplier) );
    }

    if( m_pVariables[controlid].m_Type == PanelWatchType_String )
    {
        wxString wxtext = m_pVariables[controlid].m_Handle_TextCtrl->GetValue();
        const char* text = wxtext;
        sprintf_s( (char*)m_pVariables[controlid].m_Pointer, (size_t)m_pVariables[controlid].m_Range.y, "%s", text );
        int bp = 1;
    }

    // call the parent object to say it's value changed.
    //if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
    {
        if( m_pVariables[controlid].m_Type == PanelWatchType_PointerWithDesc )
        {
            // TODO: if typed into a pointer box, deal with it... along with undo/redo.
            if( m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
                m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, true, 0 );
        }
        else
        {
            if( valueold != valuenew )
            {
                // add the command to the undo stack and change the value at the same time.
                m_pCommandStack->Do( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    valuenew - valueold,
                    m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
                    m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
            }
        }
    }

    //LOGInfo( LOGTag, "OnTextCtrlChanged end %f\n", *(float*)m_pVariables[controlid].m_Pointer );
}

void PanelWatch::OnSliderChanged(wxScrollEvent& event)
{
    MyAssert( false ); // if a slider is used, I'll need to retest all slider code.

    int controlid = event.GetId();

    if( m_pVariables[controlid].m_CapturedMouse == false )
        return;

    int value = m_pVariables[controlid].m_Handle_Slider->GetValue();

    //wxEventType eventtype = event.GetEventType();
    //LOGInfo( LOGTag, "OnSliderChanged: type:%d value:%d\n", eventtype, value );

    OnSliderChanged( controlid, value, false );
}

void PanelWatch::OnSliderChanged(int controlid, int value, bool addundocommand)
{
    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
    case PanelWatchType_Enum:
    case PanelWatchType_Flags:
        *((int*)m_pVariables[controlid].m_Pointer) = value;
        break;

    case PanelWatchType_UnsignedInt:
        *((unsigned int*)m_pVariables[controlid].m_Pointer) = value;
        break;

    case PanelWatchType_Char:
        *((char*)m_pVariables[controlid].m_Pointer) = (char)value;
        break;

    case PanelWatchType_UnsignedChar:
        *((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)value;
        break;

    case PanelWatchType_Bool:
        *((bool*)m_pVariables[controlid].m_Pointer) = value > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        *((float*)m_pVariables[controlid].m_Pointer) = value / WXSlider_Float_Multiplier;
        break;

    case PanelWatchType_Double:
        *((double*)m_pVariables[controlid].m_Pointer) = value / WXSlider_Float_Multiplier;
        break;

    //case PanelWatchType_Vector3:
    //    *((float*)m_pVariables[controlid].m_Pointer) = value / WXSlider_Float_Multiplier;
    //    break;

    case PanelWatchType_ColorFloat:
    case PanelWatchType_ColorByte:
    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_Button:
    case PanelWatchType_String:
    case PanelWatchType_SpaceWithLabel:
    case PanelWatchType_Unknown:
    case PanelWatchType_NumTypes:
        break;
    }

    {
        // call the parent object to say it's value changed... slider is still held, so value isn't finished changing.
        if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
            m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, false, 0 );

        if( addundocommand )
        {
            double valuenew = value;
            double valueold = m_pVariables[controlid].m_ValueOnLeftMouseDown;

            if( m_pVariables[controlid].m_Type == PanelWatchType_Float ||
                m_pVariables[controlid].m_Type == PanelWatchType_Double ) //||
                //m_pVariables[controlid].m_Type == PanelWatchType_Vector3 )
            {
                valuenew /= WXSlider_Float_Multiplier;
                valueold /= WXSlider_Float_Multiplier;
            }

            if( valueold != valuenew )
            {
                // add the command to the undo stack and value was already changed.
                m_pCommandStack->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    valuenew - valueold,
                    m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
                    m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
    
                // call the parent object to say it's value changed... finished changing.
                if( m_pVariables[controlid].m_pOnValueChangedCallbackFunc )
                    m_pVariables[controlid].m_pOnValueChangedCallbackFunc( m_pVariables[controlid].m_pCallbackObj, controlid, true, 0 );
            }
        }
    }

    UpdatePanel( controlid );
}

void PanelWatch::OnColourPickerChanged(wxColourPickerEvent& event)
{
    int controlid = event.GetId();

    MyAssert( m_pVariables[controlid].m_Type == PanelWatchType_ColorFloat ||
              m_pVariables[controlid].m_Type == PanelWatchType_ColorByte );

    wxColour colour = m_pVariables[controlid].m_Handle_ColourPicker->GetColour();

    ColorFloat asfloats;
    asfloats.r = colour.Red()   / 255.0f;
    asfloats.g = colour.Green() / 255.0f;
    asfloats.b = colour.Blue()  / 255.0f;
    if( m_pVariables[controlid].m_Type == PanelWatchType_ColorByte )
        asfloats.a = ((ColorByte*)m_pVariables[controlid].m_Pointer)->a / 255.0f; // colour picker doesn't do alpha.
    else
        asfloats.a = ((ColorFloat*)m_pVariables[controlid].m_Pointer)->a; // colour picker doesn't do alpha.

    // add the command to the undo stack and change the value at the same time.
    m_pCommandStack->Do( MyNew EditorCommand_PanelWatchColorChanged(
        asfloats,
        m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer, controlid,
        m_pVariables[controlid].m_pOnValueChangedCallbackFunc, m_pVariables[controlid].m_pCallbackObj ) );
}

void PanelWatch::UpdatePanel(int controltoupdate)
{
    for( int i=0; i<m_NumVariables; i++ )
    {
        //if( m_pVariables[i] == 0 )
        //    continue;

        if( controltoupdate != -1 && controltoupdate != i )
            continue;

        const int MAX_STRING_LENGTH = 255;
        char tempstring[MAX_STRING_LENGTH] = "not set";
        int slidervalue = 0;
        switch( m_pVariables[i].m_Type )
        {
        case PanelWatchType_Int:
            {
                int valueint = *(int*)m_pVariables[i].m_Pointer;
                slidervalue = valueint;
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%d", valueint );
            }
            break;

        case PanelWatchType_UnsignedInt:
            {
                unsigned int valueuint = *(unsigned int*)m_pVariables[i].m_Pointer;
                slidervalue = valueuint;
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%d", valueuint );
            }
            break;

        case PanelWatchType_Char:
            {
                char valuechar = *(char*)m_pVariables[i].m_Pointer;
                slidervalue = valuechar;
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%d", valuechar );
            }
            break;

        case PanelWatchType_UnsignedChar:
            {
                unsigned char valueuchar = *(unsigned char*)m_pVariables[i].m_Pointer;
                slidervalue = valueuchar;
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%u", valueuchar );
            }
            break;

        case PanelWatchType_Bool:
            {
                bool valuebool = *(bool*)m_pVariables[i].m_Pointer;
                slidervalue = valuebool;
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%d", valuebool );
            }
            break;

        case PanelWatchType_Float:
            {
                float valuefloat = *(float*)m_pVariables[i].m_Pointer;
                slidervalue = (int)(valuefloat * WXSlider_Float_Multiplier);
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%0.2f", valuefloat );
            }
            break;

        case PanelWatchType_Double:
            {
                double valuedouble = *(double*)m_pVariables[i].m_Pointer;
                slidervalue = (int)(valuedouble * WXSlider_Float_Multiplier);
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%0.2f", valuedouble );
            }
            break;

        //case PanelWatchType_Vector3:
        //    {
        //        double valuedouble = *(double*)m_pVariables[i].m_Pointer;
        //        slidervalue = valuedouble * WXSlider_Float_Multiplier;
        //        sprintf_s( tempstring, MAX_STRING_LENGTH, "%0.2f", valuedouble );
        //    }
        //    break;

        case PanelWatchType_ColorFloat:
            {
                ColorFloat asfloats = *(ColorFloat*)m_pVariables[i].m_Pointer;
                wxColour colour( (unsigned char)(asfloats.r * 255.0f),
                                 (unsigned char)(asfloats.g * 255.0f),
                                 (unsigned char)(asfloats.b * 255.0f),
                                 (unsigned char)(asfloats.a * 255.0f) );
                m_pVariables[i].m_Handle_ColourPicker->SetColour( colour );
            }
            break;

        case PanelWatchType_ColorByte:
            {
                ColorByte* colorbyte = (ColorByte*)m_pVariables[i].m_Pointer;
                wxColour colour( colorbyte->r, colorbyte->g, colorbyte->b, colorbyte->a );
                m_pVariables[i].m_Handle_ColourPicker->SetColour( colour );
            }
            break;

        case PanelWatchType_PointerWithDesc:
            {
                sprintf_s( tempstring, MAX_STRING_LENGTH, "%s", m_pVariables[i].m_Description );
            }
            break;

        case PanelWatchType_Enum:
            {
                int valueint = *(int*)m_pVariables[i].m_Pointer;
                slidervalue = valueint;
            }
            break;

        case PanelWatchType_Flags:
            {
                int valueint = *(int*)m_pVariables[i].m_Pointer;
                slidervalue = valueint;
            }
            break;

        case PanelWatchType_String:
            {
                snprintf_s( tempstring, MAX_STRING_LENGTH, "%s", m_pVariables[i].m_Pointer );
            }
            break;

        case PanelWatchType_Button:
        case PanelWatchType_SpaceWithLabel:
        case PanelWatchType_NumTypes:
        case PanelWatchType_Unknown: // space?
            continue;
        }

        if( strcmp( tempstring, "not set" ) == 0 )
            int bp = 1;

        if( m_pVariables[i].m_Handle_ChoiceBox == 0 && m_pVariables[i].m_Handle_TextCtrl != 0 )
            m_pVariables[i].m_Handle_TextCtrl->ChangeValue( tempstring );

        if( m_pVariables[i].m_Handle_ComboBox == 0 && m_pVariables[i].m_Handle_TextCtrl != 0 )
            m_pVariables[i].m_Handle_TextCtrl->ChangeValue( tempstring );

        if( m_pVariables[i].m_Handle_Slider != 0 )
            m_pVariables[i].m_Handle_Slider->SetValue( slidervalue );

        if( m_pVariables[i].m_Handle_ChoiceBox != 0 )
            m_pVariables[i].m_Handle_ChoiceBox->SetSelection( slidervalue );

        if( m_pVariables[i].m_Handle_CheckBox != 0 )
            m_pVariables[i].m_Handle_CheckBox->SetValue( slidervalue != 0 ? true : false );
    }
}

wxString PanelWatch::GetFlagsAsString(int variablenum)
{
    wxComboCtrl* pComboCtrl = m_pVariables[variablenum].m_Handle_ComboBox;
    wxCheckListComboPopup* pCheckListComboPopup = (wxCheckListComboPopup*)pComboCtrl->GetPopupControl();

    int count = 0;
    for( int i=0; i<32; i++ )
    {
        if( pCheckListComboPopup->IsChecked( i ) )
        {
            count++;
        }
    }

    return wxString::Format( wxT("%i selected"), count );
    //return wxEmptyString;
}
