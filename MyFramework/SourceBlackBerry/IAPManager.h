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

#ifndef __IAPManager_H__
#define __IAPManager_H__

class IAPManager;
extern IAPManager* g_pIAPManager;

class IAPManager
{
protected:
    bool m_Initialized;

    unsigned int m_PurchaseRequestID; // currently set but never used otherwise.
    unsigned int m_GetExistingPurchasesRequestID; // currently set but never used otherwise.

public:
    IAPManager();
    virtual ~IAPManager();

    //bool Tick(); // returns true if waiting for any results.
    void HandleEvent(bps_event_t* event);

    void Purchase(const char* id, const char* sku);//const QString &sku, const QString &name, const QString &metadata);
    void GetExistingPurchases();//bool refresh);
    //void getPrice(const QString &id, const QString &sku);
    //void getSubscriptionTerms(const QString &id, const QString &sku);
    //void checkSubscriptionStatus(const QString &id, const QString &sku);
    //void cancelSubscription(const QString &purchaseId);

public:
};

#endif //__IAPManager_H__
