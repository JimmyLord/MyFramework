//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "MyTweener.h"

double BounceEaseIn(double t, double b, double c, double d);
double BounceEaseOut(double t, double b, double c, double d);
double BounceEaseInOut(double t, double b, double c, double d);
double ElasticEaseIn(double t, double b, double c, double d, float elasticity);
double ElasticEaseOut(double t, double b, double c, double d, float elasticity);
double ElasticEaseInOut(double t, double b, double c, double d, float elasticity);

MyTweenPool* g_pTweenPool = 0;

MyTweenPool::MyTweenPool(int numuchars, int numchars, int numints, int numfloats)
: m_UnsignedChars(numuchars)
, m_Chars(numchars)
, m_Ints(numints)
, m_Floats(numfloats)
{
    int i;

    m_FloatBlockAlloc = MyNew TweenFloat[numfloats];
    for( i=0; i<numfloats; i++ )
    {
        m_FloatBlockAlloc[i].m_Done = true;
        m_Floats.m_ListOfVars.InsertAtIndex( i, &m_FloatBlockAlloc[i] );
    }
    m_Floats.m_HoldPositionWhenDone = true;
    m_Floats.m_HoldVarPositionsWhenIndividualVarsAreDone = false;
    m_Floats.m_ExternalAllocations = true;

    m_NumUnsignedCharsInUse = 0;
    m_NumCharsInUse = 0;
    m_NumIntsInUse = 0;
    m_NumFloatsInUse = 0;
}

MyTweenPool::~MyTweenPool()
{
    SAFE_DELETE_ARRAY( m_FloatBlockAlloc );
}

void MyTweenPool::Tick(double timepassed)
{
    unsigned int i;

    if( m_NumFloatsInUse > 0 )
        m_Floats.Tick( timepassed );

    for( i=0; i<m_NumFloatsInUse; i++ )
    {
        MyAssert( i < m_Floats.m_ListOfVars.Count() );

        TweenVar* pVar = m_Floats.m_ListOfVars[i];

        if( pVar->m_Done )
        {
            m_Floats.m_ListOfVars.RemoveIndex( i );
            m_Floats.m_ListOfVars.Add( pVar );
            m_NumFloatsInUse--;
        }
    }
}

void MyTweenPool::AddFloat(float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, int id)
{
    MyAssert( m_NumFloatsInUse < m_Floats.m_ListOfVars.Length() );

    m_Floats.SetFloat( m_NumFloatsInUse, var, startvalue, endvalue, tweentime, tweentype, delay + m_Floats.m_TimePassed, updatewhiledelayed, id );
    m_NumFloatsInUse++;
}

void MyTweenPool::CancelAllWithID(unsigned int id)
{
    for( unsigned int i=0; i<m_NumUnsignedCharsInUse; i++ )
    {
        MyAssert( i < m_UnsignedChars.m_ListOfVars.Count() );

        TweenVar* pVar = m_UnsignedChars.m_ListOfVars[i];
        if( pVar->m_ID == id )
        {
            pVar->m_Done = true;
            m_UnsignedChars.m_ListOfVars.RemoveIndex( i );
            m_UnsignedChars.m_ListOfVars.Add( pVar );
            m_NumUnsignedCharsInUse--;
            i--;
        }
    }

    for( unsigned int i=0; i<m_NumCharsInUse; i++ )
    {
        MyAssert( i < m_Chars.m_ListOfVars.Count() );

        TweenVar* pVar = m_Chars.m_ListOfVars[i];
        if( pVar->m_ID == id )
        {
            pVar->m_Done = true;
            m_Chars.m_ListOfVars.RemoveIndex( i );
            m_Chars.m_ListOfVars.Add( pVar );
            m_NumCharsInUse--;
            i--;
        }
    }

    for( unsigned int i=0; i<m_NumIntsInUse; i++ )
    {
        MyAssert( i < m_Ints.m_ListOfVars.Count() );

        TweenVar* pVar = m_Ints.m_ListOfVars[i];
        if( pVar->m_ID == id )
        {
            pVar->m_Done = true;
            m_Ints.m_ListOfVars.RemoveIndex( i );
            m_Ints.m_ListOfVars.Add( pVar );
            m_NumIntsInUse--;
            i--;
        }
    }

    for( unsigned int i=0; i<m_NumFloatsInUse; i++ )
    {
        MyAssert( i < m_Floats.m_ListOfVars.Count() );

        TweenVar* pVar = m_Floats.m_ListOfVars[i];
        if( pVar->m_ID == id )
        {
            pVar->m_Done = true;
            m_Floats.m_ListOfVars.RemoveIndex( i );
            m_Floats.m_ListOfVars.Add( pVar );
            m_NumFloatsInUse--;
            i--;
        }
    }
}

