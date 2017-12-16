//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "IAPManagerIOS.h"
#include "GameCore.h"

#import <StoreKit/StoreKit.h>

// horribly written ATM, will handle a single product id.

IAPManager* g_pIAPManager = 0;

@interface IAPInterface : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
@public
	bool m_Busy;
    SKProduct* m_ProductRequested;
    SKProductsRequest* m_ProductRequest;
}

// public
- (id)init;

- (bool)CanMakePurchases;
- (void)StartPurchase:(const char*)ProductID; // kick off a purchase request with a string id.
- (void)RestorePurchases;

// private
- (void)Purchase:(SKProduct*)ProductObject; // private, finish the transaction

@end

@implementation IAPInterface

- (id)init
{
    if( self = [super init] )
    {
        m_Busy = false;
        m_ProductRequested = 0;
        m_ProductRequest = 0;

        // set ourselves up as an observer, also restarts any purchases if they were interrupted last time the app was open
        [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
    }

    return self;
}

- (bool)CanMakePurchases
{
    return [SKPaymentQueue canMakePayments];
}

- (void)StartPurchase:(const char*)ProductID
{
    MyAssert( m_Busy == false );
    if( m_Busy == true )
        return;

    m_Busy = true;

    NSString* NSProductID = [NSString stringWithUTF8String:ProductID];

    NSSet* productIdentifiers = [NSSet setWithObject:NSProductID];
    m_ProductRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    m_ProductRequest.delegate = self;
    [m_ProductRequest start];
}

- (void)RestorePurchases
{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse *)response
{
    NSArray* products = response.products;

    if( m_ProductRequested )
        [m_ProductRequested release];

    m_ProductRequested = [products count] == 1 ? [[products firstObject] retain] : nil;
    if( m_ProductRequested )
    {
        NSLog( @"Product title: %@", m_ProductRequested.localizedTitle );
        NSLog( @"Product description: %@", m_ProductRequested.localizedDescription );
        NSLog( @"Product price: %@", m_ProductRequested.price );
        NSLog( @"Product id: %@", m_ProductRequested.productIdentifier );

        [self Purchase:(m_ProductRequested)];
    }

    for( NSString* invalidProductId in response.invalidProductIdentifiers )
    {
        NSLog( @"Invalid product id: %@", invalidProductId );

        m_Busy = false;
    }

    [m_ProductRequest release];
}

- (void)Purchase:(SKProduct*)ProductObject
{
    SKPayment* payment = [SKPayment paymentWithProduct:ProductObject];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray*)transactions
{
//    if( m_Busy == false )
//        return;

    for( SKPaymentTransaction* transaction in transactions )
    {
        switch( transaction.transactionState )
        {
            case SKPaymentTransactionStatePurchasing:
                break;

            case SKPaymentTransactionStatePurchased:
            	{
                    m_Busy = false;
                    const char* productid = [transaction.payment.productIdentifier UTF8String]; //[m_ProductRequested.productIdentifier UTF8String];
                    g_pGameCore->OnPurchaseComplete( 0, productid, IAPErrorCode_Success, true );
                    NSLog( @"OnPurchaseComplete: Success" );
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                }
                break;

            case SKPaymentTransactionStateFailed:
                {
                    m_Busy = false;
                    g_pGameCore->OnPurchaseComplete( 0, 0, IAPErrorCode_PaymentError, true );
                    NSLog( @"OnPurchaseComplete: Failed" );
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                }
                break;

            case SKPaymentTransactionStateRestored:
                {
                    m_Busy = false;
                    const char* productid = [transaction.payment.productIdentifier UTF8String];
                    g_pGameCore->OnPurchaseComplete( 0, productid, IAPErrorCode_Success, false );
                    NSLog( @"OnPurchaseComplete: Success Restore" );
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                }
                break;

            default:
                break;
        }
    }
}

@end

IAPInterface* g_pIAPInterface = 0;

IAPManager::IAPManager()
{
    m_Initialized = false;

    MyAssert( g_pIAPInterface == 0 );
    g_pIAPInterface = [IAPInterface new];

    m_Initialized = false;
}

IAPManager::~IAPManager()
{
}

void IAPManager::Initialize()
{
    m_Initialized = true;
}

// Request the purchase from the payment service based on the item's id
void IAPManager::Purchase(const char* IAPProductID)
{
    if( m_Initialized == false )
        return;

    if( g_pIAPInterface->m_Busy )
    	return;

    [g_pIAPInterface StartPurchase:(IAPProductID)];
}

// Request existing purchases from the payment service.
void IAPManager::GetExistingPurchases()
{
    if( m_Initialized == false )
        return;

    if( g_pIAPInterface->m_Busy )
        return;

    [g_pIAPInterface RestorePurchases];
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
