#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <csignal>

#define LOCALHOST "127.0.0.1"
#define CONNECTING_PORT 9191

using std::cout;
using std::endl;
using std::cerr;

void receiveMessage(int socket_fd, char* buffer)
{
    while(true)
    {
        ssize_t amountReceived = recv(socket_fd, buffer, 1024, 0);
        //recv(socket_fd, buffer, 1024, 0);
        if (amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            cout << buffer << endl;
        }
        else if (amountReceived == 0)
        {
            cout << "Connection Closed" << endl;
            break;
        }
        else
        {
            cerr << "Error Receiving Message, Error: " << errno << endl;
            break;
        }
    }
    close(socket_fd);
}


int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    struct sockaddr_in addr{};


    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONNECTING_PORT);
    inet_pton(AF_INET, LOCALHOST, &addr.sin_addr);

    int connectResult = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));

    if(connectResult == 0) cout << "Connection Successful" << endl;
    else
    {
        cerr << "Connection Failed, Error: " << errno << endl;
        return -1;
    }

    char buffer[1024] {};

    while (true)
    {
        std::thread receiveThread(receiveMessage, socket_fd, buffer);
        receiveThread.detach();
        //recv(socket_fd, buffer,1024,0);
        //cout << buffer << endl;
        std::cin.getline(buffer, 1024);
        if (strcmp(buffer, "exit") == 0) break;

        ssize_t bytesSent = send(socket_fd, buffer, 1024, 0);

        if(bytesSent == -1)
        {
            cerr << "Message Sending Failed, Error: " << errno << endl;
            return -1;
        }
        else cout << "Message Sent" << endl;
        //recv(socket_fd, buffer,1024,0);
    }


    return 0;
}