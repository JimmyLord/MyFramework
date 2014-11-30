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
: wxPanel( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600) )
{
    m_AllowWindowToBeUpdated = true;

    m_NumVariables = 0;
    m_SliderBeingDragged = -1;

    m_Handles_StaticText = MyNew wxStaticText*[MAX_PanelWatch_VARIABLES];
    m_Handles_TextCtrl = MyNew wxTextCtrl*[MAX_PanelWatch_VARIABLES];
    m_Handles_Slider = MyNew wxSlider*[MAX_PanelWatch_VARIABLES];
    m_pVariablePointers = MyNew void*[MAX_PanelWatch_VARIABLES];
    m_pVariableRanges = MyNew Vector2[MAX_PanelWatch_VARIABLES];
    m_pVariableDescriptions = MyNew const char*[MAX_PanelWatch_VARIABLES];
    m_pVariableTypes = MyNew PanelWatch_Types[MAX_PanelWatch_VARIABLES];
    m_pVariableCallbackObjs = MyNew void*[MAX_PanelWatch_VARIABLES];
    m_pVariableCallbackFuncs = MyNew PanelWatchCallback[MAX_PanelWatch_VARIABLES];

    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        m_Handles_StaticText[i] = 0;
        m_Handles_TextCtrl[i] = 0;
        m_Handles_Slider[i] = 0;
        m_pVariablePointers[i] = 0;
        m_pVariableRanges[i].Set( 0, 0 );
        m_pVariableDescriptions[i] = 0;
        m_pVariableTypes[i] = PanelWatchType_Unknown;
        m_pVariableCallbackObjs[i] = 0;
        m_pVariableCallbackFuncs[i] = 0;
    }

    m_pTimer = MyNew wxTimer(this, wxID_ANY);
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

    SAFE_DELETE_ARRAY( m_Handles_StaticText );
    SAFE_DELETE_ARRAY( m_Handles_TextCtrl );
    SAFE_DELETE_ARRAY( m_Handles_Slider );

    SAFE_DELETE_ARRAY( m_pVariablePointers );
    SAFE_DELETE_ARRAY( m_pVariableRanges );
    SAFE_DELETE_ARRAY( m_pVariableDescriptions );
    SAFE_DELETE_ARRAY( m_pVariableTypes );
    SAFE_DELETE_ARRAY( m_pVariableCallbackObjs );
    SAFE_DELETE_ARRAY( m_pVariableCallbackFuncs );

    SAFE_DELETE( m_pTimer );
}

void PanelWatch::ClearAllVariables()
{
    for( int i=0; i<MAX_PanelWatch_VARIABLES; i++ )
    {
        if( m_Handles_StaticText[i] != 0 )
            this->RemoveChild( m_Handles_StaticText[i] );
        if( m_Handles_TextCtrl[i] != 0 )
            this->RemoveChild( m_Handles_TextCtrl[i] );
        if( m_Handles_Slider[i] != 0 )
            this->RemoveChild( m_Handles_Slider[i] );

        SAFE_DELETE( m_Handles_StaticText[i] );
        SAFE_DELETE( m_Handles_TextCtrl[i] );
        SAFE_DELETE( m_Handles_Slider[i] );

        m_Handles_StaticText[i] = 0;
        m_Handles_TextCtrl[i] = 0;
        m_Handles_Slider[i] = 0;
        m_pVariablePointers[i] = 0;
        m_pVariableRanges[i].Set( 0, 0 );
        m_pVariableDescriptions[i] = 0;
        m_pVariableTypes[i] = PanelWatchType_Unknown;
        m_pVariableCallbackObjs[i] = 0;
        m_pVariableCallbackFuncs[i] = 0;
    }

    m_NumVariables = 0;
}

void PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, float min, float max)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariablePointers[m_NumVariables] = pVar;
    m_pVariableRanges[m_NumVariables].Set( min, max );
    m_pVariableDescriptions[m_NumVariables] = 0;
    m_pVariableTypes[m_NumVariables] = type;
    m_pVariableCallbackObjs[m_NumVariables] = 0;
    m_pVariableCallbackFuncs[m_NumVariables] = 0;

    AddControlsForVariable( name );

    m_NumVariables++;

    UpdatePanel();
}

void PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallback pCallBackFunc)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariablePointers[m_NumVariables] = pVar;
    m_pVariableRanges[m_NumVariables].Set( 0, 0 );
    m_pVariableDescriptions[m_NumVariables] = pDescription;
    m_pVariableTypes[m_NumVariables] = type;
    m_pVariableCallbackObjs[m_NumVariables] = pCallbackObj;
    m_pVariableCallbackFuncs[m_NumVariables] = pCallBackFunc;

    AddControlsForVariable( name );

    m_NumVariables++;

    UpdatePanel();
}

