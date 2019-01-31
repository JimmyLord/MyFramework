//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyEvent_H__
#define __MyEvent_H__

#define EventHashType uint32
typedef EventHashType EventTypeHashFunction(const char* str);
extern EventTypeHashFunction* g_pEventTypeHashFunc;

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

    EventHashType m_NameHash;
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

class MyEvent : public TCPPListNode<MyEvent*>
{
protected:
    EventHashType m_TypeHash; // Either from EventTypes enum or calculated via hash (hash_djb).
    MyEventArgument* m_FirstArgument;

    void CheckIfArgumentIsAlreadyAttached(MyEventArgument* pNewArg);
    void AttachArgument(MyEventArgument* pArg);

public:
    MyEvent();
    ~MyEvent();

    void SetType(EventHashType hash) { m_TypeHash = hash; }
    EventHashType GetType() { return m_TypeHash; }

    bool IsType(const char* name);
    bool IsType(EventHashType hash);

    // Arguments
    void ClearArguments();

    void AttachPointer(const char* name, void* value);
    void AttachBool(const char* name, bool value);
    void AttachInt(const char* name, int32 value);
    void AttachUnsignedInt(const char* name, uint32 value);
    void AttachFloat(const char* name, float value);
    void AttachDouble(const char* name, double value);

    bool IsArgumentAttached(const char* name);

    MyEventArgument* GetArgument(const char* name);
    void* GetPointer(const char* name);
    bool GetBool(const char* name);
    int32 GetInt(const char* name);
    uint32 GetUnsignedInt(const char* name);
    float GetFloat(const char* name);
    double GetDouble(const char* name);

    MyEventArgument* GetFirstArgument() { return m_FirstArgument; }
};

#endif //__EventManager_H__