MyTweener::MyTweener()
: m_ListOfVars(10)
{
    m_Done = false;
    m_HoldPositionWhenDone = false;
    m_HoldVarPositionsWhenIndividualVarsAreDone = true;
    m_ExternalAllocations = false;

    m_TimePassed = 0;
}

MyTweener::MyTweener(int numvars)
: m_ListOfVars(numvars)
{
    m_Done = false;
    m_HoldPositionWhenDone = false;
    m_HoldVarPositionsWhenIndividualVarsAreDone = true;
    m_ExternalAllocations = false;

    m_TimePassed = 0;
}

MyTweener::~MyTweener()
{
    if( m_ExternalAllocations == false )
    {
        for( unsigned int i=0; i<m_ListOfVars.Count(); i++ )
        {
            delete( m_ListOfVars[i] );
        }
    }
}

bool MyTweener::IsDone()
{
    return m_Done;
}

void MyTweener::Reset(bool removeallvariables)
{
    if( removeallvariables )
    {
        for( unsigned int i=0; i<m_ListOfVars.Count(); i++ )
        {
            delete m_ListOfVars[i];
        }
        m_ListOfVars.Clear();
    }
    else
    {
        for( unsigned int i=0; i<m_ListOfVars.Count(); i++ )
        {
            m_ListOfVars[i]->m_Done = false;
        }
    }

    m_TimePassed = 0;
    m_Done = false;
}

void MyTweener::AddUnsignedChar(unsigned char* var, unsigned char startvalue, unsigned char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    TweenUnsignedChar* pVar = MyNew TweenUnsignedChar;
    m_ListOfVars.Add( (TweenVar*)pVar );

    SetUnsignedChar( m_ListOfVars.Count()-1, var, startvalue, endvalue, tweentime, tweentype, delay, updatewhiledelayed, id );
}

void MyTweener::AddChar(char* var, char startvalue, char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    TweenChar* pVar = MyNew TweenChar;
    m_ListOfVars.Add( (TweenVar*)pVar );

    SetChar( m_ListOfVars.Count()-1, var, startvalue, endvalue, tweentime, tweentype, delay, updatewhiledelayed, id );
}

void MyTweener::AddInt(int* var, int startvalue, int endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    TweenInt* pVar = MyNew TweenInt;
    m_ListOfVars.Add( (TweenVar*)pVar );

    SetInt( m_ListOfVars.Count()-1, var, startvalue, endvalue, tweentime, tweentype, delay, updatewhiledelayed, id );
}

void MyTweener::AddFloat(float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    TweenFloat* pVar = MyNew TweenFloat;
    m_ListOfVars.Add( (TweenVar*)pVar );

    SetFloat( m_ListOfVars.Count()-1, var, startvalue, endvalue, tweentime, tweentype, delay, updatewhiledelayed, id );
}

