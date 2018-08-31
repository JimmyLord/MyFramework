//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "UDPSocket.h"

#if _DEBUG && !MYFW_WINDOWS && !MYFW_WP8 && !defined(WSAGetLastErrorDefined)
#define WSAGetLastErrorDefined 1 // Used in Android "unity" build.
static int WSAGetLastError() { return errno; }
#endif

#if MYFW_WINDOWS
#define close closesocket
#endif

UDPSocket::UDPSocket()
{
    m_Port = 0;
    m_SocketHandle = 0;

    m_KeepAliveTime = 0;
    m_TimeSinceLastPacketSent = 0;
}

UDPSocket::~UDPSocket()
{
    if( m_SocketHandle )
        close( m_SocketHandle );
}

void UDPSocket::Create(unsigned short port)
{
    m_SocketHandle = (int)socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if( m_SocketHandle <= 0 )
    {
        LOGInfo( LOGTag, "Failed to create socket\n" );
        m_SocketHandle = 0;
        return;
    }

    int numtriesleft = 10;
    while( 1 )
    {
        sockaddr_in localaddr;
        localaddr.sin_family = AF_INET;
        localaddr.sin_addr.s_addr = INADDR_ANY;
        localaddr.sin_port = htons( port );

        if( bind( m_SocketHandle, (const sockaddr*)&localaddr, sizeof(sockaddr_in) ) < 0 )
        {
            port = 10000 + rand()%5000;
            LOGInfo( LOGTag, "Failed to bind socket - port in use? trying another at random (port %d)\n", port );
            numtriesleft--;
        }
        else
        {
            break;
        }

        if( numtriesleft <= 0 )
        {
            close( m_SocketHandle );
            m_SocketHandle = 0;
            return;
        }
    }

#if MYFW_WINDOWS || MYFW_WP8
    DWORD value = 1;
    if( ioctlsocket( m_SocketHandle, FIONBIO, &value ) != 0 )
    {
        LOGInfo( LOGTag, "non blocking call failed\n" );
        close( m_SocketHandle );
        m_SocketHandle = 0;
        return;
    }
#elif MYFW_ANDROID || MYFW_BLACKBERRY || MYFW_IOS || MYFW_OSX
    if( fcntl( m_SocketHandle, F_SETFL, O_NONBLOCK ) )
    {
        LOGInfo( LOGTag, "non blocking call failed\n" );
        close( m_SocketHandle );
        m_SocketHandle = 0;
        return;
    }
#else
    if( fcntl( m_SocketHandle, F_SETFL, O_NONBLOCK, 1 ) == -1 )
    {
        LOGInfo( LOGTag, "non blocking call failed\n" );
        close( m_SocketHandle );
        m_SocketHandle = 0;
        return;
    }
#endif
}

void UDPSocket::SetupKeepAliveTime(double keepalivetime, const char* keepaliveserver, unsigned short keepaliveport)
{
    LOGInfo( LOGTag, "UDPSocket::SetupKeepAliveTime() calling inet_addr\n" );

    struct in_addr inaddr;
    inaddr.s_addr = inet_addr( keepaliveserver );

    if( inaddr.s_addr == INADDR_NONE )
    {
        //int err = WSAGetLastError();
        //LOGInfo( LOGTag, "UDPSocket::SetupKeepAliveTime() inet_addr failed err=%d\n", err );
        //LOGInfo( LOGTag, "UDPSocket::SetupKeepAliveTime() calling gethostbyname\n" );
        struct hostent* pHost = gethostbyname( keepaliveserver );
        if( pHost == 0 )
        {
#if _DEBUG
            int err = WSAGetLastError();
            LOGInfo( LOGTag, "UDPSocket::SetupKeepAliveTime() inet_addr and gethostbyname failed err=%d\n", err );
#endif
            return;
        }
        
        //inaddr = *(struct in_addr*)(*pHost->h_addr_list);
        inaddr = *(struct in_addr*)*pHost->h_addr_list;
    }

    memset( (char*)&m_KeepAliveServerAddress, 0, sizeof(sockaddr_in) );
    m_KeepAliveServerAddress.sin_family = AF_INET;
    m_KeepAliveServerAddress.sin_port = htons( keepaliveport );
    m_KeepAliveServerAddress.sin_addr.s_addr = inaddr.s_addr;

    m_KeepAliveTime = keepalivetime;
}

void UDPSocket::Close()
{
    close( m_SocketHandle );
    m_TimeSinceLastPacketSent = 0;
}

void UDPSocket::Tick(float deltaTime)
{
    m_TimeSinceLastPacketSent += deltaTime;

    if( m_KeepAliveTime > 0 && m_TimeSinceLastPacketSent > m_KeepAliveTime )
    {
        m_TimeSinceLastPacketSent = 0;

        int bytes = SendTo( (const char*)"blarg", 5, &m_KeepAliveServerAddress );

        if( bytes != 5 )
        {
            close( m_SocketHandle );
            m_SocketHandle = 0;
        }
    }
}

bool UDPSocket::SendTo(const char* message, int messagelen, const int ip, const u_short port)
{
    sockaddr_in saddr;
    memset( (char*)&saddr, 0, sizeof(sockaddr_in) );

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons( port );
    saddr.sin_addr.s_addr = ip;

    return SendTo( message, messagelen, &saddr );
}

bool UDPSocket::SendTo(const char* message, int messagelen, const sockaddr_in* saddr)
{
    if( m_SocketHandle == 0 )
    {
        LOGError( LOGTag, "UDPSocket::SendTo() Trying to use an uninitialized socket\n" );
        return false;
    }

    int bytes = (int)sendto( m_SocketHandle, message, messagelen, 0, (sockaddr*)saddr, sizeof(sockaddr_in) );

    if( bytes != messagelen )
    {
        LOGInfo( LOGTag, "Send packet failed, should have sent %d bytes, only sent %d\n", messagelen, bytes );
        close( m_SocketHandle );
        m_SocketHandle = 0;
        return false;
    }

    return true;
}

int UDPSocket::RecvFrom(char* buffer, int buffersize, sockaddr_in* saddr)
{
    if( m_SocketHandle == 0 )
    {
        LOGError( LOGTag, "UDPSocket::SendTo() Trying to use an uninitialized socket\n" );
        return 0;
    }

    socklen_t fromLength = sizeof( sockaddr_in );
    int bytes = (int)recvfrom( m_SocketHandle, buffer, buffersize, 0, (sockaddr*)saddr, &fromLength );
    return bytes;
}
