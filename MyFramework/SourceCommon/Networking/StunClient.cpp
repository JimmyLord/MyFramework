//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "StunClient.h"
#include "UDPSocket.h"

#if !MYFW_WINDOWS && !MYFW_WP8 && !defined(WSAGetLastErrorDefined)
#define WSAGetLastErrorDefined 1 // Used in Android "unity" build.
static int WSAGetLastError() { return errno; }
#endif

#if MYFW_WINDOWS
#define close closesocket
#endif

StunClient::StunClient()
{
    m_pUDPSocket = 0;

    Reset();
}

StunClient::~StunClient()
{
    //SAFE_DELETE( m_pUDPSocket ); // DON'T delete, handled outside of this class.
}

void StunClient::Reset()
{
    m_Started = false;
    m_TimeSinceLastBindingRequestSent = 0;
    m_NumberOfBindingRequestsSent = 0;
    m_HaveExternalIPAddr = false;

    m_SomethingWentWrong = false;

    m_ExternalIPAddr = 0;
    m_ExternalPort = 0;

    m_BindingRequest[0] = 0;
    m_BindingRequestLength = 0;
    m_BindingResponse[0] = 0;
    m_BindingResponseLength = 0;
}

void StunClient::Start(UDPSocket* socket, const char* stunserver, unsigned short stunport)
{
    if( m_Started )
        return;

    m_pUDPSocket = socket;

    m_Started = true;

    LOGInfo( LOGTag, "StunClient::Init() calling inet_addr\n" );

    struct in_addr inaddr;
    inaddr.s_addr = inet_addr( stunserver );

    if( inaddr.s_addr == INADDR_NONE )
    {
        //int err = WSAGetLastError();
        //LOGInfo( LOGTag, "StunClient::Start() inet_addr failed err=%d\n", err );
        //LOGInfo( LOGTag, "StunClient::Start() calling gethostbyname\n" );
        struct hostent* pHost = gethostbyname( stunserver );
        if( pHost == 0 )
        {
#if MYFW_WINDOWS
            int err = WSAGetLastError();
            LOGInfo( LOGTag, "StunClient::Start() inet_addr and gethostbyname failed err=%d\n", err );
#endif
            m_SomethingWentWrong = true;
            return;
        }
        
        //inaddr = *(struct in_addr*)(*pHost->h_addr_list);
        inaddr = *(struct in_addr*)*pHost->h_addr_list;
    }

    memset( (char*)&m_StunServerAddress, 0, sizeof(sockaddr_in) );
    m_StunServerAddress.sin_family = AF_INET;
    m_StunServerAddress.sin_port = htons( stunport );
    m_StunServerAddress.sin_addr.s_addr = inaddr.s_addr;

    m_TimeSinceLastBindingRequestSent = 99999;

    LOGInfo( LOGTag, "StunClient::Start() ip=%d\n", inaddr.s_addr );
}

