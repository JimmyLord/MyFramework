//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __CommandStack_H__
#define __CommandStack_H__

class EditorCommand;

class CommandStack
{
protected:
    std::vector<EditorCommand*> m_UndoStack;
    std::vector<EditorCommand*> m_RedoStack;

    unsigned int m_CurrentFrame;

public:
    CommandStack();
    virtual ~CommandStack();

    void IncrementFrameCount();

    unsigned int GetUndoStackSize() { return m_UndoStack.size(); }
    unsigned int GetRedoStackSize() { return m_RedoStack.size(); }
    EditorCommand* GetUndoCommandAtIndex(unsigned int index) { return m_UndoStack[index]; }

    void ClearStacks();
    void ClearUndoStack(unsigned int numtoleave = 0);

    virtual void Undo(unsigned int levels);
    virtual void Redo(unsigned int levels);

    virtual void Do(EditorCommand* pCommand, bool linktoprevious = false, bool autolinkifsameframeasprevious = true);
    virtual void Add(EditorCommand* pCommand, bool linktoprevious = false, bool autolinkifsameframeasprevious = true);
};

#endif // __CommandStack_H__
