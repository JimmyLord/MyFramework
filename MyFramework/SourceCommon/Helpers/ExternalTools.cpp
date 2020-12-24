//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include <sstream>
#if MYFW_WINDOWS
#include <direct.h>
#endif

#include "ExternalTools.h"

#if MYFW_ANDROID
#include "../../SourceAndroid/JavaInterfaceCPP.h"
#endif

#if MYFW_IOS
#include "../../SourceIOS/ExternalToolsIOS.h"
#endif

#if MYFW_USING_WX
#include "../../SourceWidgets/MYFWMainWx.h"
#endif

#if MYFW_WINDOWS && !MYFW_USING_WX
#include "../../SourceWindows/MYFWWinMain.h"
#endif

#if MYFW_BLACKBERRY
#include <bps/virtualkeyboard.h>
#include <bps/navigator.h>
#include "../../SourceBlackBerry/BBMain.h"
#endif

#if MYFW_WP8
char g_TextToShare_Subject[1000];
char g_TextToShare_Body[1000];
#endif

#if MYFW_NACL
#include "../../../MyFramework/SourceNaCL/MainInstance.h"
#endif

void SetRenderMode(bool continuous)
{
#if MYFW_ANDROID
    LOGInfo( LOGTag, "[Flow] Setting render mode() %d\n", continuous );

    if( g_pMainActivity )
    {
        jclass jActivity = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
        if( jActivity )
        {
            jmethodID jSetRenderMode = g_pJavaEnvironment->GetMethodID( jActivity, "SetRenderMode", "(Z)V" );
            g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, jSetRenderMode, continuous );
        }
    }
#endif
}

void SetExitOnBackButton(bool exit)
{
#if MYFW_ANDROID
    extern bool g_Android_ExitOnBackButton;
    g_Android_ExitOnBackButton = exit;
#endif
}

void SetMusicVolume(float volume)
{
#if MYFW_ANDROID
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "SetMusicVolume() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "SetMusicVolume", "(I)V" );
    if( methodid == 0 )
        return;
    g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, methodid, (int)volume );
    //LOGInfo( LOGTag, "SetMusicVolume %d", (int)volume );
#elif MYFW_IOS
    IOSSetMusicVolume( volume );
#elif MYFW_WINDOWS
#elif MYFW_BLACKBERRY
    if( g_pGameCore->GetMediaPlayer() == 0 )
        return;

    if( volume == 0 )
    {
        g_pGameCore->GetMediaPlayer()->Pause();
    }
    else
    {
        g_pGameCore->GetMediaPlayer()->Play();
        g_pGameCore->GetMediaPlayer()->SetVolume( volume );
    }
#elif MYFW_WP8
    if( g_pGameCore->GetSoundPlayer() == 0 )
        return;

    if( volume == 0 )
    {
        g_pGameCore->GetSoundPlayer()->PauseMusic();
    }
    else
    {
        //g_pGameCore->GetSoundPlayer()->PlayMusic(0);
        g_pGameCore->GetSoundPlayer()->UnpauseMusic();
    }
#endif
}

void LaunchURL(const char* url)
{
#if MYFW_ANDROID
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LaunchURL() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    LOGInfo( LOGTag, "LaunchURL %s, javaenv %s", url, (char*)g_pJavaEnvironment );

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    LOGInfo( LOGTag, "g_pMainActivity cls %p", cls );

    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "LaunchURL", "(Ljava/lang/String;)V" );

    LOGInfo( LOGTag, "LaunchURL %p", methodid );

    if( methodid == 0 )
        return;

    jstring jurl = g_pJavaEnvironment->NewStringUTF( url );

    g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, methodid, jurl );

    g_pJavaEnvironment->DeleteLocalRef( jurl );
#elif MYFW_IOS
    IOSLaunchURL( url );
#elif MYFW_WINDOWS
    //wchar_t urlwide[512];
    //mbstowcs_s( 0, urlwide, url, 512 );
    ShellExecute( NULL, "open", url, NULL, NULL, SW_SHOWNORMAL );
#elif MYFW_BLACKBERRY
    navigator_invoke( url, 0 );
