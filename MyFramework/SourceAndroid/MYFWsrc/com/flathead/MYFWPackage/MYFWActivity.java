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

package com.flathead.MYFWPackage;

import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

//import com.mopub.mobileads.MoPubView;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.util.Log;

import android.content.res.AssetManager;
import android.content.res.Configuration;

//import com.google.ads.AdRequest;
//import com.google.ads.AdSize;
//import com.google.ads.AdView;

//import android.widget.EditText;
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

// NOTE TO SELF: I fixed all brackets/spacing in this file.. if it reverts, find way to turn it off.

public class MYFWActivity extends Activity
{
	private LinearLayout m_MainLayout;
	// private LinearLayout m_AdViewParent;

	private GLSurfaceView m_GLView;
	// private MoPubView m_MoPubView;
    //private EditText m_EditText;
    //private Boolean h_ShowEditText = false;
    //private Boolean h_HideEditText = false;

	protected int m_MainViewResourceID;
	protected int m_MainLayoutResourceID;
	
	private AudioManager m_AudioManager;
	private AssetManager m_AssetManager;
	private BMPFactoryLoader m_BMPFactoryLoader;
	private SoundPlayer m_SoundPlayer;

	private Boolean m_ExitOnBackButton;

	private Boolean m_ShowAds = false;

	public AssetManager GetAssetManager()
	{
		return m_AssetManager;
	}

	public BMPFactoryLoader GetBMPFactoryLoader()
	{
		return m_BMPFactoryLoader;
	}

	public SoundPlayer GetSoundPlayer()
	{
		return m_SoundPlayer;
	}

	public File GetFilesDir() // ()Ljava/io/File;
	{
		return getFilesDir();
	}

	public void LaunchURL(String url) // (Ljava/lang/String;)V
	{
		Intent i = new Intent(Intent.ACTION_VIEW);
		i.setData(Uri.parse(url));
		startActivity(i);
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
		Intent sendIntent = new Intent(Intent.ACTION_SEND);
		sendIntent.putExtra(Intent.EXTRA_SUBJECT, subject);
		sendIntent.putExtra(Intent.EXTRA_TEXT, body);
		// sendIntent.setType( "application/twitter" );
		sendIntent.setType("text/plain");
		startActivity(Intent.createChooser(sendIntent, null));
	}
	
	public String GetDeviceName()
	{
		String manufacturer = Build.MANUFACTURER;
		String model = Build.MODEL;
		if( model.startsWith(manufacturer) )
		{
			return model;
		}
		else
		{
			return manufacturer + " " + model;
		}
	}	

	public void SetExitOnBackButton(boolean exit) // (Z)V
	{
		m_ExitOnBackButton = exit;
	}

	boolean m_KeyboardShowing = false;

	public void ShowKeyboard(boolean show) // (Z)V
	{
		//Log.v("Flathead", "Java - ShowKeyboard " + show);
		InputMethodManager inputMgr = (InputMethodManager)getSystemService( Context.INPUT_METHOD_SERVICE );

		if( show )
		{
			inputMgr.showSoftInput( getWindow().getDecorView(), 0 );

            //h_ShowEditText = true;
	        //m_EditText.setVisibility( View.VISIBLE );
		}
		else
		{
			inputMgr.hideSoftInputFromWindow( getWindow().getDecorView().getWindowToken(), 0);
			
            //h_HideEditText = true;
	        //m_EditText.setVisibility( View.INVISIBLE );
			//m_MainLayout.removeViewAt( 1 );
		}

		m_KeyboardShowing = show;

		// if( show )
		// {
		// if( m_KeyboardShowing == true )
		// {
		// //inputMgr.showSoftInput( m_MainLayout, 0 );
		// Configuration config = this.getResources().getConfiguration();
		// if( config.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_YES
		// ||
		// config.keyboard != Configuration.KEYBOARD_QWERTY )
		// {
		// inputMgr.toggleSoftInput( InputMethodManager.SHOW_FORCED, 0 );
		// //SHOW_IMPLICIT, 0 );
		// m_KeyboardShowing = true;
		// }
		// }
		// }
		// else
		// {
		// if( m_KeyboardShowing == true )
		// {
		// inputMgr.toggleSoftInput( InputMethodManager.SHOW_FORCED, 0 );
		// //SHOW_IMPLICIT, 0 );
		// m_KeyboardShowing = false;
		// }
		// }
	}
	
