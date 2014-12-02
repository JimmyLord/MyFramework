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
#include "PanelWatch.h"

PanelWatch* g_pPanelWatch = 0;

PanelWatch::PanelWatch(wxFrame* parentframe)
: wxScrolledWindow( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Watch" )
{
    m_AllowWindowToBeUpdated = true;

    m_NumVariables = 0;
    m_SliderBeingDragged = -1;

    m_pVariables = MyNew VariableProperties[MAX_PanelWatch_VARIABLES];

    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        m_pVariables[i].m_Handle_StaticText = 0;
        m_pVariables[i].m_Handle_TextCtrl = 0;
        m_pVariables[i].m_Handle_Slider = 0;
        m_pVariables[i].m_Pointer = 0;
        m_pVariables[i].m_Range.Set( 0, 0 );
        m_pVariables[i].m_Description = 0;
        m_pVariables[i].m_Type = PanelWatchType_Unknown;
        m_pVariables[i].m_pCallbackObj = 0;
        m_pVariables[i].m_pOnDropCallbackFunc = 0;
        m_pVariables[i].m_pOnValueChangedCallBackFunc = 0;
    }

    m_pTimer = MyNew wxTimer( this, wxID_ANY );
    m_pTimer->Start( 100 );

    //Connect( wxEVT_CHILD_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus) );
    //Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus) );
    Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(PanelWatch::OnTextCtrlChanged) );
    Connect( wxEVT_TIMER, wxTimerEventHandler(PanelWatch::OnTimer) );
}

PanelWatch::~PanelWatch()
{
    ClearAllVariables();

    SAFE_DELETE_ARRAY( m_pVariables );

    SAFE_DELETE( m_pTimer );
}

void PanelWatch::ClearAllVariables()
{
    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        if( m_pVariables[i].m_Handle_StaticText != 0 )
            this->RemoveChild( m_pVariables[i].m_Handle_StaticText );
        if( m_pVariables[i].m_Handle_TextCtrl != 0 )
            this->RemoveChild( m_pVariables[i].m_Handle_TextCtrl );
        if( m_pVariables[i].m_Handle_Slider != 0 )
            this->RemoveChild( m_pVariables[i].m_Handle_Slider );

        SAFE_DELETE( m_pVariables[i].m_Handle_StaticText );
        SAFE_DELETE( m_pVariables[i].m_Handle_TextCtrl );
        SAFE_DELETE( m_pVariables[i].m_Handle_Slider );

        m_pVariables[i].m_Handle_StaticText = 0;
        m_pVariables[i].m_Handle_TextCtrl = 0;
        m_pVariables[i].m_Handle_Slider = 0;
        m_pVariables[i].m_Pointer = 0;
        m_pVariables[i].m_Range.Set( 0, 0 );
        m_pVariables[i].m_Description = 0;
        m_pVariables[i].m_Type = PanelWatchType_Unknown;
        m_pVariables[i].m_pCallbackObj = 0;
        m_pVariables[i].m_pOnDropCallbackFunc = 0;
    }

    m_NumVariables = 0;
}

void PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( min, max );
    m_pVariables[m_NumVariables].m_Description = 0;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallBackFunc = pOnValueChangedCallBackFunc;

    AddControlsForVariable( name );

    m_NumVariables++;

    UpdatePanel();
}

void PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallback pOnDropCallBackFunc, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( 0, 0 );
    m_pVariables[m_NumVariables].m_Description = pDescription;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = pOnDropCallBackFunc;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallBackFunc = pOnValueChangedCallBackFunc;

    AddControlsForVariable( name );

    m_NumVariables++;

    UpdatePanel();
}

