//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../SourceWidgets/CommandStack.h"
#include "../SourceWidgets/EditorCommands.h"

//====================================================================================================
// EditorCommand_PanelWatchNumberValueChanged
//====================================================================================================

EditorCommand_PanelWatchNumberValueChanged::EditorCommand_PanelWatchNumberValueChanged(double difference, PanelWatch_Types type, void* pointer, PanelWatchCallbackWithID callbackfunc, void* callbackobj)
{
    m_Difference = difference;
    m_Pointer = pointer;
    m_Type = type;

    m_pOnValueChangedCallBackFunc = callbackfunc;
    m_pCallbackObj = callbackobj;
}

EditorCommand_PanelWatchNumberValueChanged::~EditorCommand_PanelWatchNumberValueChanged()
{
}

void EditorCommand_PanelWatchNumberValueChanged::Do()
{
    switch( m_Type )
    {
    case PanelWatchType_Int:
        *(int*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_UnsignedInt:
        *(unsigned int*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_Char:
        *(char*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_UnsignedChar:
        *(unsigned char*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_Bool:
        *(char*)m_Pointer += m_Difference; // treating bools as char to avoid warning/error. safe?
        break;

    case PanelWatchType_Float:
        *(float*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_Double:
        *(double*)m_Pointer += m_Difference;
        break;

    case PanelWatchType_ColorFloat:
    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_Unknown:
    default:
        assert( false );
    }

    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, -1 );
}

void EditorCommand_PanelWatchNumberValueChanged::Undo()
{
    switch( m_Type )
    {
    case PanelWatchType_Int:
        *(int*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_UnsignedInt:
        *(unsigned int*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_Char:
        *(char*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_UnsignedChar:
        *(unsigned char*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_Bool:
        *(char*)m_Pointer -= m_Difference; // treating bools as char to avoid warning/error. safe?
        break;

    case PanelWatchType_Float:
        *(float*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_Double:
        *(double*)m_Pointer -= m_Difference;
        break;

    case PanelWatchType_ColorFloat:
    case PanelWatchType_PointerWithDesc:
    case PanelWatchType_Unknown:
    default:
        assert( false );
    }

    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, -1 );
}

EditorCommand* EditorCommand_PanelWatchNumberValueChanged::Repeat()
{
    EditorCommand_PanelWatchNumberValueChanged* pCommand;
    pCommand = MyNew EditorCommand_PanelWatchNumberValueChanged( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_PanelWatchColorChanged
//====================================================================================================

EditorCommand_PanelWatchColorChanged::EditorCommand_PanelWatchColorChanged(ColorFloat newcolor, PanelWatch_Types type, void* pointer, PanelWatchCallbackWithID callbackfunc, void* callbackobj)
{
    m_NewColor = newcolor;
    m_OldColor = *(ColorFloat*)pointer;
    m_Pointer = pointer;
    m_Type = type;

    m_pOnValueChangedCallBackFunc = callbackfunc;
    m_pCallbackObj = callbackobj;
}

EditorCommand_PanelWatchColorChanged::~EditorCommand_PanelWatchColorChanged()
{
}

void EditorCommand_PanelWatchColorChanged::Do()
{
    *(ColorFloat*)m_Pointer = m_NewColor;

    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, -1 );
}

void EditorCommand_PanelWatchColorChanged::Undo()
{
    *(ColorFloat*)m_Pointer = m_OldColor;

    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, -1 );
}

EditorCommand* EditorCommand_PanelWatchColorChanged::Repeat()
{
    return 0;
}

//====================================================================================================
//====================================================================================================
