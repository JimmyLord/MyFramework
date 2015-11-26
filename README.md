MyFramework
===========
A cross-platform C++/OpenGL framework for games

Currently supports the following platforms:
- Windows
- Android 2.2+
- iOS 5+
- BlackBerry 10/Playbook
- Native Client (NaCl)
- Emscripten
- Windows phone 8 (not well maintained)

The following is a list of tools used, which version I last tested with and possibly some setup commands I used:

Windows
- Visual Studio 2010
    
Android
- vs-android (http://code.google.com/p/vs-android/) to compile c++ code in Visual Studio 2010
    - vs-android-0.964.zip
- jdk-7u76-windows-x64.exe
- android-studio-bundle-141.2422023-windows.exe
- android-ndk-r10e-windows-x86_64.exe
- apache-ant-1.9.3-bin.zip
- setx ANDROID_NDK_ROOT E:\Apps\android-ndk-r10e
- setx ANDROID_HOME E:\Apps\Android\sdk <- to allow vs-android can build apk
- setx ANT_HOME E:\Apps\apache-ant-1.9.3 <- to allow vs-android can build apk
- mklink /d C:\Users\Me\.android E:\AppData\.android

BlackBerry 10/Playbook
- Momentics SDK 2.1 (BB10)
- BlackBerry Native SDK 2.1 (Playbook)
    
iOS
- Xcode 6.1
    
WxWidgets (debug tools on Windows build)
- wxMSW-3.0.0_vc100_Dev.7z
- wxMSW-3.0.0_vc100_ReleaseDLL.7z
- wxWidgets-3.0.0.7z <- for the include dir, there might be a sep header only download.
- Unzip all to E:\Apps\wxWidgets-3.0.0
- setx wxwin E:\Apps\wxWidgets-3.0.0
    
Emscripten
- emsdk-1.16.0-web-64bit
    - https://github.com/kripken/emscripten
    
NaCl
- tested with pepper_35 and vs_addin
    - https://developer.chrome.com/native-client/sdk/download
