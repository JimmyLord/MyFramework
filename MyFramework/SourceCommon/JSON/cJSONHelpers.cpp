//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
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

void cJSONExt_AddIntArrayToObject(cJSON* object, const char* name, int* vars, int numinarray)
{
    cJSON* jsonarray = cJSON_CreateIntArray( vars, numinarray );
    cJSON_AddItemToObject( object, name, jsonarray );
}

void cJSONExt_GetIntArray(cJSON* object, const char* name, int* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        assert( arraysize <= numinarray );
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

void cJSONExt_GetDoubleArray(cJSON* object, const char* name, double* vars, int numinarray)
{
    cJSON* arrayobj = cJSON_GetObjectItem( object, name );
    if( arrayobj )
    {
        int arraysize = cJSON_GetArraySize( arrayobj );
        assert( arraysize <= numinarray );
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
