//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../SourceEditor/CommandStack.h"
#include "../SourceEditor/EditorCommands.h"

#define DEBUG_EDITOR_COMMANDS 0

CommandStack::CommandStack()
{
    m_CurrentFrame = 0;
}

CommandStack::~CommandStack()
{
    ClearStacks();
}

void CommandStack::IncrementFrameCount()
{
    m_CurrentFrame++;
}

void CommandStack::ClearStacks()
{
#if DEBUG_EDITOR_COMMANDS
    LOGInfo( "EditorCommands", "ClearStacks\n" );
#endif

    while( m_UndoStack.empty() == false )
    {
        delete m_UndoStack.back();
        m_UndoStack.pop_back();
    }

    while( m_RedoStack.empty() == false )
    {
        delete m_RedoStack.back();
        m_RedoStack.pop_back();
    }
}

void CommandStack::ClearUndoStack(unsigned int numtoleave)
{
#if DEBUG_EDITOR_COMMANDS
    LOGInfo( "EditorCommands", "ClearUndoStack\n" );
#endif

    while( m_UndoStack.size() > numtoleave )
    {
        delete m_UndoStack.back();
        m_UndoStack.pop_back();
    }
}

void CommandStack::Undo(unsigned int levels)
{
#if DEBUG_EDITOR_COMMANDS
    LOGInfo( "EditorCommands", "Undo\n" );
#endif

    MyAssert( m_UndoStack.size() >= levels );

    bool previouswaslinked = false;

    for( unsigned int i=0; i<levels; i++ )
    {
        if( m_UndoStack.size() == 0 )
            return;

        EditorCommand* pCommand = m_UndoStack.back(); m_UndoStack.pop_back();
        pCommand->Undo();

#if DEBUG_EDITOR_COMMANDS
        LOGInfo( "EditorCommands", "Undo: (%d)%s\n", pCommand->m_FrameExecuted, pCommand->m_Name );
#endif

        if( previouswaslinked )
            pCommand->m_LinkedToNextCommandOnRedoStack = true; // mark this command as being linked to the next one on the redo stack.

        if( pCommand->m_LinkedToPreviousCommandOnUndoStack )
        {
            previouswaslinked = true;
            levels++;
        }

        m_RedoStack.push_back( pCommand );
    }
}

void CommandStack::Redo(unsigned int levels)
{
#if DEBUG_EDITOR_COMMANDS
    LOGInfo( "EditorCommands", "Redo\n" );
#endif

    MyAssert( m_RedoStack.size() >= levels );

    for( unsigned int i=0; i<levels; i++ )
    {
        if( m_RedoStack.size() == 0 )
            return;

        EditorCommand* pCommand = m_RedoStack.back(); m_RedoStack.pop_back();
        pCommand->Do();

#if DEBUG_EDITOR_COMMANDS
        LOGInfo( "EditorCommands", "Do: (%d)%s\n", pCommand->m_FrameExecuted, pCommand->m_Name );
#endif

        if( pCommand->m_LinkedToNextCommandOnRedoStack )
            levels++;

        m_UndoStack.push_back( pCommand );
    }
}

void CommandStack::Do(EditorCommand* pCommand, bool linktoprevious, bool autolinkifsameframeasprevious)
{
    // Add the command to the undo stack before executing, allowing redo stack to be wiped
    Add( pCommand, linktoprevious, autolinkifsameframeasprevious );

    pCommand->Do();
}

void CommandStack::Add(EditorCommand* pCommand, bool linktoprevious, bool autolinkifsameframeasprevious)
{
    MyAssert( pCommand );
    if( pCommand == 0 )
        return;

    // Figure out if command should be linked to previous
    {
        pCommand->m_LinkedToPreviousCommandOnUndoStack = false;

        // Set command to be linked to the previous command if it was executed on same frame (and link wanted)
        pCommand->m_FrameExecuted = m_CurrentFrame;
        if( autolinkifsameframeasprevious && m_UndoStack.size() > 0 )
        {
            if( pCommand->m_FrameExecuted == m_UndoStack.back()->m_FrameExecuted )
                pCommand->m_LinkedToPreviousCommandOnUndoStack = true;
        }

        // Force link
        if( linktoprevious )
        {
            pCommand->m_LinkedToPreviousCommandOnUndoStack = linktoprevious;
        }

#if DEBUG_EDITOR_COMMANDS
        LOGInfo( "EditorCommands", "Add: (%d)%s\n", pCommand->m_FrameExecuted, pCommand->m_Name );
#endif
    }

    // Add to undo stack
    m_UndoStack.push_back( pCommand );

    // Wipe out the redo stack
    {
        for( unsigned int i=0; i<m_RedoStack.size(); i++ )
            delete( m_RedoStack[i] );
    
        m_RedoStack.clear();
    }
}
