//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "DragAndDropHackery.h"

#include <wx/treectrl.h>
#include <wx/dragimag.h>
#include <wx/dcbuffer.h>

DragAndDropStruct g_DragAndDropStruct;

void DragAndDropItem::Reset()
{
    m_Type = DragAndDropType_NotSet;
    m_Value = 0;
}

void DragAndDropItem::Set(DragAndDropTypes type, void* value)
{
    m_Type = type;
    m_Value = value;
}

void DragAndDropStruct::Clear()
{
    m_Items.clear();
    m_ControlID = -1;
}

void DragAndDropStruct::Add(DragAndDropTypes type, void* value)
{
    DragAndDropItem newitem;
    newitem.Set( type, value );

    m_Items.push_back( newitem );
}

unsigned int DragAndDropStruct::GetItemCount()
{
    return (unsigned int)m_Items.size();
}

DragAndDropItem* DragAndDropStruct::GetItem(int index)
{
    return &m_Items[index];
}

DragAndDropTreeMarker::DragAndDropTreeMarker(wxWindow* pParent)
: wxPanel( pParent, wxID_ANY, wxDefaultPosition, wxSize(80, 2) )
{
    Connect( wxEVT_PAINT, wxPaintEventHandler( DragAndDropTreeMarker::onPaint ) );
}

void DragAndDropTreeMarker::onPaint(wxPaintEvent &evt)
{
    // draw rectangle in entire 80x2 region
    wxBufferedPaintDC dc( this );
    dc.SetBrush( *wxRED_BRUSH );
    dc.DrawRectangle( 0, 0, 80, 2 );
}