void PanelWatch::AddInt(const char* name, int* pInt, float min, float max)
{
    AddVariableOfType( PanelWatchType_Int, name, pInt, min, max );
}

void PanelWatch::AddChar(const char* name, char* pChar, float min, float max)
{
    AddVariableOfType( PanelWatchType_Char, name, pChar, min, max );
}

void PanelWatch::AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max)
{
    AddVariableOfType( PanelWatchType_UnsignedChar, name, pUChar, min, max );
}

void PanelWatch::AddBool(const char* name, bool* pBool, float min, float max)
{
    AddVariableOfType( PanelWatchType_Bool, name, pBool, min, max );
}

void PanelWatch::AddFloat(const char* name, float* pFloat, float min, float max)
{
    AddVariableOfType( PanelWatchType_Float, name, pFloat, min, max );
}

void PanelWatch::AddDouble(const char* name, double* pDouble, float min, float max)
{
    AddVariableOfType( PanelWatchType_Double, name, pDouble, min, max );
}

void PanelWatch::AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj, PanelWatchCallback pCallBackFunc)
{
    AddVariableOfType( PanelWatchType_PointerWithDesc, name, pPointer, pDescription, pCallbackObj, pCallBackFunc );
}

void PanelWatch::AddSpace()
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return;

    m_pVariablePointers[m_NumVariables] = 0;
    m_pVariableTypes[m_NumVariables] = PanelWatchType_Unknown;

    m_NumVariables++;
}

void PanelWatch::AddControlsForVariable(const char* name)
{
    int PaddingTop = 3;
    int PaddingBottom = 0;
    int PaddingLeft = 2;

    int ControlHeight = PaddingTop + 20 + PaddingBottom;

    int PosX = PaddingLeft;
    int PosY = m_NumVariables*ControlHeight;
    int WindowWidth = 300;

    int TextHeight = 20;
    int TextCtrlHeight = 20;
    int SliderHeight = 20;

    int TextWidth = 100;
    int SliderWidth = 120;
    int TextCtrlWidth = 70;

    if( m_pVariableDescriptions[m_NumVariables] != 0 )
    {
        TextCtrlWidth = 150;
    }

    PosY += PaddingTop;

    // Text label
    {
        m_Handles_StaticText[m_NumVariables] = MyNew wxStaticText( this, m_NumVariables, name, wxPoint(PosX, PosY), wxSize(TextWidth, TextHeight));

        PosX += TextWidth;
    }

    // Slider
    if( m_pVariableDescriptions[m_NumVariables] == 0 )
    {
        float sliderfloatmultiplier = 1;
        if( m_pVariableTypes[m_NumVariables] == PanelWatchType_Float ||
            m_pVariableTypes[m_NumVariables] == PanelWatchType_Double )
        {
            sliderfloatmultiplier = WXSlider_Float_Multiplier;
        }

        m_Handles_Slider[m_NumVariables] = MyNew wxSlider( this, m_NumVariables, 0,
            m_pVariableRanges[m_NumVariables].x * sliderfloatmultiplier, // min
            m_pVariableRanges[m_NumVariables].y * sliderfloatmultiplier, // max
            wxPoint(PosX, PosY), wxSize(SliderWidth, SliderHeight) );

        PosX += SliderWidth;

        // if control gets focus, stop updates.
        m_Handles_Slider[m_NumVariables]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        m_Handles_Slider[m_NumVariables]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );
    }

    // Edit box
    {
        m_Handles_TextCtrl[m_NumVariables] = MyNew wxTextCtrl( this, m_NumVariables, "",
            wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight) );
        
        // if control gets focus, stop updates.
        m_Handles_TextCtrl[m_NumVariables]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        m_Handles_TextCtrl[m_NumVariables]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

        if( m_pVariableCallbackFuncs[m_NumVariables] )
        {
            PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
            pDropTarget->m_pCallbackObj = m_pVariableCallbackObjs[m_NumVariables];
            pDropTarget->m_pCallbackFunc = m_pVariableCallbackFuncs[m_NumVariables];

            m_Handles_TextCtrl[m_NumVariables]->SetDropTarget( pDropTarget );            
        }
    }
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

    wxString wxstr = m_Handles_TextCtrl[controlid]->GetValue();

    double valuedouble;
    double valueint;
    switch( m_pVariableTypes[controlid] )
    {
    case PanelWatchType_Int:
        valueint = wxAtoi( wxstr );
        *((int*)m_pVariablePointers[controlid]) = valueint;
        break;

    case PanelWatchType_Char:
        valueint = wxAtoi( wxstr );
        *((char*)m_pVariablePointers[controlid]) = (char)valueint;
        break;

    case PanelWatchType_UnsignedChar:
        valueint = wxAtoi( wxstr );
        *((unsigned char*)m_pVariablePointers[controlid]) = (unsigned char)valueint;
        break;

    case PanelWatchType_Bool:
        valueint = wxAtoi( wxstr );
        *((bool*)m_pVariablePointers[controlid]) = valueint > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        wxstr.ToDouble( &valuedouble );
        *((float*)m_pVariablePointers[controlid]) = (float)valuedouble;
        break;

    case PanelWatchType_Double:
        wxstr.ToDouble( &valuedouble );
        *((double*)m_pVariablePointers[controlid]) = valuedouble;
        break;
    }

    UpdatePanel( controlid );
}