void PanelWatch::AddInt(const char* name, int* pInt, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_Int, name, pInt, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddChar(const char* name, char* pChar, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_Char, name, pChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_UnsignedChar, name, pUChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddBool(const char* name, bool* pBool, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_Bool, name, pBool, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddFloat(const char* name, float* pFloat, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_Float, name, pFloat, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddDouble(const char* name, double* pDouble, float min, float max, void* pCallbackObj, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_Double, name, pDouble, min, max, pCallbackObj, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj, PanelWatchCallback pOnDropCallBackFunc, PanelWatchCallback pOnValueChangedCallBackFunc)
{
    AddVariableOfType( PanelWatchType_PointerWithDesc, name, pPointer, pDescription, pCallbackObj, pOnDropCallBackFunc, pOnValueChangedCallBackFunc );
}

void PanelWatch::AddSpace()
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariables[m_NumVariables].m_Pointer = 0;
    m_pVariables[m_NumVariables].m_Type = PanelWatchType_Unknown;

    m_NumVariables++;
}

void PanelWatch::AddControlsForVariable(const char* name)
{
    int PaddingTop = 3;
    int PaddingBottom = 3;
    int PaddingLeft = 2;

    int ControlPaddingTop = 3;
    int ControlPaddingBottom = 0;
    int ControlPaddingLeft = 0;

    int ControlHeight = ControlPaddingTop + 20 + ControlPaddingBottom;

    int PosX = PaddingLeft + ControlPaddingLeft;
    int PosY = PaddingTop + m_NumVariables*ControlHeight;
    int WindowWidth = 300;

    int TextHeight = 20;
    int TextCtrlHeight = 20;
    int SliderHeight = 20;

    int TextWidth = 100;
    int SliderWidth = 120;
    int TextCtrlWidth = 70;

    if( m_pVariables[m_NumVariables].m_Description != 0 )
    {
        TextCtrlWidth = 150;
    }

    // Text label
    {
        m_pVariables[m_NumVariables].m_Handle_StaticText = MyNew wxStaticText( this, m_NumVariables, name, wxPoint(PosX, PosY), wxSize(TextWidth, TextHeight));

        PosX += TextWidth;
    }

    // Slider
    if( m_pVariables[m_NumVariables].m_Description == 0 )
    {
        float sliderfloatmultiplier = 1;
        if( m_pVariables[m_NumVariables].m_Type == PanelWatchType_Float ||
            m_pVariables[m_NumVariables].m_Type == PanelWatchType_Double )
        {
            sliderfloatmultiplier = WXSlider_Float_Multiplier;
        }

        m_pVariables[m_NumVariables].m_Handle_Slider = MyNew wxSlider( this, m_NumVariables, 0,
            m_pVariables[m_NumVariables].m_Range.x * sliderfloatmultiplier, // min
            m_pVariables[m_NumVariables].m_Range.y * sliderfloatmultiplier, // max
            wxPoint(PosX, PosY), wxSize(SliderWidth, SliderHeight) );

        PosX += SliderWidth;

        // if control gets focus, stop updates.
        m_pVariables[m_NumVariables].m_Handle_Slider->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        m_pVariables[m_NumVariables].m_Handle_Slider->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );
    }

    // Edit box
    {
        m_pVariables[m_NumVariables].m_Handle_TextCtrl = MyNew wxTextCtrl( this, m_NumVariables, "",
            wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight) );
        
        PosX += TextCtrlWidth;

        // if control gets focus, stop updates.
        m_pVariables[m_NumVariables].m_Handle_TextCtrl->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        m_pVariables[m_NumVariables].m_Handle_TextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

        if( m_pVariables[m_NumVariables].m_pOnDropCallbackFunc )
        {
            PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
            pDropTarget->m_pCallbackObj = m_pVariables[m_NumVariables].m_pCallbackObj;
            pDropTarget->m_pCallbackFunc = m_pVariables[m_NumVariables].m_pOnDropCallbackFunc;

            m_pVariables[m_NumVariables].m_Handle_TextCtrl->SetDropTarget( pDropTarget );            
        }
    }

    int height = PaddingTop + (m_NumVariables+1)*ControlHeight + PaddingBottom;

    SetScrollbars( 1, 1, PosX+10, height, 0, 0 );
}

PanelWatchDropTarget::PanelWatchDropTarget()
{
    SetDataObject(MyNew wxCustomDataObject);
}

wxDragResult PanelWatchDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    return wxDragCopy;
}

wxDragResult PanelWatchDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult defResult)
{
    // figure out which object the stuff was dropped on and let it know.
    assert( m_pCallbackObj && m_pCallbackFunc );

    m_pCallbackFunc( m_pCallbackObj );

    return wxDragNone;
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

void PanelWatch::OnTimer(wxTimerEvent& event)
{
    UpdatePanel();
}

void PanelWatch::OnTextCtrlChanged(wxCommandEvent& event)
{
    int controlid = event.GetId();

    wxString wxstr = m_pVariables[controlid].m_Handle_TextCtrl->GetValue();

    if( wxstr == "" )
        return;

    // TODO: evaluate wxstr as math op, not just a simple atoi or atof.

    double valuedouble;
    double valueint;
    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
        valueint = wxAtoi( wxstr );
        *((int*)m_pVariables[controlid].m_Pointer) = valueint;
        break;

    case PanelWatchType_Char:
        valueint = wxAtoi( wxstr );
        *((char*)m_pVariables[controlid].m_Pointer) = (char)valueint;
        break;

    case PanelWatchType_UnsignedChar:
        valueint = wxAtoi( wxstr );
        *((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)valueint;
        break;

    case PanelWatchType_Bool:
        valueint = wxAtoi( wxstr );
        *((bool*)m_pVariables[controlid].m_Pointer) = valueint > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        wxstr.ToDouble( &valuedouble );
        *((float*)m_pVariables[controlid].m_Pointer) = (float)valuedouble;
        break;

    case PanelWatchType_Double:
        wxstr.ToDouble( &valuedouble );
        *((double*)m_pVariables[controlid].m_Pointer) = valuedouble;
        break;
    }

    // call the parent object to say it's value changed.
    if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallBackFunc )
    {
        m_pVariables[controlid].m_pOnValueChangedCallBackFunc( m_pVariables[controlid].m_pCallbackObj );
    }
}