void StunClient::Tick(float deltaTime)
{
    if( m_Started == false )
        return;

    if( m_SomethingWentWrong )
        return;

    if( m_HaveExternalIPAddr )
        return;

    // Check for responses:
    {
        // https://tools.ietf.org/html/rfc5389
        // http://www.3cx.com/blog/voip-howto/stun-details/
        // http://www.networksorcery.com/enp/protocol/stun.htm

        // header: 20 bytes total.
        // Message type                 - 2 bytes  - 0×0101 : Binding Response
        // Length of data after header  - 2 bytes  - depends on payload, but we're expecting MAPPED-ADDRESS as minimum.
        // Magic cookie                 - 4 bytes  - always 0x2112A442
        // Transaction ID.              - 12 bytes - should be value sent in Binding Request

        // payload
        // 0×0001: MAPPED-ADDRESS - external ip and port

        // TODO:
        // process received packet

#if MYFW_WINDOWS
        typedef int socklen_t;
#endif

        MyAssert( m_pUDPSocket );

        {
            sockaddr_in from;
            int bytes = m_pUDPSocket->RecvFrom( (char*)m_BindingResponse, 500, &from );

            if( bytes > 0 )
            {
                //unsigned int from_address = ntohl( from.sin_addr.s_addr );
                //unsigned int from_port = ntohs( from.sin_port );

                u_short type = ntohs( *(u_short*)(&m_BindingResponse[0]) );
                if( type == 0x0101 ) // if "Binding Response"
                {
                    u_short datalen = ntohs( *(u_short*)(&m_BindingResponse[2]) );
                    MyAssert( datalen <= 500-20 ); // simple hardcoded sanity check.

                    if( datalen > 500-20 )
                    {
                        LOGInfo( LOGTag, "StunClient: Data Length too long or short: len=%d\n", datalen );
                        m_SomethingWentWrong = true;
                        return;
                    }
                    else
                    {
                        u_long cookie = ntohl( *(u_long*)(&m_BindingResponse[4]) );
                        MyAssert( cookie == 0x2112a442 );

                        // ignore transaction id... for now?

                        int offset = 20;
                        while( offset < datalen + 20 ) // 20 is stun header.
                        {
                            u_short attribute = ntohs( *(u_short*)(&m_BindingResponse[offset]) );
                            u_short attrlen = ntohs( *(u_short*)(&m_BindingResponse[offset+2]) );

                            // more sanity checks.
                            if( offset + attrlen > datalen )
                            {
                                LOGInfo( LOGTag, "StunClient: Attr Length too long or short: len=%d\n", attrlen );
                                m_SomethingWentWrong = true;
                                return;
                            }
                            else
                            {
                                if( attribute == 0x0001 || attribute == 0x0020 ) // MAPPED_ADDRESS or XOR_MAPPED_ADDRESS
                                {
                                    MyAssert( attrlen == 8 );
                                    if( attrlen != 8 )
                                    {
                                        LOGInfo( LOGTag, "StunClient: Attr Length should be 8: len=%d\n", attrlen );
                                        m_SomethingWentWrong = true;
                                        return;
                                    }

                                    u_short protocolfamily = ntohs( *(u_short*)(&m_BindingResponse[offset+4]) );

                                    MyAssert( protocolfamily == 1 );
                                    if( protocolfamily != 1 )
                                    {
                                        LOGInfo( LOGTag, "StunClient: We don't handle IPv6\n" );
                                        m_SomethingWentWrong = true;
                                        return;
                                    }

                                    u_short port = ntohs( *(u_short*)(&m_BindingResponse[offset+6]) );
                                    u_long ipaddr = *(u_long*)(&m_BindingResponse[offset+8]);//ntohl(  );

                                    if( attribute == 0x0020 ) // untested
                                    {
                                        m_ExternalIPAddr = ipaddr ^ 0x2112a442;
                                        m_ExternalPort = port ^ (0x2112a442 >> 16);
                                    }
                                    else
                                    {
                                        m_ExternalIPAddr = ipaddr;
                                        m_ExternalPort = port;
                                    }

                                    m_HaveExternalIPAddr = true;

                                    return;
                                }
                            }

                            offset += attrlen + 4; // 4 is attr header - type & length;
                        }
                    }
                }

                ////0x0020: XOR-MAPPED-ADDRESS
                //
                //01 01 - Binding Response
                //00 44 - len
                //21 12 a4 42 - magic cookie ( xor with this if XOR-MAPPED-ADDRESS )
                //95 d6 c5 18 5f ac 7d ce 15 38 b1 29 - transaction id... should match rand() below. don't care.
                //
                //00 01 - MAPPED-ADDRESS
                //00 08 - length
                //00 01 - protocol family - 1 for IPv4, 2 for IPv6
                //27 0f - port 9999
                //45 a5 85 53 - ip 69.165.133.83
                //
                //00 04 - SOURCE-ADDRESS
                //00 08 - length
                //00 01 - protocol family
                //0d 96 d9 0a 44 98 - port then ip.
                //
                //00 05 - CHANGED-ADDRESS
                //00 08 - length
                //00 01 - protocol family
                //0d 97 d9 0a 44 ae - port then ip.
                //
                //80 20 - no clue
                //00 08
                //00 01 06 1d 64 b7 21 11
                //
                //80 22 - no clue
                //00 10
                //56 6f 76 69 64 61 2e 6f 72 67 20 30 2e 39 36 00
            }
        }
    }

    m_TimeSinceLastBindingRequestSent += deltaTime;

    // send a new request 5 seconds after last one failed.
    if( m_TimeSinceLastBindingRequestSent > 5 )
    {
        if( m_NumberOfBindingRequestsSent >= 1 ) // fail after 5 send requests with no reply.
        {
            //m_SomethingWentWrong = true;
            return;
        }

        m_NumberOfBindingRequestsSent++;

        // send a binding request.
        {
            // header: 20 bytes total.
            // Message type                 - 2 bytes  - 0×0001 : Binding Request
            // Length of data after header  - 2 bytes  - 0 in our case
            // Magic cookie                 - 4 bytes  - always 0x2112A442
            // Transaction ID.              - 12 bytes - random

            // TODO:
            //MyAssert( false );

            // generate a binding request packet:
            *((u_short*)&m_BindingRequest[0]) = htons( 0x0001 );
            *((u_short*)&m_BindingRequest[2]) = htons( 0 );
            *((u_long*)&m_BindingRequest[4]) = htonl( 0x2112A442 );
            for( int i=0; i<12; i++ )
                m_BindingRequest[8+i] = rand()%255; // randomly make a 12 byte number, 1 byte at a time.
            m_BindingRequestLength = 20;

            bool succ = m_pUDPSocket->SendTo( (const char*)m_BindingRequest, m_BindingRequestLength, &m_StunServerAddress );
            if( succ == false )
            {
                m_SomethingWentWrong = true;
                return;
            }
        }
    }
}

