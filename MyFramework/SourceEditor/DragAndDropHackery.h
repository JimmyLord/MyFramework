//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
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
    DragAndDropType_SoundCuePointer,
    DragAndDropType_MenuItem,

    DragAndDropType_UserDefinedTypes,
};

struct DragAndDropItem
{
    DragAndDropTypes m_Type;
    void* m_Value;

    void Reset();
    void Set(DragAndDropTypes type, void* value);
};

struct DragAndDropStruct
{
protected:
    std::vector<DragAndDropItem> m_Items;
    int m_ControlID;

public:
    void Clear();

    void SetControlID(int id) { m_ControlID = id; }
    int GetControlID() { return m_ControlID; }

    void Add(DragAndDropTypes type, void* value);
    unsigned int GetItemCount();
    DragAndDropItem* GetItem(int index);
};

extern DragAndDropStruct g_DragAndDropStruct;

#if MYFW_USING_WX
// Attempt to add a visual marker when I drag/drop from the trees
//   idea taken from: https://forums.wxwidgets.org/viewtopic.php?t=40168
class DragAndDropTreeMarker : public wxPanel
{
protected:
    void onPaint(wxPaintEvent &evt);

public:
    DragAndDropTreeMarker(wxWindow* pParent);
};
#endif

#endif // __DragAndDropHackery_H__