void MyTweener::SetUnsignedChar(int index, unsigned char* var, unsigned char startvalue, unsigned char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    MyAssert( index < m_ListOfVars.Count() );

    TweenUnsignedChar* pVar = (TweenUnsignedChar*)m_ListOfVars[index];

    pVar->m_pVariable = var;
    pVar->m_StartValue = startvalue;
    pVar->m_EndValue = endvalue;

    pVar->m_UseStartValueWhileWaitingForDelay = updatewhiledelayed;
    pVar->m_TweenTime = tweentime;
    pVar->m_TweenType = tweentype;
    pVar->m_Delay = delay;

    pVar->m_Elasticity = 10;

    pVar->m_ID = id;

    pVar->m_Done = false;
}

void MyTweener::SetChar(int index, char* var, char startvalue, char endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    MyAssert( index < m_ListOfVars.Count() );

    TweenChar* pVar = (TweenChar*)m_ListOfVars[index];

    pVar->m_pVariable = var;
    pVar->m_StartValue = startvalue;
    pVar->m_EndValue = endvalue;

    pVar->m_UseStartValueWhileWaitingForDelay = updatewhiledelayed;
    pVar->m_TweenTime = tweentime;
    pVar->m_TweenType = tweentype;
    pVar->m_Delay = delay;

    pVar->m_Elasticity = 10;

    pVar->m_ID = id;

    pVar->m_Done = false;
}

void MyTweener::SetInt(int index, int* var, int startvalue, int endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    MyAssert( index < m_ListOfVars.Count() );

    TweenInt* pVar = (TweenInt*)m_ListOfVars[index];

    pVar->m_pVariable = var;
    pVar->m_StartValue = startvalue;
    pVar->m_EndValue = endvalue;

    pVar->m_UseStartValueWhileWaitingForDelay = updatewhiledelayed;
    pVar->m_TweenTime = tweentime;
    pVar->m_TweenType = tweentype;
    pVar->m_Delay = delay;

    pVar->m_Elasticity = 10;

    pVar->m_ID = id;

    pVar->m_Done = false;
}

void MyTweener::SetFloat(int index, float* var, float startvalue, float endvalue, double tweentime, MyTweenType tweentype, double delay, bool updatewhiledelayed, unsigned int id)
{
    TweenFloat* pVar = (TweenFloat*)m_ListOfVars[index];

    pVar->m_pVariable = var;
    pVar->m_StartValue = startvalue;
    pVar->m_EndValue = endvalue;

    pVar->m_UseStartValueWhileWaitingForDelay = updatewhiledelayed;
    pVar->m_TweenTime = tweentime;
    pVar->m_TweenType = tweentype;
    pVar->m_Delay = delay;

    pVar->m_Elasticity = 10;

    pVar->m_ID = id;

    pVar->m_Done = false;
}

void MyTweener::SetUnsignedChar(int index, unsigned char startvalue, unsigned char endvalue)
{
    MyAssert( index < m_ListOfVars.Count() );
    MyAssert( m_ListOfVars[index] != 0 );
    MyAssert( m_ListOfVars[index]->m_VarType == MTVT_UnsignedChar );

    ( (TweenUnsignedChar*)m_ListOfVars[index] )->m_StartValue = startvalue;
    ( (TweenUnsignedChar*)m_ListOfVars[index] )->m_EndValue = endvalue;
}

void MyTweener::SetChar(int index, char startvalue, char endvalue)
{
    MyAssert( index < m_ListOfVars.Count() );
    MyAssert( m_ListOfVars[index] != 0 );
    MyAssert( m_ListOfVars[index]->m_VarType == MTVT_Char );

    ( (TweenChar*)m_ListOfVars[index] )->m_StartValue = startvalue;
    ( (TweenChar*)m_ListOfVars[index] )->m_EndValue = endvalue;
}

void MyTweener::SetInt(int index, int startvalue, int endvalue)
{
    MyAssert( index < m_ListOfVars.Count() );
    MyAssert( m_ListOfVars[index] != 0 );
    MyAssert( m_ListOfVars[index]->m_VarType == MTVT_Int );

    ( (TweenInt*)m_ListOfVars[index] )->m_StartValue = startvalue;
    ( (TweenInt*)m_ListOfVars[index] )->m_EndValue = endvalue;
}

