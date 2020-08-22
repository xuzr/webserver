#define _GUN_SOURCE 1
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<poll.h>
#include<fcntl.h>
#include<iostream>

using namespace std;

constexpr int BUFFER_SIZE = 64;

int main(int argc, char *argv[])
{
    if(argc<=2)
    {
        cout << "Usage: " << argv[0] << " ip_address port_number" << endl;
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_address.sin_addr);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    if(connect(sockfd,(struct sockaddr*)&server_address,sizeof(server_address))<0)
    {
        cout << "connection failed" << endl;
        close(sockfd);
        return 1;
    }

    pollfd fds[2];

    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

    while(1)
    {
        ret = poll(fds, 2, -1);
        if(ret<0)
        {
            cout << "poll failure" << endl;
            break;
        }

        if(fds[1].revents&POLLRDHUP)
        {
            cout << "server close the connection" << endl;
            break;
        }
        else if(fds[1].revents*POLLIN)
        {
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            cout << string(read_buf) << endl;
        }

        if(fds[0].revents&POLLIN)
        {
            ret = splice(0, nullptr, pipefd[1], nullptr, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0], nullptr, sockfd, nullptr, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }
    close(sockfd);
    return 0;
}