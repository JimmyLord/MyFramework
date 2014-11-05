MyFramework - a cross-platform C++/OpenGL framework for games
=============================================================

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
        - vs-android-0.951-cateia-mod-v3.rar
    - jdk-6u45 <- 32 bit version
    - adt-bundle-windows <- 20131030, 32 bit version
    - android-ndk <- used an older version for vs-android compatibility
        - android-ndk-r8e-windows-x86.zip <- 32 bit version
    - apache-ant-1.9.3
    - setx ANDROID_NDK_ROOT E:\Apps\android-ndk-r8e
    - setx ANDROID_HOME E:\Apps\adt-bundle-windows-x86-20131030\sdk
    - setx ANT_HOME E:\Apps\apache-ant-1.9.3
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