char tempipaddr[32];
const char* StunClient::GetExternalIPAsString()
{
    if( m_HaveExternalIPAddr == false )
        return 0;

    sprintf_s( tempipaddr, 32, "%lu.%lu.%lul.%lu:%d",
                m_ExternalIPAddr & 0xff,
                m_ExternalIPAddr>>8 & 0xff,
                m_ExternalIPAddr>>16 & 0xff,
                m_ExternalIPAddr>>24 & 0xff,
                m_ExternalPort );

    return tempipaddr;
}

//// from: http://pastebin.com/vKBnTWgE and not using it... keeping for future reference if needed.
//
////(c) joric^proxium, 2010, public domain                   
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <memory.h>
//
//#include "StunClient.h"
//
//STUN m_stun;
//LINK m_link;
//
//void net_recv(char *buf, int len)
//{
//    stun_recv(&m_stun, buf, len);
//}
//
//int main(int argc, char **argv)
//{
//    int port = 3478;
//    char *host = "stun.counterpath.net";
//
//    if (argc > 1)
//        host = argv[1];
//
//    net_init();
//
//    LINK *link = &m_link;
//    STUN *stun = &m_stun;
//
//    link->host = host;
//    link->port = port;
//
//    stun_start(link, stun);
//
//    while (stun_update(stun))
//        Sleep(5);
//
//
//    //make a mask for tests, i.e. 1,0,1,1 == 0x1011
//    int i, mask = 0;
//    for (i = 0; i < 4; i++)
//        mask |= stun->results[i] << (3 - i) * 4;
//
//    char *type;
//    if(stun->results[3]==1)
//        type = "Address restricted NAT";
//    else
//        type = "Port restricted NAT";
//
//
//
//    switch (mask)
//    {        
//    case 0x0000:
//        type = "UDP Blocked";
//        break;
//    case 0x1000:
//        type = "UDP Firewall";
//        break;
//    case 0x1100:
//        type = "Open Internet";
//        break;
//    case 0x1011:
//        type = "Full Cone NAT";
//        break;
//    }
//    printf("\n\nResults\n-------------------------------\n");
//    printf("tests: %04x\n", mask);
//    printf("NAT present: %d\n", stun->nat);
//    printf("first preserved port: %d\n", stun->fpresport);
//    printf("preserves port: %d\n", stun->presport);
//    if(stun->nat)
//        printf("type: %s\n", type);
//
//    printf("mapped ports: ");
//    for(i = 0; i<4; i++)
//        printf("%d ",stun->mappedports[i]);
//
//    printf("\n");
//}
