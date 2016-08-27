//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyEvent_H__
#define __MyEvent_H__

struct MyEventArgument
{
    enum ArgumentTypes
    {
        Type_Pointer,
        Type_Bool,
        Type_Int,
        Type_UnsignedInt,
        Type_Float,
        Type_Double,
    };

    union
    {
        char m_NameStr[8];
        uint64 m_NameInt;
    };

    ArgumentTypes m_Type;

    union
    {
        void* m_Pointer;
        bool m_Bool;
        int32 m_Int;
        uint32 m_UnsignedInt;
        float m_Float;
        double m_Double;
    };

    MyEventArgument* m_NextArgument;
};

class MyEvent
{
protected:
    EventTypes m_Type;
    MyEventArgument* m_FirstArgument;

    void CheckIfArgumentIsAlreadyAttached(char* name);
    void AttachArgument(MyEventArgument* pArg);

public:
    MyEvent();
    ~MyEvent();

    void SetType(EventTypes type) { m_Type = type; }
    EventTypes GetType() { return m_Type; }

    // Arguments
    void ClearArguments();

    void AttachPointer(char* name, void* value);
    void AttachBool(char* name, bool value);
    void AttachInt(char* name, int32 value);
    void AttachUnsignedInt(char* name, uint32 value);
    void AttachFloat(char* name, float value);
    void AttachDouble(char* name, double value);

    MyEventArgument* GetArgument(char* name);
    void* GetPointer(char* name);
    bool GetBool(char* name);
    int32 GetInt(char* name);
    uint32 GetUnsignedInt(char* name);
    float GetFloat(char* name);
    double GetDouble(char* name);

    MyEventArgument* GetFirstArgument() { return m_FirstArgument; }
};

#endif //__EventManager_H__
