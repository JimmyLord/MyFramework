//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "IAPManagerAndroid.h"
#include "Core/GameCore.h"

IAPManager* g_pIAPManager = 0;

IAPManager::IAPManager()
{
    m_Initialized = false;

    m_AttemptingNewPurchase = false;
    m_IAPProductID = 0;
}

IAPManager::~IAPManager()
{
}

void IAPManager::Initialize()
{
    m_Initialized = true;

    g_pEventManager->RegisterForEvents( Event_IAP, this, &IAPManager::StaticHandleEvent );
}

bool IAPManager::HandleEvent(MyEvent* pEvent)
{
    if( m_Initialized == false )
        return false;

    if( pEvent->GetType() == Event_IAP )
    {
        int responseCode = pEvent->GetInt( "responseCode" );
        
        const char* sku = (const char*)pEvent->GetPointer( "sku" );
        const char* payload = (const char*)pEvent->GetPointer( "payload" );

        LOGInfo( LOGTag, "Received Event_IAP %d, %s, %s\n", responseCode, sku, payload );

        //BILLING_RESPONSE_RESULT_OK                    0   Success
        //BILLING_RESPONSE_RESULT_USER_CANCELED         1   User pressed back or canceled a dialog
        //BILLING_RESPONSE_RESULT_SERVICE_UNAVAILABLE   2   Network connection is down
        //BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE   3   Billing API version is not supported for the type requested
        //BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE      4   Requested product is not available for purchase
        //BILLING_RESPONSE_RESULT_DEVELOPER_ERROR       5   Invalid arguments provided to the API. This error can also indicate that the application was not correctly signed or properly set up for In-app Billing in Google Play, or does not have the necessary permissions in its manifest
        //BILLING_RESPONSE_RESULT_ERROR                 6   Fatal error during the API action
        //BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED    7   Failure to purchase since item is already owned
        //BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED        8   Failure to consume since item is not owned

        if( responseCode == 0 || responseCode == 7 ) // BILLING_RESPONSE_RESULT_OK || BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED
        {
            g_pGameCore->OnPurchaseComplete( 0, sku, IAPErrorCode_Success, m_AttemptingNewPurchase );

//            if( eventcode == GET_EXISTING_PURCHASES_RESPONSE )
//            {
//                LOGInfo( LOGTag, "Payment System eventcode == GET_EXISTING_PURCHASES_RESPONSE\n" );
//            
//                int numPurchases = paymentservice_event_get_number_purchases( event );
//                for( int i=0; i<numPurchases; i++ )
//                {
//                    const char* id = paymentservice_event_get_digital_good_id( event, i );
//                    const char* sku = paymentservice_event_get_digital_good_sku( event, i );
//            
//                    g_pGameCore->OnPurchaseComplete( id, sku, IAPErrorCode_Success, false );
//                }
//            }
        }
        else
        {
#if _DEBUG
            //const char* error_text = paymentservice_event_get_error_text( event );
            LOGError( LOGTag, "Payment System error: ID: %d  Text: %s\n", responseCode, "N/A" );
#endif
        
            // let the game code know if the error message is from a new purchase request,
            // as opposed to an existing purchase lookup.
            if( responseCode == 1 ) // BILLING_RESPONSE_RESULT_USER_CANCELED
                g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_UserCancelled, m_AttemptingNewPurchase );
            else if( responseCode == 2 ) // BILLING_RESPONSE_RESULT_SERVICE_UNAVAILABLE
                g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_NoNetworkConnectivity, m_AttemptingNewPurchase );
            else
                g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_UnknownError, m_AttemptingNewPurchase );
        }

        m_AttemptingNewPurchase = false;
        m_IAPProductID = 0;

        return true;
    }

    return false;
}

// Request the purchase from the payment service based on the item's id
void IAPManager::Purchase(const char* IAPProductID)
{
    if( m_Initialized == false )
        return;

    m_AttemptingNewPurchase = true;
    m_IAPProductID = IAPProductID;

    // retrieve the MYFWActivity::GetIAPManager() method
    jclass jclassMainActivity = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    jmethodID jfuncGetIAPManager = g_pJavaEnvironment->GetMethodID( jclassMainActivity, "GetIAPManager", "()Lcom/flathead/MYFWPackage/IAPManager;" );

    // call MYFWActivity::GetIAPManager()
    jobject jobjIAPManager = g_pJavaEnvironment->CallObjectMethod( g_pMainActivity, jfuncGetIAPManager );
    jclass jclassIAPManager = g_pJavaEnvironment->GetObjectClass( jobjIAPManager );

    // call IAPManager::BuyItem()
    jmethodID jfuncBuyItem = g_pJavaEnvironment->GetMethodID( jclassIAPManager, "BuyItem", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V" );

    //jstring jsku = g_pJavaEnvironment->NewStringUTF( "android.test.purchased" );
    jstring jsku = g_pJavaEnvironment->NewStringUTF( IAPProductID );
    jstring jtype = g_pJavaEnvironment->NewStringUTF( "inapp" );
    jstring jpayload = g_pJavaEnvironment->NewStringUTF( "NoPayload" );//Custom payload string" );
    g_pJavaEnvironment->CallVoidMethod( jobjIAPManager, jfuncBuyItem, jsku, jtype, jpayload );
}

// Request existing purchases from the payment service.
void IAPManager::GetExistingPurchases()
{
    if( m_Initialized == false )
        return;
}

////// Query the payment service for an item's price based on its ID and SKU.
////void IAPManager::getPrice(const QString &id, const QString &sku)
////{
////}
////
////// Query the payment service for itmes subscription terms based on its ID, and SKU.
////void IAPManager::getSubscriptionTerms(const QString &id, const QString &sku)
////{
////}
////
////// Query the payment service for an item's subscription status based on its ID and SKU.
////void IAPManager::checkSubscriptionStatus(const QString &id, const QString &sku)
////{
////}
////
////// Cancel item's subscription based on the purchase ID of that item.
////void IAPManager::cancelSubscription(const QString &purchaseId)
////{
////}
