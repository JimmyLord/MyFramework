//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

package com.flathead.MYFWPackage;

import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
import android.media.SoundPool;
import android.media.AudioManager;
//import android.util.Log;

public class SoundPlayer
{
    public AssetManager m_AssetManager;

    private SoundPool m_SoundPool;

    //http://developer.android.com/reference/android/media/SoundPool.html#SoundPool%28int,%20int,%20int%29

    public SoundPlayer()
    {
        // 4 streams, music is what the docs suggest, sample-rate converter quality is 0 for default(no effect ATM).
        m_SoundPool = new SoundPool( 4, AudioManager.STREAM_MUSIC, 0 );
    }

    public int LoadSound(String path)
    {
        //Log.v( "Flathead", "loadSound: " + path);

        try
        {
            AssetFileDescriptor fd = m_AssetManager.openFd( path );

            int id = m_SoundPool.load( fd, 1 );

            return id;
        }
        catch(Exception exc)
        {
            //System.out.println( "Error in loadSound - " + exc.toString() );
        }

        return -1;
    }

    public void Shutdown()
    {
        m_SoundPool.release();
        m_SoundPool = null;
    }

    public void PlaySound(int soundid, int loop)
    {
        float volume = 1.0f;

        //play(int soundID, float leftVolume, float rightVolume, int priority, int loop, float rate)
        m_SoundPool.play( soundid, volume, volume, 1, loop, 1.0f );
    }

    public void StopSound(int soundid)
    {
        m_SoundPool.stop( soundid );
    }

    public void PauseSound(int soundid)
    {
        m_SoundPool.pause( soundid );
    }

    public void ResumeSound(int soundid)
    {
        m_SoundPool.resume( soundid );
    }

    public void PauseAll()
    {
        m_SoundPool.autoPause();
    }

    public void ResumeAll()
    {
        m_SoundPool.autoResume();
    }
}
