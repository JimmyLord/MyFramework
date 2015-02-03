//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "PanelWatch.h"

#include "../../Framework/MyFramework/SourceWidgets/EditorCommands.h"
#include "../../Framework/MyFramework/SourceWidgets/CommandStack.h"

PanelWatch* g_pPanelWatch = 0;

PanelWatch::PanelWatch(wxFrame* parentframe, CommandStack* pCommandStack)
: wxScrolledWindow( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Watch" )
{
    m_AllowWindowToBeUpdated = true;

    m_pCommandStack = pCommandStack;

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
        m_pVariables[i].m_SliderValueOnLeftMouseDown = 0;
        m_pVariables[i].m_SliderLeftMouseIsDown = 0;
    }

    m_pTimer = MyNew wxTimer( this, wxID_ANY );
    m_pTimer->Start( 100 );

    //Connect( wxEVT_CHILD_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus) );
    //Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus) );
    //Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    //Connect( wxEVT_SLIDER, wxScrollEventHandler(PanelWatch::OnSliderChanged) );
    Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(PanelWatch::OnTextCtrlChanged) );
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

int PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc, bool addcontrols)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( min, max );
    m_pVariables[m_NumVariables].m_Description = 0;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = 0;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallBackFunc = pOnValueChangedCallBackFunc;

    if( addcontrols )
    {
        AddControlsForVariable( name, m_NumVariables, -1, 0 );
        UpdatePanel();
    }

    m_NumVariables++;

    return m_NumVariables-1;
}

int PanelWatch::AddVariableOfType(PanelWatch_Types type, const char* name, void* pVar, const char* pDescription, void* pCallbackObj, PanelWatchCallback pOnDropCallBackFunc, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Pointer = pVar;
    m_pVariables[m_NumVariables].m_Range.Set( 0, 0 );
    m_pVariables[m_NumVariables].m_Description = pDescription;
    m_pVariables[m_NumVariables].m_Type = type;
    m_pVariables[m_NumVariables].m_pCallbackObj = pCallbackObj;
    m_pVariables[m_NumVariables].m_pOnDropCallbackFunc = pOnDropCallBackFunc;
    m_pVariables[m_NumVariables].m_pOnValueChangedCallBackFunc = pOnValueChangedCallBackFunc;

    AddControlsForVariable( name, m_NumVariables, -1, 0 );

    m_NumVariables++;

    UpdatePanel();

    return m_NumVariables-1;
}

