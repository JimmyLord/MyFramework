//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

package com.flathead.MYFWPackage;

import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

//import com.mopub.mobileads.MoPubView;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.inputmethod.InputMethodManager;
import android.util.Log;

import android.content.res.AssetManager;
import android.content.res.Configuration;

//import com.google.ads.AdRequest;
//import com.google.ads.AdSize;
//import com.google.ads.AdView;

import android.widget.LinearLayout;

//// JNI Reference
// V void
// Z boolean
// B byte
// C char
// S short
// I int
// J long
// F float
// D double
// L fully-qualified-class ; fully-qualified-class
// [ type type[]
// ( arg-types ) ret-type method type
//
// For example, the Java method:
// long f (int n, String s, int[] arr);
// has the following type signature:
// (ILjava/lang/String;[I)J

public class MYFWActivity extends Activity
{
    private LinearLayout m_MainLayout;
    // private LinearLayout m_AdViewParent;

    private GLSurfaceView m_GLView;
    // private MoPubView m_MoPubView;

    protected int m_MainViewResourceID;
    protected int m_MainLayoutResourceID;

    private AudioManager m_AudioManager = null;
    private AssetManager m_AssetManager = null;
    private BMPFactoryLoader m_BMPFactoryLoader = null;
    private SoundPlayer m_SoundPlayer = null;
    private IAPManager m_IAPManager = null;

    private Boolean m_ShowAds = false;

    public AudioManager GetAudioManager() { return m_AudioManager; }
    public AssetManager GetAssetManager() { return m_AssetManager; }
    public BMPFactoryLoader GetBMPFactoryLoader() { return m_BMPFactoryLoader; }
    public SoundPlayer GetSoundPlayer() { return m_SoundPlayer; }
    public IAPManager GetIAPManager() { return m_IAPManager; }

    public void SetRenderMode(boolean continuous) // (Z)V
    {
        if( continuous )
            m_GLView.setRenderMode( GLSurfaceView.RENDERMODE_CONTINUOUSLY );
        else
            m_GLView.setRenderMode( GLSurfaceView.RENDERMODE_WHEN_DIRTY );
    }

    public File GetFilesDir() // ()Ljava/io/File;
    {
        return getFilesDir();
    }

    public void LaunchURL(String url) // (Ljava/lang/String;)V
    {
        Intent i = new Intent( Intent.ACTION_VIEW );
        i.setData( Uri.parse( url ) );
        startActivity( i );
    }

    public void SetMusicVolume(int volume) // (I)V
    {
        Log.v( "Flathead - Java", "m_MediaPlayer.setVolume " + volume );

        Log.v( "Flathead - Java", "m_AudioManager.isMusicActive() " + m_AudioManager.isMusicActive() );

//        // if the user is fiddling with the volume and no music(ours or system) is playing, start it up.
//        if( m_AudioManager.isMusicActive() == false && m_StartedOurMusic == false )
//        {
//            Log.v( "Flathead - Java", "MediaPlayer.create" );
//            m_MediaPlayer = MediaPlayer.create( this, R.raw.wordsinbedremixmusic_lowquality );
//            m_MediaPlayer.setLooping( true );
//            m_MediaPlayer.setVolume( volume, volume );
//            m_MediaPlayer.start();
//            m_StartedOurMusic = true;
//            m_MediaPlayer.seekTo( m_CurrentMusicPosition );
//        }
//
//        if( m_StartedOurMusic )
//        {
//            m_MediaPlayer.setVolume( volume, volume );
//        }
    }

    public void ShareString(String subject, String body) // (Ljava/lang/String;Ljava/lang/String;)V
    {
        Intent sendIntent = new Intent( Intent.ACTION_SEND );
        sendIntent.putExtra( Intent.EXTRA_SUBJECT, subject );
        sendIntent.putExtra( Intent.EXTRA_TEXT, body );
        // sendIntent.setType( "application/twitter" );
        sendIntent.setType( "text/plain" );
        startActivity( Intent.createChooser( sendIntent, null ) );
    }

    public String GetDeviceName()
    {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        if( model.startsWith( manufacturer ) )
        {
            return model;
        }
        else
        {
            return manufacturer + " " + model;
        }
    }

    boolean m_KeyboardShowing = false;

    public void ShowKeyboard(boolean show) // (Z)V
    {
        //Log.v("Flathead", "Java - ShowKeyboard " + show);
        InputMethodManager inputMgr = (InputMethodManager) getSystemService( Context.INPUT_METHOD_SERVICE );

        if( show )
        {
            inputMgr.showSoftInput( getWindow().getDecorView(), 0 );
        }
        else
        {
            inputMgr.hideSoftInputFromWindow( getWindow().getDecorView().getWindowToken(), 0 );
        }

        m_KeyboardShowing = show;
    }

