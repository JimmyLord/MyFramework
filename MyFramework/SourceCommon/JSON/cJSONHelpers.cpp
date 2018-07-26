//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

static bool OverrodeJSONMemoryAllocation = false;

// To get access to these funcs/vars from cjson, I had to make the not-static in cJSON.cpp
extern void *(*cJSON_malloc)(size_t sz);
extern void (*cJSON_free)(void *ptr);
extern char* cJSON_strdup(const char* str);

void* MyJSONMalloc(size_t sz)
{
    // TODO: change to use malloc and free, deleting void* is a bad idea even though we don't need to call destructor.
    return MyNew unsigned char[sz];
}

void MyJSONFree(void* ptr)
{
    // TODO: change to use malloc and free, deleting void* is a bad idea even though we don't need to call destructor.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
#endif

    delete[] ptr;

#if __GNUC__
#pragma GCC diagnostic pop
#endif
}

void cJSONExt_free(void* ptr)
{
    // TODO: change to use malloc and free, deleting void* is a bad idea even though we don't need to call destructor.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
#endif

    if( OverrodeJSONMemoryAllocation )
        delete[] ptr;
    else
        free( ptr );

#if __GNUC__
#pragma GCC diagnostic pop
#endif
}

void OverrideJSONMallocFree()
{
    OverrodeJSONMemoryAllocation = true;

    cJSON_Hooks hooks;
    hooks.malloc_fn = MyJSONMalloc;
    hooks.free_fn = MyJSONFree;
    cJSON_InitHooks( &hooks );
}

unsigned int cJSONExt_GetDirectChildCount(cJSON* object)
{
    unsigned int count = 0;

    cJSON* child = object->child;

    while( child )
    {
        count++;
        child = child->next;
    }

    return count;
}

void cJSONExt_AddIntArrayToObject(cJSON* object, const char* name, int* vars, int numinarray)
{
    cJSON* jsonarray = cJSON_CreateIntArray( vars, numinarray );
    cJSON_AddItemToObject( object, name, jsonarray );
}

void cJSONExt_AddUnsignedIntArrayToObject(cJSON* object, const char* name, uint32* vars, int numinarray)
{
    int i;
    cJSON *n=0,*p=0,*a=cJSON_CreateArray();
    for(i=0;a && i<numinarray;i++)
    {
        n=cJSON_CreateNumber(vars[i]);
        if(!i)
            a->child=n;
        else
        {
            p->next=n;
            n->prev=p;
        }
        p=n;
    }

    cJSON_AddItemToObject( object, name, a );
}

void cJSONExt_AddFloatArrayToObject(cJSON* object, const char* name, float* vars, int numinarray)
{
    cJSON* jsonarray = cJSON_CreateFloatArray( vars, numinarray );
    cJSON_AddItemToObject( object, name, jsonarray );
}

void cJSONExt_AddFloatArrayToArray(cJSON* object, float* vars, int numinarray)
{
    cJSON* jsonarray = cJSON_CreateFloatArray( vars, numinarray );
    cJSON_AddItemToArray( object, jsonarray );
}

void cJSONExt_AddDoubleArrayToObject(cJSON* object, const char* name, double* vars, int numinarray)
{
    cJSON* jsonarray = cJSON_CreateDoubleArray( vars, numinarray );
    cJSON_AddItemToObject( object, name, jsonarray );
}

void cJSONExt_AddUnsignedCharArrayToObject(cJSON* object, const char* name, unsigned char* vars, int numinarray)
{
    int i;
    cJSON *n=0,*p=0,*a=cJSON_CreateArray();
    for(i=0;a && i<numinarray;i++)
    {
        n=cJSON_CreateNumber(vars[i]);
        if(!i)
            a->child=n;
        else
        {
            p->next=n;
            n->prev=p;
        }
        p=n;
    }

    cJSON_AddItemToObject( object, name, a );
}

void cJSONExt_GetIntArray(cJSON* object, const char* name, int* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        MyAssert( arraysize <= numinarray );
        for( int i=0; i<arraysize; i++ )
        {
            if( i >= numinarray )
                return;

            cJSON* obj = cJSON_GetArrayItem( arrayobj, i );
            if( obj )
                vars[i] = obj->valueint;
        }
    }
}

void cJSONExt_GetFloatArray(cJSON* object, const char* name, float* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        MyAssert( arraysize <= numinarray );
        for( int i=0; i<arraysize; i++ )
        {
            if( i >= numinarray )
                return;

            cJSON* obj = cJSON_GetArrayItem( arrayobj, i );
            if( obj )
                vars[i] = (float)obj->valuedouble;
        }
    }
}

void cJSONExt_GetFloatArrayFromArray(cJSON* arrayobject, unsigned int index, float* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetArrayItem( arrayobject, index );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        MyAssert( arraysize <= numinarray );
        for( int i=0; i<arraysize; i++ )
        {
            if( i >= numinarray )
                return;

            cJSON* obj = cJSON_GetArrayItem( arrayobj, i );
            if( obj )
                vars[i] = (float)obj->valuedouble;
        }
    }
}

void cJSONExt_GetDoubleArray(cJSON* object, const char* name, double* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        MyAssert( arraysize <= numinarray );
        for( int i=0; i<arraysize; i++ )
        {
            if( i >= numinarray )
                return;

            cJSON* obj = cJSON_GetArrayItem( arrayobj, i );
            if( obj )
                vars[i] = obj->valuedouble;
        }
    }
}

void cJSONExt_GetUnsignedCharArray(cJSON* object, const char* name, unsigned char* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        MyAssert( arraysize <= numinarray );
        for( int i=0; i<arraysize; i++ )
        {
            if( i >= numinarray )
                return;

            cJSON* obj = cJSON_GetArrayItem( arrayobj, i );
            if( obj )
                vars[i] = (unsigned char)obj->valuedouble;
        }
    }
}

void cJSONExt_GetBool(cJSON* object, const char* name, bool* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = obj->valueint > 0 ? true : false;
}

void cJSONExt_GetUnsignedChar(cJSON* object, const char* name, unsigned char* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = (unsigned char)obj->valueint;
}

void cJSONExt_GetChar(cJSON* object, const char* name, char* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = (char)obj->valueint;
}

void cJSONExt_GetUnsignedInt(cJSON* object, const char* name, unsigned int* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = (unsigned int)obj->valuedouble;
}

void cJSONExt_GetInt(cJSON* object, const char* name, int* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = obj->valueint;
}

void cJSONExt_GetFloat(cJSON* object, const char* name, float* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = (float)obj->valuedouble;
}

void cJSONExt_GetDouble(cJSON* object, const char* name, double* variable)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj ) *variable = obj->valuedouble;
}

void cJSONExt_GetString(cJSON* object, const char* name, char* variable, int buffersize)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj )
        strcpy_s( variable, buffersize, obj->valuestring );
}

size_t cJSONExt_GetStringLength(cJSON* object, const char* name)
{
    cJSON* obj = cJSON_GetObjectItem( object, name );
    if( obj )
        return strlen( obj->valuestring );
    
    return 0;
}

void cJSONExt_ReplaceStringInJSONObject(cJSON* object, const char* newstring)
{
    MyAssert( !(object->type&cJSON_IsReference) && object->valuestring );
    if( !(object->type&cJSON_IsReference) && object->valuestring )
    {
        cJSON_free( object->valuestring );
        object->valuestring = cJSON_strdup( newstring );
    }
}
