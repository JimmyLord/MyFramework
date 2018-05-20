//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyTweener_H__
#define __MyTweener_H__

class MyTweenPool;
extern MyTweenPool* g_pTweenPool;

// http://rechneronline.de/function-graphs/
// Sine Ease in, out, in/out
//a0=2&a1=-5 * cos(x * 3.1415927/2) + 5&a2=5 * sin(x * 3.1415927/2)&a3=-5/2 * (cos(x * 3.1415927)-1)&a4=5&a5=4&a6=24&a7=&a8=&a9=&b0=500&b1=500&b2=0&b3=1&b4=0&b5=5&b6=4&b7=4&b8=5&b9=5&c0=3&c1=0&c2=1&c3=1&c4=1&c5=1&c6=1&c7=0&c8=0&c9=0&d0=1&d1=10&d2=10&d3=0&d4=0&d5=1&d6=0&d7=1&d8=0&d9=1&e0=&e1=&e2=&e3=&e4=14&e5=14&e6=13&e7=12&e8=0&e9=0&f0=0&f1=1&f2=1&f3=0&f4=0&f5=&f6=&f7=&f8=&f9=&g0=&g1=1&g2=1&g3=0&g4=0&g5=0&g6=Y&g7=ffffff&g8=a0b0c0&g9=6080a0&h0=1&z

enum MyTweenVarType
{
    MTVT_UnsignedChar,
    MTVT_Char,
    MTVT_Int,
    MTVT_Float,
};

enum MyTweenType
{
    MTT_Linear,
    MTT_SineEaseIn,
    MTT_SineEaseOut,
    MTT_SineEaseInOut,
    MTT_BounceEaseIn,
    MTT_BounceEaseOut,
    MTT_BounceEaseInOut,
    MTT_ElasticEaseIn,
    MTT_ElasticEaseOut,
    MTT_ElasticEaseInOut,
};

class TweenVar
{
public:
    MyTweenVarType m_VarType;

    MyTweenType m_TweenType;
    bool m_UseStartValueWhileWaitingForDelay;
    double m_TweenTime;
    double m_Delay;

    float m_Elasticity;

    unsigned int m_ID; // to more easily cancel a bunch tweened values.

    bool m_Done;
};

class TweenUnsignedChar : public TweenVar
{
public:
    unsigned char* m_pVariable;
    unsigned char m_StartValue;
    unsigned char m_EndValue;

    TweenUnsignedChar()
    {
        m_VarType = MTVT_UnsignedChar;
        m_pVariable = 0;
        m_StartValue = 0;
        m_EndValue = 0;
    }
};

class TweenChar : public TweenVar
{
public:
    char* m_pVariable;
    char m_StartValue;
    char m_EndValue;

    TweenChar()
    {
        m_VarType = MTVT_Char;
        m_pVariable = 0;
        m_StartValue = 0;
        m_EndValue = 0;
    }
};

class TweenInt : public TweenVar
{
public:
    int* m_pVariable;
    int m_StartValue;
    int m_EndValue;

    TweenInt()
    {
        m_VarType = MTVT_Int;
        m_pVariable = 0;
        m_StartValue = 0;
        m_EndValue = 0;
    }
};

class TweenFloat : public TweenVar
{
public:
    float* m_pVariable;
    float m_StartValue;
    float m_EndValue;

    TweenFloat()
    {
        m_VarType = MTVT_Float;
        m_pVariable = 0;
        m_StartValue = 0;
        m_EndValue = 0;
    }
};

class MyTweener
{
public:
//protected:
    MyList<TweenVar*> m_ListOfVars;

    bool m_Done;
    bool m_HoldPositionWhenDone;
    bool m_HoldVarPositionsWhenIndividualVarsAreDone;
    bool m_ExternalAllocations;

    //bool m_PlayInReverse;

    double m_ElapsedTime;

public:
    MyTweener();
    MyTweener(int numvars);
    ~MyTweener();

    bool IsDone();
    void Reset(bool removeallvariables);

    void AddUnsignedChar(unsigned char* var, unsigned char startvalue, unsigned char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void AddChar(char* var, char startvalue, char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void AddInt(int* var, int startvalue, int endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void AddFloat(float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);

    void SetUnsignedChar(int index, unsigned char startvalue, unsigned char endvalue);
    void SetChar(int index, char startvalue, char endvalue);
    void SetInt(int index, int startvalue, int endvalue);
    void SetFloat(int index, float startvalue, float endvalue);

    void SetUnsignedChar(int index, unsigned char* var, unsigned char startvalue, unsigned char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void SetChar(int index, char* var, char startvalue, char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void SetInt(int index, int* var, int startvalue, int endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);
    void SetFloat(int index, float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, unsigned int id = 0);

    void Tick(float deltaTime);
};

class MyTweenPool
{
public:
    TweenFloat* m_FloatBlockAlloc;

    MyTweener m_UnsignedChars;
    MyTweener m_Chars;
    MyTweener m_Ints;
    MyTweener m_Floats;

    unsigned int m_NumUnsignedCharsInUse;
    unsigned int m_NumCharsInUse;
    unsigned int m_NumIntsInUse;
    unsigned int m_NumFloatsInUse;

public:
    MyTweenPool(int numuchars, int numchars, int numints, int numfloats);
    ~MyTweenPool();

    void Tick(float deltaTime);

    void AddFloat(float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed = false, int id = 0);

    void CancelAllWithID(unsigned int id);
};

#endif //__MyTweener_H__