    @Override protected void onCreate(Bundle savedInstanceState)
    {
        Log.v( "Flathead", "[Flow] Java - onCreate" );

        Global.m_Activity = this;

        m_AudioManager = (AudioManager)getSystemService( Context.AUDIO_SERVICE );
        m_AssetManager = getAssets();
        m_BMPFactoryLoader = new BMPFactoryLoader();
        m_BMPFactoryLoader.m_AssetManager = m_AssetManager;

        Log.v( "Flathead", "[Flow] Java - m_AudioManager/m_AssetManager initialized and bmpfactory created" );

        m_SoundPlayer = new SoundPlayer();
        m_SoundPlayer.m_AssetManager = m_AssetManager;

        Log.v( "Flathead", "[Flow] Java - m_SoundPlayer created" );

        super.onCreate( savedInstanceState );

        Log.v( "Flathead", "Java - before setContentView" );
        setContentView( m_MainViewResourceID );//R.layout.main );
        Log.v( "Flathead", "Java - after setContentView" );

        // Lookup your LinearLayout assuming it's been given
        // the attribute android:id="@+id/mainLayout"
        Log.v( "Flathead", "Java - findViewById -> " + m_MainLayoutResourceID );
        m_MainLayout = (LinearLayout) findViewById( m_MainLayoutResourceID );//R.id.mainLayout );
        //Log.v("Flathead", "Java - Main Layout: " + m_MainLayout);

        // m_AdViewParent = (LinearLayout) findViewById(R.id.adViewParent);
        // Log.v("Flathead", "Java - Main AdViewParent: " + m_MainLayout);

        Log.v( "Flathead", "Java - m_MainLayout setup" );

        if( m_ShowAds )
        {
            // Create a mopub view which will be added to the AdParentView found
            // above.
            // m_MoPubView = new MoPubView( this ); //(MoPubView)findViewById(
            // R.id.adview );
            // m_AdViewParent.addView( m_MoPubView );
            // m_MoPubView.setAdUnitId( "agltb3B1Yi1pbmNyDQsSBFNpdGUY4p7-Egw" );
            // // Enter your Ad Unit ID from www.mopub.com
            // m_MoPubView.loadAd();
        }
        else
        {
            // m_MainLayout.removeView(m_AdViewParent);
        }

        Log.v( "Flathead", "Java - Creating MyGL2SurfaceView" );
        m_GLView = new MyGL2SurfaceView( this );

        m_MainLayout.addView( m_GLView );

        NativeOnCreate( Global.m_Activity, Global.m_Activity.GetAssetManager(),
                        Global.m_Activity.GetBMPFactoryLoader(),
                        Global.m_Activity.GetSoundPlayer(),
                        GetDeviceName() );

        m_IAPManager = new IAPManager( this );
        m_IAPManager.GetPurchasesAsync();
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if( m_IAPManager.OnResult( requestCode, resultCode, data ) )
            return;
    }

    @Override public void onUserInteraction()
    {
        super.onUserInteraction();

        //Log.v( "Flathead", "[Flow] Java - onUserInteraction" );

        m_GLView.setRenderMode( GLSurfaceView.RENDERMODE_CONTINUOUSLY );

        if( m_ShowAds )
        {
            // if( m_AdViewParent.getChildCount() == 0 )
            // m_AdViewParent.addView( m_MoPubView );
        }
    }

    @Override public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        //Log.v( "Flathead", "[Flow] Java - onKeyDown" );

        m_GLView.setRenderMode( GLSurfaceView.RENDERMODE_CONTINUOUSLY );

        switch( keyCode )
        {
        case KeyEvent.KEYCODE_BACK:
            if( event.isAltPressed() == false ) // for Xperia Play(alt-back is the 'o' key)
            {
                Log.v( "Flathead", "[Flow] Java - onBackPressed" );
                NativeOnBackPressed( Global.m_Activity,
                                     Global.m_Activity.GetAssetManager(),
                                     Global.m_Activity.GetBMPFactoryLoader(),
                                     Global.m_Activity.GetSoundPlayer() );
            }
            return true;

        case KeyEvent.KEYCODE_VOLUME_UP:
            m_AudioManager.adjustStreamVolume( AudioManager.STREAM_MUSIC,
                                               AudioManager.ADJUST_RAISE, AudioManager.FLAG_SHOW_UI );
            return true;

        case KeyEvent.KEYCODE_VOLUME_DOWN:
            m_AudioManager.adjustStreamVolume( AudioManager.STREAM_MUSIC,
                                               AudioManager.ADJUST_LOWER, AudioManager.FLAG_SHOW_UI );
            return true;

        default:
            {
                NativeOnKeyDown( event.getKeyCode(), event.getUnicodeChar(),
                                 Global.m_Activity,
                                 Global.m_Activity.GetAssetManager(),
                                 Global.m_Activity.GetBMPFactoryLoader(),
                                 Global.m_Activity.GetSoundPlayer() );
                return true;
            }
        }