void MyTweener::SetFloat(int index, float startvalue, float endvalue)
{
    MyAssert( index < m_ListOfVars.Count() );
    MyAssert( m_ListOfVars[index] != 0 );
    MyAssert( m_ListOfVars[index]->m_VarType == MTVT_Float );

    ( (TweenFloat*)m_ListOfVars[index] )->m_StartValue = startvalue;
    ( (TweenFloat*)m_ListOfVars[index] )->m_EndValue = endvalue;
}

void MyTweener::Tick(double timepassed)
{
    if( m_Done && m_HoldPositionWhenDone == false )
        return;

    m_TimePassed += timepassed;

    m_Done = true;

    for( unsigned int i=0; i<m_ListOfVars.Count(); i++ )
    {
        TweenVar* pVar = m_ListOfVars[i];

        if( pVar->m_Done && m_HoldVarPositionsWhenIndividualVarsAreDone == false )
            continue;

        double passed = m_TimePassed - pVar->m_Delay;
        if( passed < 0 )
        {
            m_Done = false;

            if( pVar->m_UseStartValueWhileWaitingForDelay == false )
                return; // don't affect value until delay is over...

            passed = 0;
        }

        double totaltime = pVar->m_TweenTime;
        double timeperc = passed/totaltime;

        if( timeperc > 1 )
        {
            timeperc = 1;
            pVar->m_Done = true;
        }
        else
        {
            m_Done = false;
        }

        double startvalue = 0;
        double valuerange = 0;

        if( pVar->m_VarType == MTVT_UnsignedChar )
        {
            startvalue = ((TweenUnsignedChar*)pVar)->m_StartValue;
            valuerange = ((TweenUnsignedChar*)pVar)->m_EndValue - ((TweenUnsignedChar*)pVar)->m_StartValue;
        }
        else if( pVar->m_VarType == MTVT_Char )
        {
            startvalue = ((TweenChar*)pVar)->m_StartValue;
            valuerange = ((TweenChar*)pVar)->m_EndValue - ((TweenChar*)pVar)->m_StartValue;
        }
        else if( pVar->m_VarType == MTVT_Int )
        {
            startvalue = ((TweenInt*)pVar)->m_StartValue;
            valuerange = ((TweenInt*)pVar)->m_EndValue - ((TweenInt*)pVar)->m_StartValue;
        }
        else if( pVar->m_VarType == MTVT_Float )
        {
            startvalue = ((TweenFloat*)pVar)->m_StartValue;
            valuerange = ((TweenFloat*)pVar)->m_EndValue - ((TweenFloat*)pVar)->m_StartValue;
        }

        double newvalue = 0;

        if( pVar->m_Done )
        {
            newvalue = startvalue + valuerange;
        }
        else
        {
            switch( pVar->m_TweenType )
            {
            case MTT_Linear:
                newvalue = startvalue + valuerange*timeperc;
                break;

            case MTT_SineEaseIn: // -5 * cos(x * 3.1415927/2) + 5
                newvalue = startvalue + -valuerange * cos(timeperc * PI/2) + valuerange;
                break;

            case MTT_SineEaseOut: // 5 * sin(x * 3.1415927/2)
                newvalue = startvalue + valuerange * sin(timeperc * PI/2);
                break;

            case MTT_SineEaseInOut: // -5/2 * (cos(x * 3.1415927)-1)
                newvalue = startvalue + -valuerange/2 * (cos(timeperc * PI)-1);
                break;

            case MTT_BounceEaseIn:
                newvalue = BounceEaseIn( passed, startvalue, valuerange, totaltime );
                break;

            case MTT_BounceEaseOut:
                newvalue = BounceEaseOut( passed, startvalue, valuerange, totaltime );
                break;

            case MTT_BounceEaseInOut:
                newvalue = BounceEaseInOut( passed, startvalue, valuerange, totaltime );
                break;

            case MTT_ElasticEaseIn:
                newvalue = ElasticEaseIn( passed, startvalue, valuerange, totaltime, pVar->m_Elasticity );
                break;

            case MTT_ElasticEaseOut:
                newvalue = ElasticEaseOut( passed, startvalue, valuerange, totaltime, pVar->m_Elasticity );
                break;

            case MTT_ElasticEaseInOut:
                newvalue = ElasticEaseInOut( passed, startvalue, valuerange, totaltime, pVar->m_Elasticity );
                break;
            }
        }
            
        if( pVar->m_VarType == MTVT_UnsignedChar )
        {
            *((TweenUnsignedChar*)pVar)->m_pVariable = (unsigned char)newvalue;
        }
        else if( pVar->m_VarType == MTVT_Char )
        {
            *((TweenChar*)pVar)->m_pVariable = (char)newvalue;
        }
        else if( pVar->m_VarType == MTVT_Int )
        {
            *((TweenInt*)pVar)->m_pVariable = (int)newvalue;
        }
        else if( pVar->m_VarType == MTVT_Float )
        {
            *((TweenFloat*)pVar)->m_pVariable = (float)newvalue;
        }
    }
}