#elif MYFW_WP8
    //const char* url = "http://www.flatheadgames.com";
    //const char* url = "zune:reviewapp";
    wchar_t wideurl[100];
    MultiByteToWideChar( CP_ACP, 0, url, strlen(url), wideurl, 100 );
    wideurl[strlen(url)] = 0;
    auto uri = ref new Windows::Foundation::Uri( ref new Platform::String( wideurl ) );
    Windows::System::Launcher::LaunchUriAsync( uri );

    //concurrency::task<bool> launchUriOperation(Windows::System::Launcher::LaunchUriAsync(uri));
    //launchUriOperation.then([](bool success)
    //{
    //    if (success)
    //    {
    //        // URI launched
    //    }
    //    else
    //    {
    //        // URI launch failed
    //    }
    //});
         //launchUriOperation( uri );
    //MarketplaceDetailTask marketplaceDetailTask = new MarketplaceDetailTask();
    //marketplaceDetailTask.ContentIdentifier = null;
    //marketplaceDetailTask.Show();
#endif
}

#if MYFW_EDITOR
void LaunchFileInDefaultApp(const char* filePath)
{
#if MYFW_WINDOWS
    char fullPath[MAX_PATH];

    // If the file path contains a ':', consider it a full path, otherwise consider it relative.
    if( strstr( filePath, ":" ) )
    {
        sprintf_s( fullPath, MAX_PATH, "%s", filePath );
    }
    else
    {
        char workingDir[MAX_PATH];
        _getcwd( workingDir, MAX_PATH * sizeof(char) );
        sprintf_s( fullPath, MAX_PATH, "%s/%s", workingDir, filePath );
    }

    ShellExecuteA( 0, nullptr, fullPath, nullptr, nullptr, SW_SHOWNORMAL );
#endif
}

void LaunchApplication(const char* appname, const char* arguments, bool hidden, bool async, std::vector<std::string>* pOutput)
{
#if MYFW_WINDOWS
    {
        HANDLE hChildStdoutRd = 0;
        HANDLE hChildStdoutWr = 0;

        // Setup some structs needed to create the stdout pipe.
        SECURITY_ATTRIBUTES securityAttributes;
        securityAttributes.nLength = sizeof( SECURITY_ATTRIBUTES );
        securityAttributes.bInheritHandle = true;
        securityAttributes.lpSecurityDescriptor = nullptr;

        // Create a pipe and set StdOut's Inherit flag to false.
        bool success = CreatePipe( &hChildStdoutRd, &hChildStdoutWr, &securityAttributes, 0 );
        MyAssert( success );
        success = SetHandleInformation( hChildStdoutRd, HANDLE_FLAG_INHERIT, 0 );
        MyAssert( success );

        // Setup some structs needed to create the process with the pipe created above.
        PROCESS_INFORMATION processInfo;
        ZeroMemory( &processInfo, sizeof( PROCESS_INFORMATION ) );

        STARTUPINFO startupInfo;
        ZeroMemory( &startupInfo, sizeof( STARTUPINFO ) );
        startupInfo.cb = sizeof( STARTUPINFO );
        startupInfo.hStdError = hChildStdoutWr;
        startupInfo.hStdOutput = hChildStdoutWr;
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::string command = std::string( "cmd.exe /C \"" ) + std::string( appname ) + "\" " + arguments;

        DWORD creationFlags = 0;
        if( hidden )
            creationFlags |= CREATE_NO_WINDOW;

        // Execute a synchronous child process & get exit code.
        success = CreateProcess(
            nullptr,                // Application name.
            (LPSTR)command.c_str(), // Command line.
            nullptr,                // Process security attributes.
            nullptr,                // Primary thread security attributes.
            true,                   // Handles are inherited.
            creationFlags,          // Creation flags.
            nullptr,                // Use parent's environment.
            nullptr,                // Use parent's current directory.
            &startupInfo,           // STARTUPINFO pointer.
            &processInfo            // Receives PROCESS_INFORMATION.
        );
        MyAssert( success );

        DWORD exitCode = 1;
        WaitForSingleObject( processInfo.hProcess, INFINITE );
        GetExitCodeProcess( processInfo.hProcess, &exitCode );

        // Return if the caller is not requesting the stdout results.
        if( pOutput == nullptr )
            return;

        // Get the number of chars in the pipe.
        DWORD bytesInPipe = 0;
        success = PeekNamedPipe( hChildStdoutRd, nullptr, 0, nullptr, &bytesInPipe, nullptr );
        MyAssert( success );
        if( bytesInPipe == 0 )
            return;

        // Read the data written to the pipe.
        DWORD bytesRead;
        char* pipeContents = new char[bytesInPipe+1];
        success = ReadFile( hChildStdoutRd, pipeContents, bytesInPipe, &bytesRead, nullptr );
        pipeContents[bytesInPipe] = '\0';
        MyAssert( success || bytesRead == 0 );

        // Split the data into lines and add them to the return vector.
        std::stringstream stream( pipeContents );
        std::string str;
        while( getline( stream, str ) )
        {
            pOutput->push_back( str );
        }

        delete[] pipeContents;

        CloseHandle( processInfo.hProcess );
        CloseHandle( processInfo.hThread );
    }
#elif MYFW_OSX
    // TODO: pass arguments to process.
    system( appname );
#endif
}
#endif