	@Override protected void onCreate(Bundle savedInstanceState)
    {
		Log.v("Flathead", "Java - [Flow] - onCreate");

		Global.m_Activity = this;

		m_AudioManager = (AudioManager)getSystemService( Context.AUDIO_SERVICE );
		m_AssetManager = getAssets();
		m_BMPFactoryLoader = new BMPFactoryLoader();
		m_BMPFactoryLoader.m_AssetManager = m_AssetManager;

		m_ExitOnBackButton = true;

		m_SoundPlayer = new SoundPlayer();
		m_SoundPlayer.m_AssetManager = m_AssetManager;

		super.onCreate(savedInstanceState);

		setContentView( m_MainViewResourceID );//R.layout.main );

		// Lookup your LinearLayout assuming it's been given
		// the attribute android:id="@+id/mainLayout"
		m_MainLayout = (LinearLayout)findViewById( m_MainLayoutResourceID );//R.id.mainLayout );
		//Log.v("Flathead", "Java - Main Layout: " + m_MainLayout);

		// m_AdViewParent = (LinearLayout) findViewById(R.id.adViewParent);
		// Log.v("Flathead", "Java - Main AdViewParent: " + m_MainLayout);

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
		
        //m_EditText = new EditText( this );
        //m_EditText.setText( "Testing EditText" );

        ////m_MainLayout.addView( m_EditText );

        //ViewGroup.LayoutParams params;
        //params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        //addContentView( m_EditText, params );
        //m_EditText.setVisibility( View.INVISIBLE );

		NativeOnCreate(Global.m_Activity, Global.m_Activity.GetAssetManager(),
				Global.m_Activity.GetBMPFactoryLoader(),
				Global.m_Activity.GetSoundPlayer(),
				GetDeviceName() );
	}

	@Override public void onUserInteraction()
	{
		super.onUserInteraction();

		if( m_ShowAds )
		{
			// if( m_AdViewParent.getChildCount() == 0 )
			// m_AdViewParent.addView( m_MoPubView );
		}

//        if( h_ShowEditText )
//        {
////	        m_EditText.setVisibility( View.VISIBLE );
//            h_ShowEditText = false;
//        }
//        if( h_HideEditText )
//        {
////	        m_EditText.setVisibility( View.INVISIBLE );
//            h_HideEditText = false;
//        }
	}

	// @Override public void onBackPressed()
	public void MyOnBackPressed()
    {
		Log.v("Flathead", "Java - [Flow] - onBackPressed");

		// if( m_KeyboardShowing == true )
		// m_KeyboardShowing = false;

		if( m_ExitOnBackButton )
        {
			finish();
			// super.onBackPressed(); // calling this will kill the app...
			Log.v("Flathead", "Java - [Flow] - attempting to kill app");
		}
        else
        {
			NativeOnBackPressed(Global.m_Activity,
					Global.m_Activity.GetAssetManager(),
					Global.m_Activity.GetBMPFactoryLoader(),
					Global.m_Activity.GetSoundPlayer());
		}
	}

	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
    {
		switch( keyCode )
        {
		    case KeyEvent.KEYCODE_BACK:
		    	if( event.isAltPressed() == false ) // for Xperia Play(alt-back is the 'o' key)
		    		MyOnBackPressed();
			    return true;

		    case KeyEvent.KEYCODE_VOLUME_UP:
			    m_AudioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC,
					    AudioManager.ADJUST_RAISE, AudioManager.FLAG_SHOW_UI);
			    return true;

		    case KeyEvent.KEYCODE_VOLUME_DOWN:
			    m_AudioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC,
					    AudioManager.ADJUST_LOWER, AudioManager.FLAG_SHOW_UI);
			    return true;

