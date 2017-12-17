//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#import "CommonHeader.h"

#import "ViewController.h"
#import "AppController.h"
#import "AppDelegate.h"
#import <GameKit/GameKit.h>

extern bool IOSFUNC_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);
// drop this in main.mm for all projects.
//bool IOSFUNC_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation)
//{
//    if( interfaceOrientation == UIInterfaceOrientationLandscapeLeft || 
//       interfaceOrientation == UIInterfaceOrientationLandscapeRight )
//        return true;
//    
//    return false;
//}

@interface ViewController ()
{
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;
@property (strong, nonatomic) GKLeaderboardViewController* leaderboardController;
@property (strong, nonatomic) GKAchievementViewController* achievementController;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

@synthesize context = _context;
@synthesize effect = _effect;
@synthesize leaderboardController = _leaderboardController;
@synthesize achievementController = _achievementController;

- (void)dealloc
{
    [_context release];
    [_effect release];
    [_leaderboardController release];
    [super dealloc];
}

- (void)showGameCenterLeaderboard:(const char*)LeaderboardID
{
    if( self.leaderboardController == nil )
    {
        LOGInfo( LOGTag, "Allocating leaderboardController" );
        
        self.leaderboardController = [[GKLeaderboardViewController alloc] init];
        if( self.leaderboardController != nil )
        {
            AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
            AppController* appController = appDelegate.appController;
                    
            self.leaderboardController.leaderboardDelegate = appController;
        }
    }

    if( self.leaderboardController != nil )
    {
        NSString* nsstr = [NSString stringWithUTF8String:LeaderboardID];
        self.leaderboardController.category = nsstr; //@"TimedModeID";
        [self presentModalViewController: self.leaderboardController animated:YES];
    }
}

- (void)showGameCenterAchievements
{
    if( self.achievementController == nil )
    {
        LOGInfo( LOGTag, "Allocating achievementController" );
        
        self.achievementController = [[GKAchievementViewController alloc] init];
        if( self.achievementController )
        {
            AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
            AppController* appController = appDelegate.appController;
        
            self.achievementController.achievementDelegate = appController;
        }
    }
    
    if( self.achievementController != nil )
    {
        [self presentModalViewController: self.achievementController animated:YES];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)viewDidUnload
{    
    [super viewDidUnload];
    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    bool ret = IOSFUNC_shouldAutorotateToInterfaceOrientation( interfaceOrientation );
    
    if( ret == true )
        return YES;
    
    return NO;

//    if( interfaceOrientation == UIInterfaceOrientationLandscapeLeft || 
//        interfaceOrientation == UIInterfaceOrientationLandscapeRight )
//        return NO;
//    
//    return YES;
//    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
//    {
//        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
//    }
//    else
//    {
//        return YES;
//    }
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];

    float scale = self.view.contentScaleFactor;

    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, self.view.bounds.size.width*scale, self.view.bounds.size.height*scale );
    g_pGameCore->OneTimeInit();
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];

    g_pGameCore->OnSurfaceLost();
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    float scale = self.view.contentScaleFactor;
    g_pGameCore->OnSurfaceChanged( 0, 0, self.view.bounds.size.width*scale, self.view.bounds.size.height*scale );

    double timepassed = self.timeSinceLastUpdate;

    g_pGameCore->OnDrawFrameStart( 0 );
    g_UnpausedTime += g_pGameCore->Tick( timepassed );
    
    if( g_pGameCore->IsReadyToRender() )
    {
        AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
        
        [appDelegate killSplashScreen];
        
        //[self showLeaderboard];
    }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    if( g_pGameCore->IsReadyToRender() )
    {
        g_pGameCore->OnDrawFrameStart( 0 );
        g_pGameCore->OnDrawFrame( 0 );
        g_pGameCore->OnDrawFrameDone();
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //Get all the touches.
    NSSet* allTouches = event.allTouches;

    float scale = self.view.contentScaleFactor;
    
    for( int i=0; i<allTouches.count; i++ )
    {
        //UITouch* touch = allTouches.
        UITouch* touch = [[allTouches allObjects] objectAtIndex:i];

        CGPoint location = [touch locationInView:touch.view];

        g_pGameCore->OnTouch( GCBA_Down, i, location.x*scale, location.y*scale, 0, 0 );
        
        //LOGInfo( LOGTag, "touchesBegan %d, loc(%0.1f, %0.1f)\n", i, location.x, location.y );
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    //Get all the touches.
    NSSet* allTouches = event.allTouches;

    float scale = self.view.contentScaleFactor;

    for( int i=0; i<allTouches.count; i++ )
    {
        //UITouch* touch = allTouches.
        UITouch* touch = [[allTouches allObjects] objectAtIndex:i];
        
        CGPoint location = [touch locationInView:touch.view];
        
        g_pGameCore->OnTouch( GCBA_Up, i, location.x*scale, location.y*scale, 0, 0 );

        //LOGInfo( LOGTag, "touchesEnded %d, loc(%0.1f, %0.1f)\n", i, location.x, location.y );
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    //Get all the touches.
    NSSet* allTouches = event.allTouches;
    
    float scale = self.view.contentScaleFactor;

    for( int i=0; i<allTouches.count; i++ )
    {
        //UITouch* touch = allTouches.
        UITouch* touch = [[allTouches allObjects] objectAtIndex:i];
        
        CGPoint location = [touch locationInView:touch.view];
        
        g_pGameCore->OnTouch( GCBA_Held, i, location.x*scale, location.y*scale, 0, 0 );

        //LOGInfo( LOGTag, "touchesMoved %d, loc(%0.1f, %0.1f)\n", i, location.x, location.y );
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
}

@end

@implementation KeyboardView

- (void)insertText:(NSString *)text
{
    // Do something with the typed character
    const char* textstr = [text cStringUsingEncoding:NSASCIIStringEncoding];

    g_pGameCore->OnKeyDown( textstr[0], textstr[0] );
}

- (void)deleteBackward
{
    // Handle the delete key
    g_pGameCore->OnKeyDown( MYKEYCODE_BACKSPACE, MYKEYCODE_BACKSPACE );
}

- (BOOL)hasText
{
    // Return whether there's any text present
    return YES;
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

@end
