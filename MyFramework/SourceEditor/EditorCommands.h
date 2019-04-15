//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorCommands_H__
#define __EditorCommands_H__

class EditorCommand;
class EditorCommand_UnloadSoundCues;
class SoundCue;

class EditorCommand
{
    friend class CommandStack;

protected:
    // If these flags are set, then the commands get undone/redone as a single entity.
    bool m_LinkedToPreviousCommandOnUndoStack;
    bool m_LinkedToNextCommandOnRedoStack; // Assigned a value during an undo op before being places on redo stack.

    unsigned int m_FrameExecuted;

    const char* m_Name; // Used for debug output and in some cases to identify command types.

public:
    EditorCommand()
    {
        m_LinkedToPreviousCommandOnUndoStack = false;
        m_LinkedToNextCommandOnRedoStack = false;
        m_FrameExecuted = 0;
        m_Name = nullptr;
    }
    virtual ~EditorCommand() {}
    virtual const char* GetName() { return m_Name; }

    virtual void Do() = 0;
    virtual void Undo() = 0;
    virtual EditorCommand* Repeat() = 0; // Make a copy of the command and execute it.
};

//====================================================================================================

class EditorCommand_UnloadSoundCues : public EditorCommand
{
protected:
    // If this is in undo stack, then this stores the only reference to the unloaded sound cue.
    std::vector<SoundCue*> m_SoundCues;
    bool m_ReleaseSoundCuesWhenDestroyed;

public:
    EditorCommand_UnloadSoundCues(const std::vector<SoundCue*>& selectedsoundcues);
    virtual ~EditorCommand_UnloadSoundCues();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================

#endif // __EditorCommands_H__
