#include "CommonHeader.h"

#include "SceneGraph_Base.h"

SceneGraph_Base::SceneGraph_Base()
{
    m_pObjectPool.AllocateObjects( 10000 );
}

SceneGraph_Base::~SceneGraph_Base()
{
}
