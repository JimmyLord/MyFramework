//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#import "CommonHeader.h"

#import "AppDelegate.h"
#import <AVFoundation/AVFoundation.h>
#include <AudioToolbox/AudioServices.h>

#import "ViewController.h"
#import "AppController.h"

@implementation AppDelegate

@synthesize window = _window;
@synthesize viewController = _viewController;
@synthesize appController = _appController;
@synthesize splashScreen = _splashScreen;
@synthesize avAudioPlayer = m_avAudioPlayer;
@synthesize keyboardView = _keyboardView;

- (void)dealloc
{
    [_window release];
    [_viewController release];
    [_appController release];
    [_splashScreen release];
    [_keyboardView release];
    [super dealloc];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // init audio stuff
    {
        [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryAmbient error: nil];

        UInt32 otherAudioIsPlaying = 0;
        UInt32 propertySize = sizeof( otherAudioIsPlaying );
        AudioSessionGetProperty( kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &otherAudioIsPlaying );
    
        NSString* musicfile = [[NSBundle mainBundle] pathForResource:@"Data/Audio/WordsInBedRemixMusic_lowQuality" ofType:@"wav"];
        if( musicfile )
        {
            m_avAudioPlayer = [[AVAudioPlayer alloc]initWithContentsOfURL:[NSURL fileURLWithPath:musicfile] error:NULL];
            if( m_avAudioPlayer )
            {
                m_avAudioPlayer.numberOfLoops = -1;
                m_avAudioPlayer.volume = 1.0;
            }
        }
        
//        if( otherAudioIsPlaying == false )
//            [m_avAudioPlayer play];
    }
    
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
//    self.window = [[[UIWindow alloc] init] autorelease];
    self.appController = [[[AppController alloc] init] autorelease];

    self.viewController = [[[ViewController alloc] init] autorelease];
//    // Override point for customization after application launch.
//    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
//    {
//        self.viewController = [[[ViewController alloc] initWithNibName:@"ViewController_iPhone" bundle:nil] autorelease];
//    }
//    else
//    {
//        self.viewController = [[[ViewController alloc] initWithNibName:@"ViewController_iPad" bundle:nil] autorelease];
//    }
    
    //CGRect bounds = [[UIScreen mainScreen] bounds];
    //int boundwidth = self.viewController.view.bounds.size.width;
    //int boundheight = self.viewController.view.bounds.size.height;
    //int framewidth = self.viewController.view.frame.size.width;
    //int frameheight = self.viewController.view.frame.size.height;
    //float scalefactor = self.viewController.view.contentScaleFactor;

    self.window.rootViewController = self.viewController;

    if( self.viewController.view.bounds.size.width < 400 )       
        self.splashScreen = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]];
    else if( self.viewController.view.bounds.size.width == 640 )
        self.splashScreen = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default@2x.png"]];
    else if( self.viewController.view.bounds.size.width == 768 )
        self.splashScreen = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default-Portrait~ipad.png"]];
    else
        self.splashScreen = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default-Portrait@2x~ipad.png"]];

    [self.window.rootViewController.view addSubview:self.splashScreen];

    self.keyboardView = [[[KeyboardView alloc] init] autorelease];
    //CGRect rect = [[UIScreen mainScreen] applicationFrame];
    //self.keyboardView = [[[KeyboardView alloc] initWithFrame:CGRectMake(0,0,1000,1000)] autorelease];
    
    [self.window.rootViewController.view addSubview:self.keyboardView];
    [self.window.rootViewController.view bringSubviewToFront:self.keyboardView];

//    [self.splashScreen release];
    
    [self.window makeKeyAndVisible];
    
//    [UIView animateWithDuration:2.0 animations:^{self.splashScreen.alpha = 0.99999;}
//                                    completion:(void (^)(BOOL)) ^{
//                                            [self.splashScreen removeFromSuperview];
//                                    }
//    ];
        
    //g_pGameCore->OnSurfaceCreated();
    //g_pGameCore->OneTimeInit();
    
    return YES;
}

- (void)killSplashScreen
{
//    [self.splashScreen removeFromSuperview];
//    [UIView animateWithDuration:0.5 animations:^{self.splashScreen.alpha = 0.0;} completion:NULL];
    [UIView animateWithDuration:0.5 delay:0.5 options:UIViewAnimationOptionTransitionNone animations:^{self.splashScreen.alpha = 0.0;} completion:NULL];
}


- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    LOGInfo( LOGTag, "applicationWillResignActive\n" );
    g_pGameCore->OnPrepareToDie();
    LOGInfo( LOGTag, "called g_pGameCore->OnPrepareToDie\n" );

    //[m_avAudioPlayer stop];
    //g_pGameCore->GetSoundPlayer()->DeactivateSoundContext();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    LOGInfo( LOGTag, "applicationDidEnterBackground\n" );

    g_pGameCore->OnFocusLost();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    LOGInfo( LOGTag, "applicationWillEnterForeground\n" );
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    LOGInfo( LOGTag, "applicationDidBecomeActive\n" );

    g_pGameCore->OnFocusGained();

    if( m_avAudioPlayer )
    {
        UInt32 otherAudioIsPlaying = 0;
        UInt32 propertySize = sizeof( otherAudioIsPlaying );
        AudioSessionGetProperty( kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &otherAudioIsPlaying );
        
        if( otherAudioIsPlaying ) //== false )
            [m_avAudioPlayer stop];
    }

    //g_pGameCore->GetSoundPlayer()->ActivateSoundContext();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    LOGInfo( LOGTag, "applicationWillTerminate\n" );
    g_pGameCore->OnPrepareToDie();
}

- (void)ShowKeyboard:(bool)show
{
    if( show )
    {
        LOGInfo( LOGTag, "ShowKeyboard - show\n" );

        [self.keyboardView becomeFirstResponder];
    }
    else
    {
        LOGInfo( LOGTag, "ShowKeyboard - hide\n" );
        
        [self.keyboardView resignFirstResponder];
    }
}

@end
