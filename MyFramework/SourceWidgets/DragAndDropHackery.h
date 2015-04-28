//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __DragAndDropHackery_H__
#define __DragAndDropHackery_H__

// gave up on implementing drag and drop properly in wxWidgets... might revisit someday.
enum DragAndDropTypes
{
    DragAndDropType_NotSet,
    DragAndDropType_GameObjectPointer,
    DragAndDropType_ComponentPointer,
    DragAndDropType_ShaderGroupPointer,
    DragAndDropType_MaterialDefinitionPointer,
    DragAndDropType_FileObjectPointer,
    DragAndDropType_TextureDefinitionPointer,
};

struct DragAndDropStruct
{
    DragAndDropTypes m_Type;
    void* m_Value;
    int m_ID;

    void Reset()
    {
        m_Type = DragAndDropType_NotSet;
        m_Value = 0;
        m_ID = -1;
    }
};

extern DragAndDropStruct g_DragAndDropStruct;

#endif // __DragAndDropHackery_H__
