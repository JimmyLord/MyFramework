//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __cJSON_Helpers_H__
#define __cJSON_Helpers_H__

void OverrideJSONMallocFree();
void cJSONExt_free(void* ptr);

unsigned int cJSONExt_GetDirectChildCount(cJSON* object);

void cJSONExt_AddBoolArrayToObject(cJSON* object, const char* name, bool* vars, int numinarray);
void cJSONExt_AddIntArrayToObject(cJSON* object, const char* name, int* vars, int numinarray);
void cJSONExt_AddUnsignedIntArrayToObject(cJSON* object, const char* name, uint32* vars, int numinarray);
void cJSONExt_AddFloatArrayToObject(cJSON* object, const char* name, float* vars, int numinarray);
void cJSONExt_AddFloatArrayToArray(cJSON* object, float* vars, int numinarray);
void cJSONExt_AddDoubleArrayToObject(cJSON* object, const char* name, double* vars, int numinarray);
void cJSONExt_AddUnsignedCharArrayToObject(cJSON* object, const char* name, unsigned char* vars, int numinarray);

template <typename Type> void cJSONExt_AddNumberToObjectIfDiffers(cJSON* object, const char* name, Type number, Type refnumber)
{
    if( number != refnumber )
        cJSON_AddNumberToObject( object, name, number );
}

#define cJSONExt_GetEnum(object, name, variable) \
{ \
    cJSON* obj = cJSON_GetObjectItem( object, name ); \
    if( obj ) variable = obj->valueint; \
}

void cJSONExt_GetBoolArray(cJSON* object, const char* name, bool* vars, int numinarray);
void cJSONExt_GetIntArray(cJSON* object, const char* name, int* vars, int numinarray);
void cJSONExt_GetUnsignedIntArray(cJSON* object, const char* name, uint32* vars, int numinarray);
void cJSONExt_GetFloatArray(cJSON* object, const char* name, float* vars, int numinarray);
void cJSONExt_GetFloatArrayFromArray(cJSON* arrayobject, unsigned int index, float* vars, int numinarray);
void cJSONExt_GetDoubleArray(cJSON* object, const char* name, double* vars, int numinarray);
void cJSONExt_GetUnsignedCharArray(cJSON* object, const char* name, unsigned char* vars, int numinarray);

void cJSONExt_GetBool(cJSON* object, const char* name, bool* variable);
void cJSONExt_GetUnsignedChar(cJSON* object, const char* name, unsigned char* variable);
void cJSONExt_GetChar(cJSON* object, const char* name, char* variable);
void cJSONExt_GetUnsignedInt(cJSON* object, const char* name, unsigned int* variable);
void cJSONExt_GetInt(cJSON* object, const char* name, int* variable);
void cJSONExt_GetFloat(cJSON* object, const char* name, float* variable);
void cJSONExt_GetDouble(cJSON* object, const char* name, double* variable);
void cJSONExt_GetString(cJSON* object, const char* name, char* variable, int buffersize);

void cJSONExt_GetBool(cJSON* object, const char* name, bool& variable);
void cJSONExt_GetUnsignedChar(cJSON* object, const char* name, unsigned char& variable);
void cJSONExt_GetChar(cJSON* object, const char* name, char& variable);
void cJSONExt_GetUnsignedInt(cJSON* object, const char* name, unsigned int& variable);
void cJSONExt_GetInt(cJSON* object, const char* name, int& variable);
void cJSONExt_GetFloat(cJSON* object, const char* name, float& variable);
void cJSONExt_GetDouble(cJSON* object, const char* name, double& variable);
void cJSONExt_GetString(cJSON* object, const char* name, char& variable, int buffersize);

size_t cJSONExt_GetStringLength(cJSON* object, const char* name);

void cJSONExt_ReplaceStringInJSONObject(cJSON* object, const char* newstring);

#endif //__cJSON_Helpers_H__
