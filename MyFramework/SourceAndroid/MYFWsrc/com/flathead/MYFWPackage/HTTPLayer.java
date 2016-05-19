//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

package com.flathead.MYFWPackage;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;

//import android.util.Log;

public class HTTPLayer
{
    boolean m_ExpectingResponse = false;
    static StringBuilder m_SharedString = new StringBuilder(10000);
    private Thread m_Thread;
    
    public boolean IsRunning()
    {
        if( m_Thread.isAlive() )
            return true;
        
        return false;
    }

    public String GetLastResponse()
    {
        if( m_Thread.isAlive() )
        {
            //Log.v( "Flathead", "HTTPLayer::GetLastResponse - Thread isn't done." );
            return null;
        }
        
        //Log.v( "Flathead", "HTTPLayer::GetLastResponse - returning string." );
        return m_SharedString.toString();
    }

    public void RequestPage(String url)
    {
        if( m_Thread.isAlive() )
        {
            //Log.v( "Flathead", "HTTPLayer::RequestPage: An old thread is still alive " );
            return;
        }
        
        //Log.v( "Flathead", "HTTPLayer::RequestPage: " + url );
        
        m_Thread = new Thread( new Thread_HtmlLoad(url) );
        m_Thread.start();
    }
    
    private static class Thread_HtmlLoad implements Runnable
    {
        String m_URL;
        
        Thread_HtmlLoad(String url)
        {
            m_URL = url;
        }
        
        public void run()
        {
            try
            {
                URL updateURL = new URL( m_URL );
                URLConnection conn = updateURL.openConnection();
                InputStream is = conn.getInputStream();
                BufferedReader rd = new BufferedReader( new InputStreamReader(is) );

                String line = "";
                m_SharedString.delete( 0, m_SharedString.length() );
                
                while( (line = rd.readLine()) != null )
                {
                    //Log.v( "Flathead", "HTTPLayer::Thread_HtmlLoad: " + line );
                    m_SharedString.append(line); 
                }                
            }
            catch( Exception e )
            {
                //Log.v( "Flathead", "HTTPLayer::Thread_HtmlLoad: exception" );
            }
        }
    }
    

/*    private class GetURLTask extends AsyncTask<String, String, String>
    {
	    String toastMessage;
	
	    @Override protected String doInBackground(String... params)
	    {
	        //toastMessage = params[0];
	        return toastMessage;
	    }
	
	    protected void OnProgressUpdate(String... values)
	    { 
	        super.onProgressUpdate(values);
	    }
	
	    protected void onPostExecute(String result)
	    {
           //Toast toast = Toast.makeText(getApplicationContext(), result, Toast.LENGTH_SHORT);
           //toast.show();
	    }
    }
    
    public String GetLastResponse()
    {
    	Log.v( "Flathead", "HTTPLayer::GetLastResponse" );

    	if( m_ExpectingResponse == false )
    		return null;
    	
    	//if( m_Response.getStatusLine() == )
    	m_ExpectingResponse = false;
    	
        String line = "";

        m_SharedString.delete( 0, m_SharedString.length() );

        // Wrap a BufferedReader around the InputStream
        try
        {
        	Log.v( "Flathead", "HTTPLayer::GetLastResponse: before getContent" );

        	InputStream is = m_Response.getEntity().getContent();
	        BufferedReader rd = new BufferedReader( new InputStreamReader(is) );
	
	        // Read response until the end
	        while( (line = rd.readLine()) != null )
	        {
	        	Log.v( "Flathead", "HTTPLayer::GetLastResponse: " + line );
	        	m_SharedString.append(line); 
	        }
        }
        catch( ClientProtocolException e )
        {
        	return null;
        }
        catch( IOException e )
        {
        	return null;
        }
        
        return m_SharedString.toString();
    }

    public void RequestPage(String string)
    {
    	m_RequestPageThread.start();
    	
    	// Create a new HttpClient and Post Header
        HttpClient httpclient = new DefaultHttpClient();
        HttpPost httppost = new HttpPost( string ); //"http://www.flatheadgames.com/index.php" );

        try
        {
            // Add your data
            //List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(2);
            //nameValuePairs.add( new BasicNameValuePair("id", "12345") );
            //nameValuePairs.add( new BasicNameValuePair("stringdata", "AndDev is Cool!") );
            //httppost.setEntity( new UrlEncodedFormEntity(nameValuePairs) );

        	Log.v( "Flathead", "HTTPLayer::RequestPage: " + string );
            // Execute HTTP Post Request
            m_Response = httpclient.execute( httppost );
        	Log.v( "Flathead", "HTTPLayer::RequestPage: executed" );
            m_ExpectingResponse = true;
        }
        catch( ClientProtocolException e )
        {
        	m_ExpectingResponse = false;
        }
        catch( IOException e )
        {
        	m_ExpectingResponse = false;
        }
    }
    
    private static class Thread_HtmlLoad implements Runnable
    {
    	public void run()
    	{
    		try
    		{
                URL updateURL = new URL("http://iconic.4feets.com/update");
                URLConnection conn = updateURL.openConnection();
                InputStream is = conn.getInputStream();
                BufferedReader rd = new BufferedReader( new InputStreamReader(is) );

                String line = "";
                m_SharedString.delete( 0, m_SharedString.length() );
                
                // Read response until the end
                while( (line = rd.readLine()) != null )
                {
                    Log.v( "Flathead", "HTTPLayer::GetLastResponse: " + line );
                    m_SharedString.append(line); 
                }                
                
                BufferedInputStream bis = new BufferedInputStream(is);
                //ByteArrayBuffer baf = new ByteArrayBuffer(50);

                int current = 0;
                m_SharedString.delete( 0, m_SharedString.length() );
                
                while( (current = bis.read()) != -1 )
                {
                    m_SharedString.append( current ); 
                    //baf.append( (byte)current );
                }

                 Convert the Bytes read to a String. 
                //html = new String(baf.toByteArray());
                //m_Handler.post(showUpdate);
            }
    		catch( Exception e )
    		{
    		}
    	}
    }
    
    private Thread m_RequestPageThread = new Thread()
    {
        public void run()
        {
            try
            {

            }
            catch (Exception e)
            {
            }
        }
        
        private Runnable showUpdate = new Runnable()
        {
            public void run()
            {
                //Toast.makeText(Iconic.this, "HTML Code: " + html, Toast.LENGTH_SHORT).show();
            }
        };
    };*/
}
