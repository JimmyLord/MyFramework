//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#import <UIKit/UIApplication.h>
#import "AppDelegate.h"
#include <AudioToolbox/AudioServices.h>

void IOSLaunchURL(const char* url)
{
    NSString* nsstr = [NSString stringWithUTF8String:url];
    NSURL* tempurl = [NSURL URLWithString:nsstr]; //@"http://www.flatheadgames.com"];
    [[UIApplication sharedApplication] openURL:tempurl];
}

void IOSSetMusicVolume(float volume)
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
//    appDelegate.avAudioPlayer.volume = volume;
    
    if( volume == 0.0 )
        [appDelegate.avAudioPlayer stop];
    else
    {
        UInt32 otherAudioIsPlaying = 0;
        UInt32 propertySize = sizeof( otherAudioIsPlaying );
        AudioSessionGetProperty( kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &otherAudioIsPlaying );
        if( otherAudioIsPlaying == false )
            [appDelegate.avAudioPlayer play];
    }
}

void IOSShowKeyboard(bool show)
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];

    [appDelegate ShowKeyboard:show];    
}