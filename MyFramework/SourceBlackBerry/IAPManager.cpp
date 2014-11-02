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

#include "CommonHeader.h"

#include "IAPManager.h"

#include "bbutil.h"

IAPManager* g_pIAPManager = 0;

IAPManager::IAPManager()
{
    m_Initialized = false;

    if( BPS_SUCCESS != paymentservice_request_events( 0 ) )
    {
        LOGError( LOGTag, "paymentservice_request_events failed\n" );
        return;
    }

    m_Initialized = true;

    // set paymentservice_set_connection_mode to true for debugging.
    //paymentservice_set_connection_mode( true );

    m_PurchaseRequestID = 0;
    m_GetExistingPurchasesRequestID = 0;
}

IAPManager::~IAPManager()
{
}

//bool IAPManager::Tick()
//{
//    return false;
//}

void IAPManager::HandleEvent(bps_event_t* event)
{
    if( m_Initialized == false )
        return;

    unsigned int requestid = paymentservice_event_get_request_id( event );
    unsigned int responsecode = paymentservice_event_get_response_code( event );

    if( responsecode == SUCCESS_RESPONSE )
    {
        int eventcode = bps_event_get_code( event );

        if( eventcode == PURCHASE_RESPONSE )
        {
            LOGInfo( LOGTag, "Payment System eventcode == PURCHASE_RESPONSE\n" );

            const char* id = paymentservice_event_get_digital_good_id( event, 0 );
            const char* sku = paymentservice_event_get_digital_good_sku( event, 0 );

            g_pGameCore->OnPurchaseComplete( id, sku, IAPErrorCode_Success, true );
        }

        if( eventcode == GET_EXISTING_PURCHASES_RESPONSE )
        {
            LOGInfo( LOGTag, "Payment System eventcode == GET_EXISTING_PURCHASES_RESPONSE\n" );

            int numPurchases = paymentservice_event_get_number_purchases( event );
            for( int i=0; i<numPurchases; i++ )
            {
                const char* id = paymentservice_event_get_digital_good_id( event, i );
                const char* sku = paymentservice_event_get_digital_good_sku( event, i );

                g_pGameCore->OnPurchaseComplete( id, sku, IAPErrorCode_Success, false );
            }
        }
    }
    else if( responsecode == FAILURE_RESPONSE )
    {
        int error_id = paymentservice_event_get_error_id( event );

#if _DEBUG
        const char* error_text = paymentservice_event_get_error_text( event );
        LOGError( LOGTag, "Payment System error: ID: %d  Text: %s\n", error_id, error_text ? error_text : "N/A" );
#endif

        // let the game code know if the error message is from a new purchase request,
        // as opposed to an existing purchase lookup.
        bool newpurchase = (requestid == m_PurchaseRequestID);

        if( error_id == 1 ) //PAYMENTSERVICE_ERROR_USER_CANCELLED )
            g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_UserCancelled, newpurchase );
        else if( error_id == 2 ) //PAYMENTSERVICE_ERROR_SYSTEM_BUSY )
            g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_PaymentSystemBusy, newpurchase );
        else if( error_id == 3 ) //PAYMENTSERVICE_ERROR_PAYMENTSERVICE_FAILED )
            g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_PaymentError, newpurchase );
        else if( error_id == 8 ) //PAYMENTSERVICE_ERROR_NO_NETWORK )
            g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_NoNetworkConnectivity, newpurchase );
        else
            g_pGameCore->OnPurchaseComplete( "", "", IAPErrorCode_UnknownError, newpurchase );
    }

    //if( eventcode == FAILURE_RESPONSE )
    //{
    //    paymentservice_event_get_error_info();
    //    paymentservice_event_get_error_text();
    //}
}

// Request the purchase from the payment service based on the item's id, sku, name and metadata.
void IAPManager::Purchase(const char* id, const char* sku) //const QString &id, const QString &sku, const QString &name, const QString &metadata)
{
    if( m_Initialized == false )
        return;

    int error = paymentservice_purchase_request( id, sku, 0, 0, 0, 0, get_window_group_id(), &m_PurchaseRequestID );
    if( error == BPS_FAILURE )
    {
        LOGError( LOGTag, "IAPManager::Purchase() failed\n" );
    }
}

// Request existing purchases from the payment service.
void IAPManager::GetExistingPurchases() //bool refresh)
{
    if( m_Initialized == false )
        return;

    int error = paymentservice_get_existing_purchases_request( true, get_window_group_id(), &m_GetExistingPurchasesRequestID );
    if( error == BPS_FAILURE )
    {
        LOGError( LOGTag, "IAPManager::GetExistingPurchases() failed\n" );
    }
}

//// Query the payment service for an item's price based on its ID and SKU.
//void IAPManager::getPrice(const QString &id, const QString &sku)
//{
//}
//
//// Query the payment service for itmes subscription terms based on its ID, and SKU.
//void IAPManager::getSubscriptionTerms(const QString &id, const QString &sku)
//{
//}
//
//// Query the payment service for an item's subscription status based on its ID and SKU.
//void IAPManager::checkSubscriptionStatus(const QString &id, const QString &sku)
//{
//}
//
//// Cancel item's subscription based on the purchase ID of that item.
//void IAPManager::cancelSubscription(const QString &purchaseId)
//{
//}