void PanelWatch::OnSliderChanged(wxScrollEvent& event)
{
    int controlid = event.GetId();

    switch( m_pVariableTypes[controlid] )
    {
    case PanelWatchType_Int:
        *((int*)m_pVariablePointers[controlid]) = m_Handles_Slider[controlid]->GetValue();
        break;

    case PanelWatchType_Char:
        *((char*)m_pVariablePointers[controlid]) = (char)m_Handles_Slider[controlid]->GetValue();
        break;

    case PanelWatchType_UnsignedChar:
        *((unsigned char*)m_pVariablePointers[controlid]) = (unsigned char)m_Handles_Slider[controlid]->GetValue();
        break;

    case PanelWatchType_Bool:
        *((bool*)m_pVariablePointers[controlid]) = m_Handles_Slider[controlid]->GetValue() > 0 ? true : false;
        break;

    case PanelWatchType_Float:
        *((float*)m_pVariablePointers[controlid]) = m_Handles_Slider[controlid]->GetValue() / WXSlider_Float_Multiplier;
        break;

    case PanelWatchType_Double:
        *((double*)m_pVariablePointers[controlid]) = m_Handles_Slider[controlid]->GetValue() / WXSlider_Float_Multiplier;
        break;
    }

    UpdatePanel( controlid );
}

void PanelWatch::UpdatePanel(int controltoupdate)
{
    for( int i=0; i<m_NumVariables; i++ )
    {
        //if( m_pVariablePointers[i] == 0 )
        //    continue;

        if( controltoupdate != -1 && controltoupdate != i )
            continue;

        char tempstring[50] = "not set";
        int slidervalue;
        switch( m_pVariableTypes[i] )
        {
        case PanelWatchType_Int:
            {
                int valueint = *(int*)m_pVariablePointers[i];
                slidervalue = valueint;
                sprintf_s( tempstring, 50, "%d", valueint );
            }
            break;

        case PanelWatchType_Char:
            {
                char valuechar = *(char*)m_pVariablePointers[i];
                slidervalue = valuechar;
                sprintf_s( tempstring, 50, "%d", valuechar );
            }
            break;

        case PanelWatchType_UnsignedChar:
            {
                unsigned char valueuchar = *(unsigned char*)m_pVariablePointers[i];
                slidervalue = valueuchar;
                sprintf_s( tempstring, 50, "%u", valueuchar );
            }
            break;

        case PanelWatchType_Bool:
            {
                bool valuebool = *(bool*)m_pVariablePointers[i];
                slidervalue = valuebool;
                sprintf_s( tempstring, 50, "%d", valuebool );
            }
            break;

        case PanelWatchType_Float:
            {
                float valuefloat = *(float*)m_pVariablePointers[i];
                slidervalue = valuefloat * WXSlider_Float_Multiplier;
                sprintf_s( tempstring, 50, "%0.2f", valuefloat );
            }
            break;

        case PanelWatchType_Double:
            {
                double valuedouble = *(double*)m_pVariablePointers[i];
                slidervalue = valuedouble * WXSlider_Float_Multiplier;
                sprintf_s( tempstring, 50, "%0.2f", valuedouble );
            }
            break;

        case PanelWatchType_PointerWithDesc:
            {
                sprintf_s( tempstring, 50, "%s", m_pVariableDescriptions[i] );
            }
            break;

        case PanelWatchType_Unknown: // space?
            continue;
        }

        m_Handles_TextCtrl[i]->ChangeValue( tempstring );
        if( m_Handles_Slider[i] != 0 )
            m_Handles_Slider[i]->SetValue( slidervalue );
    }
}
