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

CommandStack::CommandStack()
{
}

CommandStack::~CommandStack()
{
    ClearStacks();
}

void CommandStack::ClearStacks()
{
    for( unsigned int i=0; i<m_UndoStack.size(); i++ )
        delete( m_UndoStack[i] );

    for( unsigned int i=0; i<m_RedoStack.size(); i++ )
        delete( m_RedoStack[i] );
}

void CommandStack::Undo(unsigned int levels)
{
    assert( m_UndoStack.size() >= levels );

    for( unsigned int i=0; i<levels; i++ )
    {
        if( m_UndoStack.size() == 0 )
            return;

        EditorCommand* pCommand = m_UndoStack.back(); m_UndoStack.pop_back();
        pCommand->Undo();

        m_RedoStack.push_back( pCommand );
    }
}

void CommandStack::Redo(unsigned int levels)
{
    assert( m_RedoStack.size() >= levels );

    for( unsigned int i=0; i<levels; i++ )
    {
        if( m_RedoStack.size() == 0 )
            return;

        EditorCommand* pCommand = m_RedoStack.back(); m_RedoStack.pop_back();
        pCommand->Do();

        m_UndoStack.push_back( pCommand );
    }
}

void CommandStack::Do(EditorCommand* pCommand)
{
    pCommand->Do();

    Add( pCommand );
}

void CommandStack::Add(EditorCommand* pCommand)
{
    assert( pCommand );
    if( pCommand == 0 )
        return;

    m_UndoStack.push_back( pCommand );

    for( unsigned int i=0; i<m_RedoStack.size(); i++ )
        delete( m_RedoStack[i] );
    
    m_RedoStack.clear();
}