int PanelWatch::AddInt(const char* name, int* pInt, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_Int, name, pInt, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddUnsignedInt(const char* name, unsigned int* pInt, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_UnsignedInt, name, pInt, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddChar(const char* name, char* pChar, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_Char, name, pChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddUnsignedChar(const char* name, unsigned char* pUChar, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_UnsignedChar, name, pUChar, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddBool(const char* name, bool* pBool, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_Bool, name, pBool, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddFloat(const char* name, float* pFloat, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_Float, name, pFloat, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddDouble(const char* name, double* pDouble, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_Double, name, pDouble, min, max, pCallbackObj, pOnValueChangedCallBackFunc, true );
}

int PanelWatch::AddVector2(const char* name, Vector2* pVector2, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    int first;
    first = AddVariableOfType( PanelWatchType_Float, "x", &pVector2->x, min, max, pCallbackObj, pOnValueChangedCallBackFunc, false );
    AddVariableOfType( PanelWatchType_Float, "y", &pVector2->y, min, max, pCallbackObj, pOnValueChangedCallBackFunc, false );

    AddControlsForVariable( name, first+0, 0, "x" );
    AddControlsForVariable( name, first+1, 1, "y" );

    return first;
}

int PanelWatch::AddVector3(const char* name, Vector3* pVector3, float min, float max, void* pCallbackObj, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    int first;
    first = AddVariableOfType( PanelWatchType_Float, "x", &pVector3->x, min, max, pCallbackObj, pOnValueChangedCallBackFunc, false );
    AddVariableOfType( PanelWatchType_Float, "y", &pVector3->y, min, max, pCallbackObj, pOnValueChangedCallBackFunc, false );
    AddVariableOfType( PanelWatchType_Float, "z", &pVector3->z, min, max, pCallbackObj, pOnValueChangedCallBackFunc, false );

    AddControlsForVariable( name, first+0, 0, "x" );
    AddControlsForVariable( name, first+1, 1, "y" );
    AddControlsForVariable( name, first+2, 2, "z" );

    return first;
}

int PanelWatch::AddPointerWithDescription(const char* name, void* pPointer, const char* pDescription, void* pCallbackObj, PanelWatchCallback pOnDropCallBackFunc, PanelWatchCallbackWithID pOnValueChangedCallBackFunc)
{
    return AddVariableOfType( PanelWatchType_PointerWithDesc, name, pPointer, pDescription, pCallbackObj, pOnDropCallBackFunc, pOnValueChangedCallBackFunc );
}

int PanelWatch::AddSpace()
{
    assert( m_NumVariables < MAX_PanelWatch_VARIABLES );
    if( m_NumVariables >= MAX_PanelWatch_VARIABLES )
        return -1;

    m_pVariables[m_NumVariables].m_Pointer = 0;
    m_pVariables[m_NumVariables].m_Type = PanelWatchType_Unknown;

    m_NumVariables++;

    return m_NumVariables-1;
}

void PanelWatch::AddControlsForVariable(const char* name, int variablenum, int component, const char* componentname)
{
    int PaddingTop = 3;
    int PaddingBottom = 3;
    int PaddingLeft = 2;

    int ControlPaddingTop = 3;
    int ControlPaddingBottom = 0;
    int ControlPaddingLeft = 0;

    int ControlHeight = ControlPaddingTop + 20 + ControlPaddingBottom;

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

    int WindowWidth = 300;

    int TextHeight = 20;
    int TextCtrlHeight = 20;
    int SliderHeight = 20;

    wxString variablename = name;
    
    int TextWidth = 100;
    int SliderWidth = 120;
    int TextCtrlWidth = 70;

    if( component >= 0 )
    {
        TextWidth = 5;
        SliderWidth = 30;
        TextCtrlWidth = 45;

        if( component == 0 )
        {
            TextWidth = 60;
            variablename = name;
            variablename.Append( " " );
            variablename.Append( componentname );
        }
        else
        {
            variablename = componentname;
        }
    }

    if( m_pVariables[variablenum].m_Description != 0 )
    {
        TextCtrlWidth = 150;
    }

    m_pVariables[variablenum].m_Rect_XYWH.x = PosX;
    m_pVariables[variablenum].m_Rect_XYWH.y = PosY;
    m_pVariables[variablenum].m_Rect_XYWH.z = TextWidth + SliderWidth + TextCtrlWidth;
    m_pVariables[variablenum].m_Rect_XYWH.w = ControlHeight;

    // Text label
    {
        m_pVariables[variablenum].m_Handle_StaticText = MyNew wxStaticText( this, variablenum, variablename, wxPoint(PosX, PosY), wxSize(TextWidth, TextHeight));

        PosX += TextWidth;
    }

    // Slider
    if( m_pVariables[variablenum].m_Description == 0 )
    {
        float sliderfloatmultiplier = 1;
        if( m_pVariables[variablenum].m_Type == PanelWatchType_Float ||
            m_pVariables[variablenum].m_Type == PanelWatchType_Double ) //|| 
            //m_pVariables[variablenum].m_Type == PanelWatchType_Vector3 )
        {
            sliderfloatmultiplier = WXSlider_Float_Multiplier;
        }

        m_pVariables[variablenum].m_Handle_Slider = MyNew wxSlider( this, variablenum, 0,
            m_pVariables[variablenum].m_Range.x * sliderfloatmultiplier, // min
            m_pVariables[variablenum].m_Range.y * sliderfloatmultiplier, // max
            wxPoint(PosX, PosY), wxSize(SliderWidth, SliderHeight) );

        //m_pVariables[variablenum].m_Handle_Slider->set

        PosX += SliderWidth;

        // if control gets focus, stop updates.
        m_pVariables[variablenum].m_Handle_Slider->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        m_pVariables[variablenum].m_Handle_Slider->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

        m_pVariables[variablenum].m_Handle_Slider->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler(PanelWatch::OnMouseDown), 0, this );
        m_pVariables[variablenum].m_Handle_Slider->Connect( wxEVT_LEFT_UP, wxMouseEventHandler(PanelWatch::OnMouseUp), 0, this );
        m_pVariables[variablenum].m_Handle_Slider->Connect( wxEVT_MOTION, wxMouseEventHandler(PanelWatch::OnMouseMove), 0, this );
    }

    // Edit box
    {
        wxTextCtrl* pTextCtrl = MyNew wxTextCtrl( this, variablenum, "",
            wxPoint(PosX, PosY), wxSize(TextCtrlWidth, TextCtrlHeight), wxTE_PROCESS_ENTER );

        m_pVariables[variablenum].m_Handle_TextCtrl = pTextCtrl;

        PosX += TextCtrlWidth;

        // if control gets focus, stop updates.
        pTextCtrl->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(PanelWatch::OnSetFocus), 0, this );
        pTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(PanelWatch::OnKillFocus), 0, this );

        if( m_pVariables[variablenum].m_pOnDropCallbackFunc )
        {
            PanelWatchDropTarget* pDropTarget = MyNew PanelWatchDropTarget;
            pDropTarget->m_pCallbackObj = m_pVariables[variablenum].m_pCallbackObj;
            pDropTarget->m_pCallbackFunc = m_pVariables[variablenum].m_pOnDropCallbackFunc;
            pDropTarget->m_ControlIndex = variablenum;

            pTextCtrl->SetDropTarget( pDropTarget );            
        }
    }

    int height = PaddingTop + (variablenum+1)*ControlHeight + PaddingBottom;

    SetScrollbars( 1, 1, PosX+10, height, 0, 0 );
}

PanelWatchDropTarget::PanelWatchDropTarget()
{
    SetDataObject( MyNew wxCustomDataObject );
    m_ControlIndex = -1;
}

wxDragResult PanelWatchDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    return wxDragCopy;
}

wxDragResult PanelWatchDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult defResult)
{
    // figure out which object the stuff was dropped on and let it know.
    assert( m_pCallbackObj && m_pCallbackFunc );

    g_DragAndDropStruct.m_ID = m_ControlIndex;
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

void PanelWatch::OnMouseDown(wxMouseEvent& event)
{
    //LOGInfo( LOGTag, "OnMouseDown:\n" );
    event.Skip();

    int controlid = event.GetId();
    int value = m_pVariables[controlid].m_Handle_Slider->GetValue();

    m_pVariables[controlid].m_SliderLeftMouseIsDown = true;
    m_pVariables[controlid].m_SliderValueOnLeftMouseDown = value;
}

void PanelWatch::OnMouseUp(wxMouseEvent& event)
{
    //LOGInfo( LOGTag, "OnMouseUp:\n" );
    event.Skip();

    int controlid = event.GetId();
    int value = m_pVariables[controlid].m_Handle_Slider->GetValue();

    //wxEventType eventtype = event.GetEventType();
    //LOGInfo( LOGTag, "OnMouseUp: type:%d value:%d\n", eventtype, value );

    m_pVariables[controlid].m_SliderLeftMouseIsDown = false;

    OnSliderChanged( controlid, value, true );
}

void PanelWatch::OnMouseMove(wxMouseEvent& event)
{
    int controlid = event.GetId();

    event.Skip();

    if( m_pVariables[controlid].m_SliderLeftMouseIsDown )
    {
        wxPoint pos = event.GetPosition();

        wxRect rect = m_pVariables[controlid].m_Handle_Slider->GetClientRect();
        int min = m_pVariables[controlid].m_Handle_Slider->GetMin();
        int max = m_pVariables[controlid].m_Handle_Slider->GetMax();
        if( pos.x > rect.x + rect.width )
        {
            m_pVariables[controlid].m_Handle_Slider->SetMax( max + 1000 );
        }
        if( pos.x < rect.x )
        {
            m_pVariables[controlid].m_Handle_Slider->SetMin( min - 1000 );
        }
    }
}

void PanelWatch::OnTimer(wxTimerEvent& event)
{
    UpdatePanel();
}

void PanelWatch::OnTextCtrlChanged(wxCommandEvent& event)
{
    int controlid = event.GetId();

    wxString wxstr = m_pVariables[controlid].m_Handle_TextCtrl->GetValue();

    bool isblank = false;
    if( wxstr == "" )
        isblank = true;

    // TODO: evaluate wxstr as math op, not just a simple atoi or atof.

    double valueold = 0;
    double valuenew = 0;
    //double valueint;
    switch( m_pVariables[controlid].m_Type )
    {
    case PanelWatchType_Int:
        if( isblank == false )
        {
            valueold = *(int*)m_pVariables[controlid].m_Pointer;
            valuenew = wxAtoi( wxstr );
            //*((int*)m_pVariables[controlid].m_Pointer) = valueint;
        }
        break;

    case PanelWatchType_UnsignedInt:
        if( isblank == false )
        {
            valueold = *(unsigned int*)m_pVariables[controlid].m_Pointer;
            valuenew = wxAtoi( wxstr );
            //*((unsigned int*)m_pVariables[controlid].m_Pointer) = valueint;
        }
        break;

    case PanelWatchType_Char:
        if( isblank == false )
        {
            valueold = *(char*)m_pVariables[controlid].m_Pointer;
            valuenew = wxAtoi( wxstr );
            //*((char*)m_pVariables[controlid].m_Pointer) = (char)valueint;
        }
        break;

    case PanelWatchType_UnsignedChar:
        if( isblank == false )
        {
            valueold = *(unsigned char*)m_pVariables[controlid].m_Pointer;
            valuenew = wxAtoi( wxstr );
            //*((unsigned char*)m_pVariables[controlid].m_Pointer) = (unsigned char)valueint;
        }
        break;

    case PanelWatchType_Bool:
        if( isblank == false )
        {
            valueold = *(bool*)m_pVariables[controlid].m_Pointer;
            valuenew = wxAtoi( wxstr );
            //*((bool*)m_pVariables[controlid].m_Pointer) = valueint > 0 ? true : false;
        }
        break;

    case PanelWatchType_Float:
        if( isblank == false )
        {
            valueold = *(float*)m_pVariables[controlid].m_Pointer;
            wxstr.ToDouble( &valuenew );
            //*((float*)m_pVariables[controlid].m_Pointer) = (float)valuedouble;
        }
        break;

    case PanelWatchType_Double:
        if( isblank == false )
        {
            valueold = *(double*)m_pVariables[controlid].m_Pointer;
            wxstr.ToDouble( &valuenew );
            //*((double*)m_pVariables[controlid].m_Pointer) = valuedouble;
        }
        break;

    //case PanelWatchType_Vector3:
    //    if( isblank == false )
    //    {
    //    }

    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_Unknown:
    default:
        break;
    }

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
            m_pVariables[controlid].m_Range.x = valuenew;
        }
        if( valuenew > m_pVariables[controlid].m_Range.y )
        {
            m_pVariables[controlid].m_Range.y = valuenew;
        }

        m_pVariables[controlid].m_Handle_Slider->SetRange( m_pVariables[controlid].m_Range.x * WXSlider_Float_Multiplier,
                                                           m_pVariables[controlid].m_Range.y * WXSlider_Float_Multiplier );
    }    

    // call the parent object to say it's value changed.
    if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallBackFunc )
    {
        if( m_pVariables[controlid].m_Type == PanelWatchType_PointerWithDesc )
        {
            // TODO: if typed into a pointer box, deal with it... along with undo/redo.
            m_pVariables[controlid].m_pOnValueChangedCallBackFunc( m_pVariables[controlid].m_pCallbackObj, controlid );
        }
        else
        {
            if( valueold != valuenew )
            {
                // add the command to the undo stack and change the value at the same time.
                m_pCommandStack->Do( MyNew EditorCommand_PanelWatchNumberValueChanged(
                    valuenew - valueold,
                    m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer,
                    m_pVariables[controlid].m_pOnValueChangedCallBackFunc, m_pVariables[controlid].m_pCallbackObj ) );
            }
        }
    }

    m_pVariables[controlid].m_Handle_TextCtrl->Navigate();
    if( controlid < m_NumVariables && m_pVariables[controlid+1].m_Handle_Slider )
        m_pVariables[controlid+1].m_Handle_Slider->Navigate();
}

