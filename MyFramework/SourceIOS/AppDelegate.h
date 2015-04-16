//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

@class ViewController;
@class AppController;
@class KeyboardView;

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) ViewController *viewController;
@property (strong, nonatomic) AppController *appController;
@property (strong, nonatomic) UIImageView *splashScreen;
@property (strong, nonatomic) AVAudioPlayer* avAudioPlayer;
@property (strong, nonatomic) KeyboardView* keyboardView;

- (void)killSplashScreen;
- (void)ShowKeyboard:(bool)show;

@end
