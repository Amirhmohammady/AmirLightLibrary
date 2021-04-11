#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif // _WIN32_WINNT
#define buff_len 2048
#include "../Headers/AmirNetwork.h"
#include "../Headers/AmirLib1Globals.h"
#include "../Headers/AmirException.h"
#include <thread>
#include <ws2tcpip.h>

using namespace std;

//you cat make client_connections non static, but you should wait in server socket destructor to close all server client connections threads.
//and you should enable thread counter.
//std::vector<AmirServerSocket::ClientConnection> AmirServerSocket::client_connections;
mutex AmirServerSocket::mtx;
condition_variable AmirServerSocket::cv;
mutex AmirClientSocket::mtx;
condition_variable AmirClientSocket::cv;
//===============================================================
WSADATA wsa;
enum WsaUsage
{
    notChecked, notUsedBefore, UsedBefore
} isWsaUsedBefore = notChecked;
int WSAuseres = 0;
//===============================================================
bool isWinsockInitialized()
{
    unsigned int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED)
    {
        return false;
    }
    closesocket(s);
    return true;
}
//===============================================================
AmirServerSocket::AmirServerSocket()
{
    if (isWsaUsedBefore == notChecked)
    {
        _AMIRLOG(AmirLib1LOG, _INFO,"Checking Winsock DLL Initialization ... ");
        if (isWinsockInitialized())
        {
            _AMIRLOG(AmirLib1LOG, _INFO,"Winsock DLL Initialized Before.");
            isWsaUsedBefore = UsedBefore;
        }
        else
        {
            _AMIRLOG(AmirLib1LOG, _INFO,"Winsock DLL not Initialized Before.");
            isWsaUsedBefore = notUsedBefore;
        }
    }
    if (isWsaUsedBefore == notUsedBefore)
    {
        if (!WSAuseres)
        {
            _AMIRLOG(AmirLib1LOG, _INFO,"Initializing Winsock DLL for process ... ");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                _AMIRLOG(AmirLib1LOG, _ERROR,AmirString("Failed. Error Code : ")<<WSAGetLastError());
                throw AmirException("Fail to Initializing WSA.");
            }
            _AMIRLOG(AmirLib1LOG, _INFO,"Initialized.");
        }
        WSAuseres++;
    }
}
//===============================================================
AmirServerSocket::AmirServerSocket(unsigned int listenport, ServerEvent &srvrevnt) :AmirServerSocket()
{
    setPortandEventClass(listenport, srvrevnt);
}
//===============================================================
void AmirServerSocket::setPortandEventClass(unsigned int listenport, ServerEvent &srvrevnt)
{
    this->srvrevnt = &srvrevnt;
    this->listenport = listenport;
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("Checking if port ")<<listenport<<" is used or not.");
    if (isPortOpen(listenport))
    {
        _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("port ")<<listenport<<" is used by another process.");
        throw AmirException(AmirString("Error port ")<<listenport<<" is used!");
    }
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("port ")<<listenport<<" is not used.");
    isinitialized = true;
}
//===============================================================
void AmirServerSocket::start()
{
    if (isinitialized)
    {
        if(is_threads_in_use) return;
        is_threads_in_use = true;
        _AMIRLOG(AmirLib1LOG, _INFO,AmirString("Thread ID:") << GetCurrentThreadId() << "  Creating ServerSocket...");
        struct addrinfo *result = NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        AmirString as(listenport);
        if (getaddrinfo(NULL, as.c_str(), &hints, &result) != 0)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("resolve the server address and port failed with error: ")<< WSAGetLastError());
            isinitialized = false;
            is_threads_in_use = false;
            throw AmirException("resolve the server address and port failed.");
        }
        // Create a SOCKET for connecting to server
        server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (server_socket <= 0)//== INVALID_SOCKET)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("Could not create socket: ")<< WSAGetLastError());
            freeaddrinfo(result);
            isinitialized = false;
            is_threads_in_use = false;
            throw AmirException("Could not create server socket");
        }
        isserversocketactive = true;
        _AMIRLOG(AmirLib1LOG, _INFO, "Socket created.");
        // Setup the TCP listening socket
        if (bind(server_socket, result->ai_addr, (int)result->ai_addrlen) <= SOCKET_ERROR)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("bind failed with error: ")<< WSAGetLastError());
            freeaddrinfo(result);
            closesocket(server_socket);
            isinitialized = false;
            is_threads_in_use = false;
            throw AmirException("bind failed");
        }
        _AMIRLOG(AmirLib1LOG, _INFO, "Bind done.");
        freeaddrinfo(result);//free getaddrinfo
        // Listen to incoming connections
        if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("listen failed with error: ")<< WSAGetLastError());
            closesocket(server_socket);
            isinitialized = false;
            is_threads_in_use = false;
            throw AmirException("listen failed.");
        }
        thread srvthrd(startServerThread, this);
        srvthrd.detach();
    }
    else throw AmirException("server socket starting failed Port and Server Event Class not set!");
}
//===============================================================
bool AmirServerSocket::isStarted()
{
    return (isinitialized && is_threads_in_use);
}
//===============================================================
void AmirServerSocket::closeClientConnection(unsigned int connectionID)
{
    if (connectionID<client_connections.size())
        if (client_connections[connectionID].issocketactive)
        {
            client_connections[connectionID].issocketactive = false;
            closesocket(client_connections[connectionID].skt);
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("connection ")<<connectionID<<" closed");
        }
}
//===============================================================
void AmirServerSocket::closeAllClientConnection()
{
    for (unsigned int z1=0; z1<client_connections.size(); z1++)
        closeClientConnection(z1);
}
//===============================================================
void AmirServerSocket::closeServerConnection()
{
    if(isserversocketactive){
        closeAllClientConnection();
        isserversocketactive = false;
        closesocket(server_socket);
        _AMIRLOG(AmirLib1LOG, _INFO, "server closed!");
    }
}
//===============================================================
void AmirServerSocket::sendToClient(unsigned int connectionID,
                                    char* buff, unsigned int bufflen)
{
    if (connectionID<client_connections.size())
        if (client_connections[connectionID].issocketactive)
            send(client_connections[connectionID].skt, buff, bufflen, 0);
}
//===============================================================
void AmirServerSocket::startServerThread()
{
    //Accept and incoming connection
    _AMIRLOG(AmirLib1LOG, _INFO, "Waiting for incoming connections...");
    int addrlen = sizeof(struct sockaddr_in);

    int sn = getUnActiveClientID();
    struct sockaddr_in client;//, server;
    while ((client_connections[sn].skt = accept(server_socket, (struct sockaddr *)&client, &addrlen)) != INVALID_SOCKET)
    {
        client_connections[sn].issocketactive = true;
        char *client_ip = inet_ntoa(client.sin_addr);
        int client_port = ntohs(client.sin_port);
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("incoming request from ")<< client_ip << ':' << client_port);
        threadmessage = 0;
        _AMIRLOG(AmirLib1LOG, _INFO, "starting client Thread...");
        client_connections[sn].isthreadactive = true;
        aliveclientthreads++;
        thread th01(clientThread, this, sn);
        th01.detach();
        _AMIRLOG(AmirLib1LOG, _INFO, "waiting for client thread start...");
        while (!threadmessage)
            Sleep(100);
        _AMIRLOG(AmirLib1LOG, _INFO, "new client thread started.");
        sn = getUnActiveClientID();
    }
    //closeServerConnection();
    if (isserversocketactive) srvrevnt->onCloseServerSocket();
    isserversocketactive = false;
    isinitialized = false;
    //issocketalive = false;
    is_threads_in_use = false;
    unique_lock<mutex> lck(mtx);
    cv.notify_all();
}
//===============================================================
void AmirServerSocket::clientThread(int connectionID)
{
    int sn = connectionID;
    threadmessage = 1;
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("Accepting client socket: ")<<sn<<" ... Thread ID: "<< GetCurrentThreadId());

    /*if (client_connections[sn].skt <= 0 )//== INVALID_SOCKET)
    {
        _AMIRLOG(AmirLib1LOG, _FATAL, "in client thread we reach a unset socket: ")<< WSAGetLastError());
        client_connections[sn].issocketactive = false;
        aliveclientthreads--;
        return;
    }*/
    srvrevnt->onCreateClientSocket(sn);

    char buff[buff_len];
    int number_of_bytes_received;
    while ((number_of_bytes_received = recv(client_connections[sn].skt, buff, buff_len-1, 0))>0 && client_connections[sn].issocketactive)// != SOCKET_ERROR)
    {
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString(number_of_bytes_received)<<" Bytes received in connection: "<<sn);
        srvrevnt->onReciveData(sn, buff, number_of_bytes_received);
    }
    if (client_connections[sn].issocketactive)
    {
        if (number_of_bytes_received != 0)
        {
            int error = WSAGetLastError();
            switch (error)
            {
            case WSAEFAULT: // https://stackoverflow.com/questions/25194542/c-wsaefault-error-sending-integer
                _AMIRLOG(AmirLib1LOG, _INFO, AmirString(error)<< ": Bad address");
                break;
            case WSAECONNRESET:
                _AMIRLOG(AmirLib1LOG, _INFO, AmirString(error)<< ": Connection reset by peer");
                break;
            default:
                _AMIRLOG(AmirLib1LOG, _INFO, AmirString(error)<<": connection closed");
                break;
            }
        }
        //shutting down the client connection
        /*number_of_bytes_received = shutdown(client_connections[sn].skt, SD_SEND);
        if (number_of_bytes_received == SOCKET_ERROR)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("shutting down! failed with error: ")<< WSAGetLastError());
        }*/
        client_connections[sn].issocketactive = false;
        closesocket(client_connections[sn].skt);
        srvrevnt->onCloseClientSocket(sn);
    }
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("Client thread ID ")<<connectionID<<" closed!");
    client_connections[sn].isthreadactive = false;
    /* */
    aliveclientthreads--;
    unique_lock<mutex> lck(mtx);
    cv.notify_all();
    /* */
}
//===============================================================
int AmirServerSocket::getUnActiveClientID()
{
    bool found_un_active_client_socket = false;
    int rstl;
    for (unsigned int z01 = 0; z01 < client_connections.size(); z01++)
        if (!client_connections[z01].isthreadactive)
        {
            found_un_active_client_socket = true;
            rstl = z01;
            break;
        }
    if (found_un_active_client_socket)
    {
        for (int z01 = client_connections.size()-1; z01 > rstl; z01--)
            if(!client_connections[z01].isthreadactive) client_connections.pop_back();
            else break;
        return rstl;
    }
    ClientConnection cc;
    //delete these two comment
    //cc.isthreadactive = false;
    //cc.issocketactive = false;
    client_connections.push_back(cc);
    return (client_connections.size()-1);
}
//===============================================================
void AmirServerSocket::waitUntilServerSocketClosed()
{
    unique_lock<mutex> lck(mtx);
    while(is_threads_in_use || aliveclientthreads>0)//if (is_threads_in_use)
        cv.wait(lck);
}
//===============================================================
bool AmirServerSocket::isPortOpen(unsigned int port)
{
    struct sockaddr_in addr;
    SOCKET tsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tsocket>0)
    {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        int z01 = connect(tsocket, (struct sockaddr *)&addr, sizeof(addr));
        bool isopen = (z01 >= 0);
        closesocket(tsocket);
        return isopen;
    }
    else return false;
}
//===============================================================
AmirServerSocket::~AmirServerSocket()
{
    //close all sockets
    closeServerConnection();
    waitUntilServerSocketClosed();
    //The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
    /*if (isWsaUsedBefore == notUsedBefore)
    {
        WSAuseres--;
        if (!WSAuseres)
        {
            WSACleanup();
            _AMIRLOG(AmirLib1LOG, _INFO, "WSA Cleanuped.");
            isWsaUsedBefore = notChecked;
        }
    }*/
}
//===============================================================
//===============================================================
AmirClientSocket::AmirClientSocket()
{
    if (isWsaUsedBefore == notChecked)
    {
        _AMIRLOG(AmirLib1LOG, _INFO, "Checking Winsock DLL Initialization ... ");
        if (isWinsockInitialized())
        {
            _AMIRLOG(AmirLib1LOG, _INFO, "Winsock DLL Initialized Before.");
            isWsaUsedBefore = UsedBefore;
        }
        else
        {
            _AMIRLOG(AmirLib1LOG, _INFO, "Winsock DLL not Initialized Before.");
            isWsaUsedBefore = notUsedBefore;
        }
    }
    if (isWsaUsedBefore == notUsedBefore)
    {
        if (!WSAuseres)
        {
            _AMIRLOG(AmirLib1LOG, _INFO, "Initializing Winsock DLL for process ... ");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("Failed. Error Code : ")<<WSAGetLastError());
                throw AmirException("Fail to Initializing WSA.");
            }
            _AMIRLOG(AmirLib1LOG, _INFO, "Initialized.");
        }
        WSAuseres++;
    }
}
//===============================================================
bool AmirClientSocket::isStarted()
{
    return (isinitialized && is_thread_in_use);
}
//===============================================================
AmirClientSocket::AmirClientSocket(string host, unsigned short port, ClientEvent &clientevnt):AmirClientSocket()
{
    setPortandEventClass(host, port, clientevnt);
}
//===============================================================
void AmirClientSocket::setPortandEventClass(string host, unsigned short port, ClientEvent &clientevnt)
{
    this->clientevnt = &clientevnt;
    this->host = host;
    this->port = port;
    isinitialized = true;
}
//===============================================================
void AmirClientSocket::start()
{
    if(isinitialized)
    {
        if (is_thread_in_use) return;
        is_thread_in_use = true;
        struct sockaddr_in addr;
        _AMIRLOG(AmirLib1LOG, _INFO, "Creating client socket...");
        if ((ssocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("Could not create socket : ")<< WSAGetLastError());
        }
        _AMIRLOG(AmirLib1LOG, _INFO, "Socket created.");

        addr.sin_addr.s_addr = inet_addr(host.c_str());
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        //Connect to remote server
        if (connect(ssocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("Error connecting ")<<host<<':'<<port);
            closesocket(ssocket);
            isinitialized = false;
            is_thread_in_use = false;
            throw AmirException(AmirString("Client connecting: ")<<host<<':'<<port<<" failed!");
        }
        issocketactive = true;
        _AMIRLOG(AmirLib1LOG, _INFO, "Connected");
        thread th01(ServerThread, this);
        th01.detach();
        //char *message = "GET / HTTP/1.1\r\n\r\n";
    }
    else throw AmirException("client socket starting failed Port and Client Event Class not set!");
}
//===============================================================
void AmirClientSocket::closeConnection()
{
    if(issocketactive)
    {
        issocketactive = false;
        closesocket(ssocket);
        _AMIRLOG(AmirLib1LOG, _INFO, "Socket closed.");
    }
}
//===============================================================
void AmirClientSocket::sendToServer(char* buff, unsigned int bufflen)
{
    if (issocketactive)
        if (send(ssocket, buff, bufflen, 0) <= 0)
            _AMIRLOG(AmirLib1LOG, _IMPORTANT, "Send failed");
}
//===============================================================
void AmirClientSocket::ServerThread()
{
    int number_of_bytes_received;
    char buff[buff_len];
    while ((number_of_bytes_received = recv(ssocket, buff, buff_len-1, 0)) > 0 && issocketactive)// != SOCKET_ERROR)
    {
        clientevnt->onReciveData(buff,number_of_bytes_received);
    }
    if (issocketactive){
        issocketactive = false;
        _AMIRLOG(AmirLib1LOG, _IMPORTANT, AmirString("Server connection closed. Error Code : ")<<WSAGetLastError());
        clientevnt->onCloseServerSocket();
    }
    isinitialized = false;
    //closeConnection();
    is_thread_in_use = false;
    unique_lock<mutex> lck(mtx);
    cv.notify_all();
}
//===============================================================
void AmirClientSocket::waitUntilClientSocketClosed()
{
    unique_lock<mutex> lck(mtx);
    while(is_thread_in_use) cv.wait(lck);
}
//===============================================================
AmirClientSocket::~AmirClientSocket()
{
    closeConnection();
    waitUntilClientSocketClosed();
    //The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
    /*if (isWsaUsedBefore == notUsedBefore)
    {
        WSAuseres--;
        if (!WSAuseres)
        {
            WSACleanup();
            _AMIRLOG(AmirLib1LOG, _INFO, "WSA Cleaning up.");
            isWsaUsedBefore = notChecked;
        }
    }*/
}
