//http://www.kirupa.com/forum/archive/index.php/t-76799.html
double BounceEaseIn(double t, double b, double c, double d)
{
    return c - BounceEaseOut( d-t, 0, c, d ) + b;
}

double BounceEaseOut(double t, double b, double c, double d)
{
    if( (t/=d) < (1/2.75f) )
    {
        return c*(7.5625f*t*t) + b;
    }
    else if (t < (2/2.75f))
    {
        double postFix = t-=(1.5f/2.75f);
        return c*(7.5625f*(postFix)*t + .75f) + b;
    }
    else if (t < (2.5/2.75))
    {
        double postFix = t-=(2.25f/2.75f);
        return c*(7.5625f*(postFix)*t + .9375f) + b;
    }
    else
    {
        double postFix = t-=(2.625f/2.75f);
        return c*(7.5625f*(postFix)*t + .984375f) + b;
    }
}

double BounceEaseInOut(double t, double b, double c, double d)
{
    if( t < d/2 )
        return BounceEaseIn( t*2, 0, c, d ) * .5f + b;
    else
        return BounceEaseOut( t*2-d, 0, c, d ) * .5f + c*.5f + b;
}

double ElasticEaseIn(double t, double b, double c, double d, float elasticity)
{
    t += 1.0; // hack to remove intial delay

    if( t == 0 )
        return b;
    if( t > d )
        return b+c;
    if( ( t /= d ) == 1 )
        return b+c;

    double p = d * 0.3f;
    double a = c;
    double s = p/4;

    double postFix = a * pow( 10, elasticity*(t-=1) );

    return -( postFix * sin( ( t*d-s ) * ( 2*PI )/p ) ) + b;
}

double ElasticEaseOut(double t, double b, double c, double d, float elasticity)
{
    if( t==0 )
        return b;
    if( t > d )
        return b+c;
    if( ( t /= d ) == 1 )
        return b+c;

    double p = d * 0.3f;
    double a = c;
    double s = p/4;

    return ( a * pow( 10, -elasticity*t ) * sin( ( t*d-s ) * ( 2*PI )/p ) + c + b);
}

double ElasticEaseInOut(double t, double b, double c, double d, float elasticity)
{
    if( t == 0 )
        return b;
    if( t > d )
        return b+c;
    if( ( t /= d/2 ) == 2 )
        return b+c;

    double p = d * ( 0.3f * 1.5f );
    double a = c;
    double s = p/4;

    if( t < 1 )
    {
        double postFix = a * pow( 10, elasticity*( t-=1 ) );
        return -0.5f * ( postFix * sin( ( t*d-s ) * ( 2*PI )/p ) ) + b;
    }

    double postFix = a * pow( 10, -elasticity*( t-=1 ) );
    return postFix * sin( ( t*d-s ) * ( 2*PI )/p ) * 0.5f + c + b;
}
