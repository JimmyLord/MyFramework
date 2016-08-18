//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

package com.flathead.MYFWPackage;

import java.io.FileInputStream;
import java.io.InputStream;
import java.nio.channels.FileChannel;
import java.nio.ByteBuffer;

import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

public class BMPFactoryLoader
{
    public AssetManager m_AssetManager;

    public long GetBinaryFileSize(String path)
    {
		// read the entire file to get it's length... ugly.
		// TODO: fix this, maybe load into a buffer and see if we can pass the buffer without copying it to C++.
        try
        {
            InputStream input = m_AssetManager.open( path );
			
			int length = 0;
			int data = input.read();
			while( data != -1 )
			{
				length++;
				data = input.read();
			}
			
			input.close();
			
			//Log.v( "Flathead", "   JAVA: GetBinaryFileSize: " + length );
            return length;
        }
        catch(Exception exc)
        {
            System.out.println( "Error in LoadBinaryFile - " + exc.toString() );
        }

        return 0;
    }

    public void LoadBinaryFile(String path, ByteBuffer buffer)
    {
        String npath = path;
        //Log.v( "Flathead", "   JAVA: LoadBinaryFile: " + npath );

        buffer.position(0);

        try
        {
            InputStream input = m_AssetManager.open( path );
			
			int length = 0;
			int data = input.read();
			while( data != -1 )
			{
				buffer.put( length, (byte)data );
				length++;
				data = input.read();
			}
			
			input.close();
        }
        catch(Exception exc)
        {
            System.out.println("Error in LoadBinaryFile - " + exc.toString());
        }
    }

    public Bitmap open(String path)
    {
        try
        {
            Bitmap bitmap = BitmapFactory.decodeStream( m_AssetManager.open(path) );
            //Log.v("Flathead", "BMPFactoryLoader open - success");
            return bitmap;
        }
        catch(Exception e)
        {
            //Log.v("Flathead", "BMPFactoryLoader open - exception");
        }

        return null;
    }

    public int getWidth(Bitmap bmp) { return bmp.getWidth(); }
    public int getHeight(Bitmap bmp) { return bmp.getHeight(); }

    public void getPixels(Bitmap bmp, int[] pixels)
    {
        int w = bmp.getWidth();
        int h = bmp.getHeight();
        bmp.getPixels( pixels, 0, w, 0, 0, w, h );
    }

    public void close(Bitmap bmp)
    {
        bmp.recycle();
    }
}
