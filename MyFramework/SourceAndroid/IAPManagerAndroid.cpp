//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "IAPManagerAndroid.h"
#include "GameCore.h"

// horribly written ATM, will handle a single product id.

IAPManager* g_pIAPManager = 0;

IAPManager::IAPManager()
{
    m_Initialized = true;
}

IAPManager::~IAPManager()
{
}

// Request the purchase from the payment service based on the item's id
void IAPManager::Purchase(const char* IAPProductID)
{
    if( m_Initialized == false )
        return;

    // retrieve the MYFWActivity::GetIAPManager() method
    jclass jclassMainActivity = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    jmethodID jfuncGetIAPManager = g_pJavaEnvironment->GetMethodID( jclassMainActivity, "GetIAPManager", "()Lcom/flathead/MYFWPackage/IAPManager;" );

    // call MYFWActivity::GetIAPManager()
    jobject jobjIAPManager = g_pJavaEnvironment->CallObjectMethod( g_pMainActivity, jfuncGetIAPManager );
    jclass jclassIAPManager = g_pJavaEnvironment->GetObjectClass( jobjIAPManager );

    // call IAPManager::BuyItem()
    jmethodID jfuncBuyItem = g_pJavaEnvironment->GetMethodID( jclassIAPManager, "BuyItem", "()V" );
    g_pJavaEnvironment->CallVoidMethod( jobjIAPManager, jfuncBuyItem );
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