void PanelWatch::OnSliderChanged(wxScrollEvent& event)
{
    int controlid = event.GetId();

    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
        *((int*)m_pVariables[controlid].m_Pointer) = m_pVariables[controlid].m_Handle_Slider->GetValue();
        break;

    case PanelWatchType_Char:
        *((char*)m_pVariables[controlid].m_Pointer) = (char)m_pVariables[controlid].m_Handle_Slider->GetValue();
        break;

    case PanelWatchType_UnsignedChar:
        *((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)m_pVariables[controlid].m_Handle_Slider->GetValue();
        break;

    case PanelWatchType_Bool:
        *((bool*)m_pVariables[controlid].m_Pointer) = m_pVariables[controlid].m_Handle_Slider->GetValue() > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        *((float*)m_pVariables[controlid].m_Pointer) = m_pVariables[controlid].m_Handle_Slider->GetValue() / WXSlider_Float_Multiplier;
        break;

    case PanelWatchType_Double:
        *((double*)m_pVariables[controlid].m_Pointer) = m_pVariables[controlid].m_Handle_Slider->GetValue() / WXSlider_Float_Multiplier;
        break;
    }

    // call the parent object to say it's value changed.
    if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallBackFunc )
    {
        m_pVariables[controlid].m_pOnValueChangedCallBackFunc( m_pVariables[controlid].m_pCallbackObj );
    }

    UpdatePanel( controlid );
}

void PanelWatch::UpdatePanel(int controltoupdate)
{
    for( int i=0; i<m_NumVariables; i++ )
    {
        //if( m_pVariables[i] == 0 )
        //    continue;

        if( controltoupdate != -1 && controltoupdate != i )
            continue;

        char tempstring[50] = "not set";
        int slidervalue;
        switch( m_pVariables[i].m_Type )
        {
        case PanelWatchType_Int:
            {
                int valueint = *(int*)m_pVariables[i].m_Pointer;
                slidervalue = valueint;
                sprintf_s( tempstring, 50, "%d", valueint );
            }
            break;

        case PanelWatchType_Char:
            {
                char valuechar = *(char*)m_pVariables[i].m_Pointer;
                slidervalue = valuechar;
                sprintf_s( tempstring, 50, "%d", valuechar );
            }
            break;

        case PanelWatchType_UnsignedChar:
            {
                unsigned char valueuchar = *(unsigned char*)m_pVariables[i].m_Pointer;
                slidervalue = valueuchar;
                sprintf_s( tempstring, 50, "%u", valueuchar );
            }
            break;

        case PanelWatchType_Bool:
            {
                bool valuebool = *(bool*)m_pVariables[i].m_Pointer;
                slidervalue = valuebool;
                sprintf_s( tempstring, 50, "%d", valuebool );
            }
            break;

        case PanelWatchType_Float:
            {
                float valuefloat = *(float*)m_pVariables[i].m_Pointer;
                slidervalue = valuefloat * WXSlider_Float_Multiplier;
                sprintf_s( tempstring, 50, "%0.2f", valuefloat );
            }
            break;

        case PanelWatchType_Double:
            {
                double valuedouble = *(double*)m_pVariables[i].m_Pointer;
                slidervalue = valuedouble * WXSlider_Float_Multiplier;
                sprintf_s( tempstring, 50, "%0.2f", valuedouble );
            }
            break;

        case PanelWatchType_PointerWithDesc:
            {
                sprintf_s( tempstring, 50, "%s", m_pVariables[i].m_Description );
            }
            break;

        case PanelWatchType_Unknown: // space?
            continue;
        }

        m_pVariables[i].m_Handle_TextCtrl->ChangeValue( tempstring );
        if( m_pVariables[i].m_Handle_Slider != 0 )
            m_pVariables[i].m_Handle_Slider->SetValue( slidervalue );
    }
}
