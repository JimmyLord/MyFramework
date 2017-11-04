//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __UDPSocket_H__
#define __UDPSocket_H__

// TODONACLUPDATE: try to unignore UPDSocket, webrequest and stunclient.

class UDPSocket
{
public:
    unsigned short m_Port;
    int m_SocketHandle;

    sockaddr_in m_KeepAliveServerAddress; // some server that we'll send a ping to every now and then to keep the socket open.
    double m_KeepAliveTime;
    double m_TimeSinceLastPacketSent;

public:
    UDPSocket();
    ~UDPSocket();

    void Create(unsigned short port);
    void SetupKeepAliveTime(double keepalivetime, const char* keepaliveserver, unsigned short keepaliveport);
    void Close();
    void Tick(double TimePassed);

    bool SendTo(const char* message, int messagelen, const sockaddr_in* saddr);
    bool SendTo(const char* message, int messagelen, const int ip, const u_short port);
    int RecvFrom(char* buffer, int buffersize, sockaddr_in* saddr);
};

#endif //__UDPSocket_H__
