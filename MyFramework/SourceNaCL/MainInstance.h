//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MAININSTANCE_H__
#define __MAININSTANCE_H__

#include <pthread.h>
#include <map>
#include <vector>

#include "OpenGLContext.h"
#include "ppapi/cpp/instance.h"
#include "../SourceCommon/GameCore.h"

class GameCore;
class MainInstance;

extern MainInstance* g_pInstance;

class MainInstance : public pp::Instance
{
public:
    bool m_ButtonsDown[GCBI_NumButtons];
    int m_ShiftsHeld;

public:
    explicit MainInstance(PP_Instance instance);

    // The dtor makes the 3D context current before deleting the cube view, then
    // destroys the 3D context both in the module and in the browser.
    virtual ~MainInstance();

    virtual bool HandleInputEvent(const pp::InputEvent& event);

    // Called by the browser when the NaCl module is loaded and all ready to go.
    virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

    bool InitGL(int32_t new_width, int32_t new_height);

    // Called whenever the in-browser window changes size.
    virtual void DidChangeView(const pp::View& view);

    // Called by the browser to handle the postMessage() call in Javascript.
    virtual void HandleMessage(const pp::Var& message);

    // Called to draw the contents of the module's browser area.
    void DrawSelf(int32_t somevaluethecallbackfactorywants);

private:
    pp::Graphics3D m_OpenGLContext;
    pp::CompletionCallbackFactory<MainInstance> m_CallbackFactory;
};

#endif // __MAININSTANCE_H__