        // return super.onKeyDown(keyCode, event);
    }

    @Override public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        switch( keyCode )
        {
        default:
            {
                NativeOnKeyUp( event.getKeyCode(), event.getUnicodeChar(),
                               Global.m_Activity,
                               Global.m_Activity.GetAssetManager(),
                               Global.m_Activity.GetBMPFactoryLoader(),
                               Global.m_Activity.GetSoundPlayer() );
                return true;
            }
        }

        // return super.onKeyUp(keyCode, event);
    }

    @Override public boolean onKeyMultiple(int keyCode, int count, KeyEvent event)
    {
        Log.v( "Flathead", "onKeyMultiple " + count + " " + keyCode );

        if( keyCode == KeyEvent.KEYCODE_UNKNOWN )
        {
            String string = event.getCharacters();
            Log.v( "Flathead", "onKeyMultiple " + string );

            byte[] chars = string.getBytes();

            for( int i = 0; i < chars.length; i++ )
            {
                Log.v( "Flathead", "onKeyMultiple " + chars[i] );
                NativeOnKeyDown( chars[i], chars[i],
                                 Global.m_Activity,
                                 Global.m_Activity.GetAssetManager(),
                                 Global.m_Activity.GetBMPFactoryLoader(),
                                 Global.m_Activity.GetSoundPlayer() );
                NativeOnKeyUp( chars[i], chars[i],
                               Global.m_Activity,
                               Global.m_Activity.GetAssetManager(),
                               Global.m_Activity.GetBMPFactoryLoader(),
                               Global.m_Activity.GetSoundPlayer() );
            }
            return true;
        }

        return false;
    }

    @Override public void onConfigurationChanged(Configuration newConfig)
    {
        m_GLView.setRenderMode( GLSurfaceView.RENDERMODE_CONTINUOUSLY );

        // Checks the orientation of the screen
        if( newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE )
        {
            Log.v( "Flathead", "[Flow] Java - onConfigurationChanged to landscape" );
        }
        else if( newConfig.orientation == Configuration.ORIENTATION_PORTRAIT )
        {
            Log.v( "Flathead", "[Flow] Java - onConfigurationChanged to portrait" );
        }

        super.onConfigurationChanged( newConfig );
    }

    @Override protected void onPause()
    {
        Log.v( "Flathead", "[Flow] Java - onPause" );

        if( m_ShowAds )
        {
            //m_AdViewParent.removeView( m_MoPubView );
        }

        m_GLView.onPause();

        super.onPause();
    }

    @Override protected void onResume()
    {
        Log.v( "Flathead", "[Flow] Java - onResume" );
        super.onResume();

        m_GLView.onResume();
    }

    @Override protected void onPostResume()
    {
        Log.v( "Flathead", "[Flow] Java - onPostResume" );

        super.onPostResume();

        if( m_ShowAds )
        {
            // if( m_AdViewParent.getChildCount() == 0 )
            // m_AdViewParent.addView( m_MoPubView );
        }
    }

    @Override public void finish()
    {
        Log.v( "Flathead", "[Flow] Java - finish" );

        super.finish();
    }

    @Override public void onDestroy()
    {
        Log.v( "Flathead", "[Flow] Java - onDestroy" );

        NativeOnDestroy();

        if( m_ShowAds )
        {
            // m_MoPubView.destroy();
            // //m_AdView.destroy();
        }

        super.onDestroy();

        m_IAPManager.Shutdown();
    }

    private static native void NativeOnCreate(Object activity, Object assetmgr,
                                              Object bmploader, Object sndplayer, String devicename);

    private static native void NativeOnDestroy();

    private static native void NativeOnBackPressed(Object activity,
                                                   Object assetmgr, Object bmploader, Object sndplayer);

    private static native void NativeOnKeyDown(int keycode, int unicodechar, Object activity,
                                               Object assetmgr, Object bmploader, Object sndplayer);

    private static native void NativeOnKeyUp(int keycode, int unicodechar, Object activity,
                                             Object assetmgr, Object bmploader, Object sndplayer);
}

class MyGL2SurfaceView extends GLSurfaceView
{
    MyGL2Renderer m_Renderer;