void PanelWatch::OnSliderChanged(wxScrollEvent& event)
{
    int controlid = event.GetId();

    if( m_pVariables[controlid].m_SliderLeftMouseIsDown == false )
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
    }

    // call the parent object to say it's value changed.
    if( m_pVariables[controlid].m_pCallbackObj && m_pVariables[controlid].m_pOnValueChangedCallBackFunc )
    {
        m_pVariables[controlid].m_pOnValueChangedCallBackFunc( m_pVariables[controlid].m_pCallbackObj, controlid );

        if( addundocommand )
        {
            double valuenew = value;
            double valueold = m_pVariables[controlid].m_SliderValueOnLeftMouseDown;

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
                    m_pVariables[controlid].m_Type, m_pVariables[controlid].m_Pointer,
                    m_pVariables[controlid].m_pOnValueChangedCallBackFunc, m_pVariables[controlid].m_pCallbackObj ) );
            }
        }
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

        case PanelWatchType_UnsignedInt:
            {
                unsigned int valueuint = *(unsigned int*)m_pVariables[i].m_Pointer;
                slidervalue = valueuint;
                sprintf_s( tempstring, 50, "%d", valueuint );
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

        //case PanelWatchType_Vector3:
        //    {
        //        double valuedouble = *(double*)m_pVariables[i].m_Pointer;
        //        slidervalue = valuedouble * WXSlider_Float_Multiplier;
        //        sprintf_s( tempstring, 50, "%0.2f", valuedouble );
        //    }
        //    break;

        case PanelWatchType_PointerWithDesc:
            {
                sprintf_s( tempstring, 50, "%s", m_pVariables[i].m_Description );
            }
            break;

        case PanelWatchType_Unknown: // space?
            continue;
        }

        if( m_pVariables[i].m_Handle_TextCtrl != 0 )
            m_pVariables[i].m_Handle_TextCtrl->ChangeValue( tempstring );

        if( m_pVariables[i].m_Handle_Slider != 0 )
            m_pVariables[i].m_Handle_Slider->SetValue( slidervalue );
    }
}
