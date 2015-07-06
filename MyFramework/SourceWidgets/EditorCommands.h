//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorCommands_H__
#define __EditorCommands_H__

class EditorCommand
{
    friend class CommandStack;

protected:
    // if these flags are set, then the commands get undone/redone as a single entity.
    bool m_LinkedToPreviousCommandOnUndoStack;
    bool m_LinkedToNextCommandOnRedoStack; // assigned a value during an undo op before being places on redo stack.

public:
    EditorCommand()
    {
        m_LinkedToPreviousCommandOnUndoStack = false;
        m_LinkedToNextCommandOnRedoStack = false;
    }
    virtual ~EditorCommand() {}

    virtual void Do() = 0;
    virtual void Undo() = 0;
    virtual EditorCommand* Repeat() = 0; // make a copy of the command and execute it.
};

//====================================================================================================

class EditorCommand_PanelWatchNumberValueChanged : public EditorCommand
{
protected:
    double m_Difference;
    PanelWatch_Types m_Type;
    void* m_Pointer;
    int m_ControlID;

    PanelWatchCallbackValueChanged m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_PanelWatchNumberValueChanged(double difference, PanelWatch_Types type, void* pointer, int controlid, PanelWatchCallbackValueChanged callbackfunc, void* callbackobj);
    virtual ~EditorCommand_PanelWatchNumberValueChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_PanelWatchColorChanged : public EditorCommand
{
protected:
    ColorFloat m_NewColor;
    ColorFloat m_OldColor;
    PanelWatch_Types m_Type;
    void* m_Pointer;
    int m_ControlID;

    PanelWatchCallbackValueChanged m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_PanelWatchColorChanged(ColorFloat newcolor, PanelWatch_Types type, void* pointer, int controlid, PanelWatchCallbackValueChanged callbackfunc, void* callbackobj);
    virtual ~EditorCommand_PanelWatchColorChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

class EditorCommand_PanelWatchPointerChanged : public EditorCommand
{
protected:
    void* m_NewValue;
    void* m_OldValue;
    PanelWatch_Types m_Type;
    void** m_pPointer;
    int m_ControlID;

    PanelWatchCallbackValueChanged m_pOnValueChangedCallBackFunc;
    void* m_pCallbackObj;

public:
    EditorCommand_PanelWatchPointerChanged(void* newvalue, PanelWatch_Types type, void** ppointer, int controlid, PanelWatchCallbackValueChanged callbackfunc, void* callbackobj);
    virtual ~EditorCommand_PanelWatchPointerChanged();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

#endif // __EditorCommands_H__