		    default:
		    // if( m_KeyboardShowing )
		    {
			    // Log.v( "keydown",
			    // Character.toString(Character.toChars(event.getUnicodeChar())[0])
			    // );
			    NativeOnKeyDown(event.getKeyCode(), event.getUnicodeChar(),
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
		    // if( m_KeyboardShowing )
		    {
			    // Log.v( "keyup",
			    // Character.toString(Character.toChars(event.getUnicodeChar())[0])
			    // );
			    NativeOnKeyUp(event.getKeyCode(), event.getUnicodeChar(),
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
        	
        	for( int i=0; i<chars.length; i++ )
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
		// Checks the orientation of the screen
		if( newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE )
		{
			Log.v("Flathead", "Java - [Flow] - onConfigurationChanged to landscape");
		}
		else if( newConfig.orientation == Configuration.ORIENTATION_PORTRAIT )
		{
			Log.v("Flathead", "Java - [Flow] - onConfigurationChanged to portrait");
		}

		super.onConfigurationChanged( newConfig );

		Log.v("Flathead", "Java - [Flow] - onConfigurationChanged post super");
	}

	@Override protected void onPause()
	{
		Log.v("Flathead", "Java - [Flow] - onPause");
		super.onPause();

		if( m_ShowAds )
		{
			// m_AdViewParent.removeView( m_MoPubView );
		}
		m_GLView.onPause();
	}

	@Override protected void onResume()
	{
		Log.v("Flathead", "Java - [Flow] - onResume");
		super.onResume();

		m_GLView.onResume();
	}

	@Override protected void onPostResume()
	{
		Log.v("Flathead", "Java - [Flow] - onPostResume");

		super.onPostResume();

		if( m_ShowAds )
		{
			// if( m_AdViewParent.getChildCount() == 0 )
			// m_AdViewParent.addView( m_MoPubView );
		}
	};

	@Override public void finish()
	{
		Log.v("Flathead", "Java - [Flow] - finish");

		super.finish();
	}

	@Override public void onDestroy()
	{
		Log.v("Flathead", "Java - [Flow] - onDestroy");

		NativeOnDestroy();

		if( m_ShowAds )
		{
			// m_MoPubView.destroy();
			// //m_AdView.destroy();
		}

		super.onDestroy();
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
		super(context);

		//Log.v( "Flathead", "MyGL2SurfaceView constructor" );

		// Create an OpenGL ES 2.0 context.
		setEGLContextClientVersion( 2 );
        setEGLConfigChooser( 5, 6, 5, 0, 16, 0 );

		m_Renderer = new MyGL2Renderer();
		setRenderer( m_Renderer );

        //Log.v( "Flathead", "MyGL2SurfaceView constructor end" );
	}
	
//    @Override public InputConnection onCreateInputConnection(EditorInfo outAttrs)
//    {
//        outAttrs.actionLabel = "";
//        outAttrs.hintText = "";
//        outAttrs.initialCapsMode = 0;
//        outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;
//        outAttrs.label = "";
//        outAttrs.imeOptions = EditorInfo.IME_ACTION_UNSPECIFIED | EditorInfo.IME_FLAG_NO_EXTRACT_UI;        
//        outAttrs.inputType = InputType.TYPE_CLASS_TEXT;        
//        
//        final InputConnection in = new BaseInputConnection(this, false)
//        {        
//            //private HudInputElement getInputElement(){...}
//        
//            @Override
//            public boolean setComposingText(CharSequence text, int newCursorPosition)
//            {
//            	Log.v( "Flathead", "Java - setComposingText " + text + " " + newCursorPosition );
//                //B2DEngine.getLogger().info("composing text: "+text+","+newCursorPosition);
//                //HudInputElement input = getInputElement();
//                //if(input!=null)
//                //{
//                //    input.setComposingText(text.toString());
//                //}
//                return super.setComposingText(text, newCursorPosition);
//            }
//
//            @Override
//            public boolean finishComposingText()
//            {
//            	Log.v( "Flathead", "Java - finishComposingText " );
////                HudInputElement input = getInputElement();
////                if(input!=null)
////                {
////                    input.doneComposing();
////                }
//                return super.finishComposingText();
//            }
//
//            @Override
//            public boolean commitText(CharSequence text, int newCursorPosition)
//            {
//            	Log.v( "Flathead", "Java - commitText " + text + " " + newCursorPosition );
////                B2DEngine.getLogger().info("commit:"+text.toString()+","+this.getEditable().toString());
////                HudInputElement input = getInputElement();
////                if(input!=null)
////                {
////                    input.doneComposing();
////                }
//                return super.commitText(text, newCursorPosition);
//            }           
//        };       
//
//        return in;
//    }
//
////    @Override public InputConnection onCreateInputConnection(EditorInfo outAttrs)
////    {
////        BaseInputConnection fic = new BaseInputConnection( this, false ); //true );
////        outAttrs.actionLabel = null;
////        outAttrs.inputType = InputType.TYPE_NULL; //TYPE_CLASS_TEXT;
////        outAttrs.imeOptions = EditorInfo.IME_ACTION_NEXT;
////        return fic;
////    }
//
//    @Override public boolean onCheckIsTextEditor()
//    {
//        return true;
//    }
//
//    public boolean onKeyPreIme(int keycode, KeyEvent event)
//    {
//        Log.v( "Flathead", "Java - onKeyPreIme " + keycode );
//        return false;
//    }

	public boolean onTouchEvent(final MotionEvent ev)
	{
		// Log.v("Flathead", "Java - MyGL2SurfaceView - onTouchEvent");

		int action = ev.getAction();
		int actionindex = ev.getActionIndex();
		int actionmasked = ev.getActionMasked();

		int count = ev.getPointerCount();
		for( int p = 0; p < count; p++ )
		{
			int tool = 0; // ev.getToolType( p );
			int id = ev.getPointerId(p);
			float x = ev.getX(p);
			float y = ev.getY(p);
			float pressure = ev.getPressure(p);
			float size = ev.getSize(p);

			NativeOnTouchEvent(Global.m_Activity,
					Global.m_Activity.GetAssetManager(),
					Global.m_Activity.GetBMPFactoryLoader(),
					Global.m_Activity.GetSoundPlayer(), action, actionindex,
					actionmasked, tool, id, x, y, pressure, size);
		}
		return true;
	}

	public void onPause()
	{
		//Log.v( "Flathead", "Java - MyGL2SurfaceView - onPause" );

		super.onPause();
		NativeOnPause();
	}

	private static native void NativeOnPause();

	private static native void NativeOnTouchEvent(Object activity,
			Object assetmgr, Object bmploader, Object sndplayer, int action,
			int actionindex, int actionmasked, int tool, int id, float x,
			float y, float pressure, float size);
}

class MyGL2Renderer implements GLSurfaceView.Renderer
{
	public long startTime;// = System.currentTimeMillis();
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		//Log.v( "Flathead", "Java - MyGL2Renderer - onSurfaceCreated" );
		NativeOnSurfaceCreated(Global.m_Activity,
				Global.m_Activity.GetAssetManager(),
				Global.m_Activity.GetBMPFactoryLoader(),
				Global.m_Activity.GetSoundPlayer());
	}

	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
		//Log.v( "Flathead", "Java - MyGL2Renderer - onSurfaceChanged" );
		nativeOnSurfaceChanged(w, h, Global.m_Activity,
				Global.m_Activity.GetAssetManager(),
				Global.m_Activity.GetBMPFactoryLoader(),
				Global.m_Activity.GetSoundPlayer());
	}

	public void onDrawFrame(GL10 gl)
	{
	    long endTime = System.currentTimeMillis();
	    long dt = endTime - startTime;
	    if( dt < 1000/30 )
	    {
			try
			{
				Thread.sleep( 1000/30 - dt );
			}
			catch (InterruptedException e)
			{
				//e.printStackTrace();
			}
	    }
	    startTime = System.currentTimeMillis();

	    //Log.v( "Flathead", "Java - MyGL2Renderer - onDrawFrame" );
		nativeRender(Global.m_Activity, Global.m_Activity.GetAssetManager(),
				Global.m_Activity.GetBMPFactoryLoader(),
				Global.m_Activity.GetSoundPlayer());
	}

	private static native void NativeOnSurfaceCreated(Object activity,
			Object assetmgr, Object bmploader, Object sndplayer);

	private static native void nativeOnSurfaceChanged(int w, int h,
			Object activity, Object assetmgr, Object bmploader, Object sndplayer);

	private static native void nativeRender(Object activity, Object assetmgr,
			Object bmploader, Object sndplayer);

	private static native void nativeDone();
}
