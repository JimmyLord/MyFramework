MyFramework
===========
A cross-platform C++/OpenGL framework for games

Currently supports the following platforms:
- Windows
- Linux (WIP)
- Android 2.2+
- iOS 5+
- BlackBerry 10/Playbook
- Native Client (NaCl)
- Emscripten
- Windows phone 8 (not maintained for years)

The following is a list of tools used, which version I last tested with and possibly some setup commands I used:

Windows
- Visual Studio 2010+

Linux
- Debian
    - build-essential
    - cmake
    - libboost-all-dev (for SpriteTool)
    - mesa-common-dev
    - libopenal-dev
    - libgl1-mesa-dev
    - libgtk-3-dev (for wxWidgets)
    - libglu1-mesa-dev (for wxWidgets)
    - gdb
- IDE
    - vscode
    - xterm (for VSCode debugging)
    
Android
- vs-android (http://code.google.com/p/vs-android/) to compile c++ code in Visual Studio 2010
    - vs-android-0.964.zip
- jdk-7u76-windows-x64.exe
- android-studio-bundle-141.2422023-windows.exe
- android-ndk-r10e-windows-x86_64.exe
- apache-ant-1.9.3-bin.zip
- setx ANDROID_NDK_ROOT E:\Apps\android-ndk-r10e
- setx ANDROID_HOME E:\Apps\Android\sdk <- to allow vs-android to build apk
- setx ANT_HOME E:\Apps\apache-ant-1.9.3 <- to allow vs-android to build apk
- mklink /d C:\Users\Me\.android E:\AppData\.android

BlackBerry 10/Playbook
- Momentics SDK 2.1 (BB10)
- BlackBerry Native SDK 2.1 (Playbook)
    
iOS
- Xcode 6.1
    
Emscripten
- emsdk-1.16.0-web-64bit
    - https://github.com/kripken/emscripten
    
NaCl
- tested with pepper_49 and vs_addin
    - https://developer.chrome.com/native-client/sdk/download
- setx NACL_SDK_ROOT E:\Apps\nacl_sdk\pepper_49