    public MyGL2SurfaceView(Context context)
    {
        super( context );

        //Log.v( "Flathead", "MyGL2SurfaceView constructor" );

        // Create an OpenGL ES 2.0 context.
        setEGLContextClientVersion( 2 );
        if( Build.VERSION.SDK_INT >= 11 ) //Build.VERSION_CODES.HONEYCOMB // Android 3.0.x
	        setPreserveEGLContextOnPause( true );
        setEGLConfigChooser( 5, 6, 5, 0, 16, 0 ); // R,G,B,A,Depth,Stencil

        m_Renderer = new MyGL2Renderer();
        setRenderer( m_Renderer );

        setRenderMode( RENDERMODE_CONTINUOUSLY );
        //setRenderMode( RENDERMODE_WHEN_DIRTY );

        //Log.v( "Flathead", "MyGL2SurfaceView constructor end" );
    }

    @Override public boolean onTouchEvent(final MotionEvent ev)
    {
        //Log.v("Flathead", "Java - MyGL2SurfaceView - onTouchEvent");

        setRenderMode( RENDERMODE_CONTINUOUSLY );

        int action = ev.getAction();
        int actionindex = ev.getActionIndex();
        int actionmasked = ev.getActionMasked();

        int count = ev.getPointerCount();
        for( int p = 0; p < count; p++ )
        {
            int tool = 0; // ev.getToolType( p );
            int id = ev.getPointerId( p );
            float x = ev.getX( p );
            float y = ev.getY( p );
            float pressure = ev.getPressure( p );
            float size = ev.getSize( p );

            NativeOnTouchEvent( Global.m_Activity,
                                Global.m_Activity.GetAssetManager(),
                                Global.m_Activity.GetBMPFactoryLoader(),
                                Global.m_Activity.GetSoundPlayer(), action, actionindex,
                                actionmasked, tool, id, x, y, pressure, size );
        }
        return true;
    }

    @Override public void onPause()
    {
        Log.v( "Flathead", "Java - MyGL2SurfaceView - onPause" );
        super.onPause();
    }

    @Override public void onResume()
    {
        Log.v( "Flathead", "Java - MyGL2SurfaceView - onResume" );

        setRenderMode( RENDERMODE_CONTINUOUSLY );

        super.onResume();
    }

    @Override public void surfaceCreated(SurfaceHolder holder)
    {
        Log.v( "Flathead", "[Flow] Java - MyGL2SurfaceView - surfaceCreated()" );

        super.surfaceCreated( holder );
        NativeOnSurfaceCreated();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        Log.v( "Flathead", "[Flow] Java - MyGL2SurfaceView - surfaceChanged()" );

        setRenderMode( RENDERMODE_CONTINUOUSLY );

        super.surfaceChanged( holder, format, w, h );
        NativeOnSurfaceChanged( w, h );
    }

    @Override public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.v( "Flathead", "[Flow] Java - MyGL2SurfaceView - surfaceDestroyed()" );

        super.surfaceDestroyed( holder );
        NativeOnSurfaceDestroyed();
    }

    private static native void NativeOnSurfaceCreated();
    private static native void NativeOnSurfaceChanged(int w, int h);
    private static native void NativeOnSurfaceDestroyed();

    private static native void NativeOnTouchEvent(Object activity,
                                                  Object assetmgr, Object bmploader, Object sndplayer, int action,
                                                  int actionindex, int actionmasked, int tool, int id, float x,
                                                  float y, float pressure, float size);
}

class MyGL2Renderer implements GLSurfaceView.Renderer
{
    public long startTime;

    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        //Log.v( "Flathead", "Java - MyGL2Renderer - onSurfaceCreated" );
    }

    public void onSurfaceChanged(GL10 gl, int w, int h)
    {
        //Log.v( "Flathead", "Java - MyGL2Renderer - onSurfaceChanged" );
    }

    public void onDrawFrame(GL10 gl)
    {
        long endTime = System.currentTimeMillis();
        long dt = endTime - startTime;
        if( dt < 1000 / 30 )
        {
            try
            {
                Thread.sleep( 1000 / 30 - dt );
            }
            catch( InterruptedException e )
            {
                //e.printStackTrace();
            }
        }
        startTime = System.currentTimeMillis();

        //Log.v( "Flathead", "[Flow] Java - MyGL2Renderer - onDrawFrame" );
        NativeRender( Global.m_Activity, Global.m_Activity.GetAssetManager(),
                      Global.m_Activity.GetBMPFactoryLoader(),
                      Global.m_Activity.GetSoundPlayer() );
    }

    private static native void NativeRender(Object activity, Object assetmgr, Object bmploader, Object sndplayer);
}