void ShareString(const char* subject, const char* body)
{
#if MYFW_ANDROID
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "ShareString() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    LOGInfo( LOGTag, "g_pMainActivity cls %p", cls );

    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "ShareString", "(Ljava/lang/String;Ljava/lang/String;)V" );

    LOGInfo( LOGTag, "ShareString %p", methodid );
    if( methodid == 0 )
        return;

    jstring jsubject = g_pJavaEnvironment->NewStringUTF( subject );
    jstring jbody = g_pJavaEnvironment->NewStringUTF( body );

    g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, methodid, jsubject, jbody );

    g_pJavaEnvironment->DeleteLocalRef( jsubject );
    g_pJavaEnvironment->DeleteLocalRef( jbody );
#elif MYFW_BLACKBERRY
    // todo
#elif MYFW_WP8
    strcpy_s( g_TextToShare_Subject, 1000, subject );
    strcpy_s( g_TextToShare_Body, 1000, body );

    Windows::ApplicationModel::DataTransfer::DataTransferManager::ShowShareUI();
#endif
}

void ShowKeyboard(bool show)
{
#if MYFW_ANDROID
    LOGInfo( LOGTag, "ShowKeyboard\n" );
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "ShowKeyboard() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    LOGInfo( LOGTag, "g_pJavaEnvironment %p\n", g_pJavaEnvironment );

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    LOGInfo( LOGTag, "g_pMainActivity cls %p\n", cls );

    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "ShowKeyboard", "(Z)V" );

    LOGInfo( LOGTag, "ShowKeyboard %p\n", methodid );
    if( methodid == 0 )
        return;

    jboolean jshow = show;

    g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, methodid, jshow );
    LOGInfo( LOGTag, "~ShowKeyboard\n" );
#elif MYFW_BLACKBERRY
    if( show )
    {
        virtualkeyboard_change_options(VIRTUALKEYBOARD_LAYOUT_DEFAULT, VIRTUALKEYBOARD_ENTER_DEFAULT);
        virtualkeyboard_show();
    }
    else
    {
        virtualkeyboard_hide();
    }
#elif MYFW_IOS
    IOSShowKeyboard( show );
#endif
}

bool PlatformSpecific_CheckKeyState(int keycode)
{
#if MYFW_USING_WX && !MYFW_WINDOWS
    return wxGetKeyState( (wxKeyCode)keycode );
#elif MYFW_WINDOWS
    if( keycode == MYKEYCODE_LCTRL )        return (GetKeyState( VK_CONTROL ) & 0x8000) ? true : false;
    else if( keycode == MYKEYCODE_LALT )    return (GetKeyState( VK_MENU ) & 0x8000)    ? true : false;
    else if( keycode == MYKEYCODE_LSHIFT )  return (GetKeyState( VK_SHIFT ) & 0x8000)   ? true : false;
    else if( keycode == ' ' )               return (GetKeyState( VK_SPACE ) & 0x8000)   ? true : false;

    // default: just pass the keycode through, may not always work in wx where mykeycodes match wx's not win32's
    else return GetKeyState( keycode ) ? true : false;
#else
    //MyAssert( false ); // fix me on mac/linux?
#endif

    return false;
}

void PlatformSpecific_SetMousePosition(float x, float y)
{
#if MYFW_USING_WX
    g_pMainApp->m_pMainFrame->m_pGLCanvas->WarpPointer( (int)x, (int)y );
#elif MYFW_NACL
    MyAssert( false ); // TODO: fix me on nacl
#else
    //MyAssert( false ); // TODO: fix me on everything else...
#endif
}

void PlatformSpecific_SetMouseLock(bool lock)
{
#if MYFW_USING_WX
    SetMouseLock( lock );
#elif MYFW_WINDOWS
    SetMouseLock( lock );
#elif MYFW_BLACKBERRY
    SetMouseLock( lock );
#elif MYFW_NACL
    g_pInstance->SetMouseLock( lock );
#else
    //MyAssert( false ); // TODO: fix me on everything else...
#endif
}

bool PlatformSpecific_IsMouseLocked()
{
#if MYFW_WINDOWS || MYFW_BLACKBERRY
    return IsMouseLocked();
#elif MYFW_NACL
    return g_pInstance->IsMouseLocked();
#else
    //MyAssert( false ); // TODO: fix me on everything else...
    return false;
#endif
}
