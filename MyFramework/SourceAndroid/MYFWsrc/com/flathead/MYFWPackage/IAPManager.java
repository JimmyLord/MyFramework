//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

package com.flathead.MYFWPackage;

import com.android.vending.billing.IInAppBillingService;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

public class IAPManager
{
    public IInAppBillingService m_IAPService = null;
    public Activity m_Activity = null;

    ServiceConnection m_ServiceConnection = new ServiceConnection()
    {
        @Override public void onServiceDisconnected(ComponentName name)
        {
            m_IAPService = null;
        }

        @Override public void onServiceConnected(ComponentName name, IBinder service)
        {
            m_IAPService = IInAppBillingService.Stub.asInterface( service );
        }
    };

    public IAPManager(Activity activity)
    {
        m_Activity = activity;

        // Setup the IAP service
        Intent serviceIntent = new Intent( "com.android.vending.billing.InAppBillingService.BIND" );
        serviceIntent.setPackage( "com.android.vending" );
        m_Activity.bindService( serviceIntent, m_ServiceConnection, Context.BIND_AUTO_CREATE );
    }

    public void Shutdown()
    {
        // Destroy the IAP service
        if( m_IAPService != null ) // IAP
        {
            m_Activity.unbindService( m_ServiceConnection );
            m_Activity = null;
        }
    }

    public void BuyItem()
    {
        try
        {
            InternalBuyItem();
        }
        catch( Exception e )
        {
            Log.v( "Flathead", "Failed to parse purchase data." );
        }
    }

    protected void InternalBuyItem() throws RemoteException, IntentSender.SendIntentException
    {
        Bundle buyIntentBundle = m_IAPService.getBuyIntent( 3, m_Activity.getPackageName(),
                                                            "android.test.purchased", "inapp", "Custom payload string");
//                                                            "android.test.canceled", "inapp", "Custom payload string");
//                                                            "android.test.refunded", "inapp", "Custom payload string");
//                                                            "android.test.item_unavailable", "inapp", "Custom payload string");

        PendingIntent pendingIntent = buyIntentBundle.getParcelable( "BUY_INTENT" );

        m_Activity.startIntentSenderForResult( pendingIntent.getIntentSender(),
                                               1001, new Intent(), Integer.valueOf( 0 ), Integer.valueOf( 0 ),
                                               Integer.valueOf( 0 ) );
    }

    public Boolean OnResult(int requestCode, int resultCode, Intent data)
    {
        if( requestCode == 1001 )
        {
            int responseCode = data.getIntExtra( "RESPONSE_CODE", 0 );
            String purchaseData = data.getStringExtra( "INAPP_PURCHASE_DATA" );
            String dataSignature = data.getStringExtra( "INAPP_DATA_SIGNATURE" );

            if( resultCode == Activity.RESULT_OK )
            {
                try
                {
                    JSONObject jo = new JSONObject( purchaseData );
                    String sku = jo.getString( "productId" );
                    Log.v( "Flathead", "You have bought the " + sku + ". Excellent choice, adventurer !");
                }
                catch( JSONException e )
                {
                    Log.v( "Flathead", "Failed to parse purchase data." );
                    e.printStackTrace();
                }
            }

            return true;
        }

        return false;
    }
}
