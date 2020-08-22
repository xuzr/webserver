#define _GNU_SOURCE 1
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<poll.h>
#include<iostream>

using namespace std;

constexpr int USER_LIMIT = 5;
constexpr int BUFFER_SIZE = 64;
constexpr int FD_LIMIT = 65535;

struct client_data{
    sockaddr_in address;
    char *write_buf;
    char buf[BUFFER_SIZE];
};

int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char*argv[])
{
    if(argc<=2)
    {
        cout << "Usage: " << argv[0] << " ip_address, port_number" << endl;
        return 1;

    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != 1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    client_data *users = new client_data[FD_LIMIT];
    pollfd fds[USER_LIMIT + 1];
    int user_counter = 0;
    for (int i = 1; i <= USER_LIMIT;++i)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }

    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while(1)
    {
        ret = poll(fds, user_counter + 1, -1);
        if(ret<0)
        {
            cout << "poll failure" << endl;
            break;
        }

        for (int i = 0; i < user_counter + 1;++i)
        {
            if((fds[i].fd==listenfd)&&(fds[i].revents&POLLIN))
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if(connfd<0)
                {
                    cout << "errno is :" << errno << endl;
                    continue;
                }

                if(user_counter>=USER_LIMIT)
                {
                    const char *info = "too many users\n";
                    cout << string(info) << endl;
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                user_counter++;
                users[connfd].address = client_address;
                setNonBlocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                cout << "comes a new user, now have " << user_counter << " users" << endl;
            }
            else if(fds[i].revents&POLLERR)
            {
                cout << "get an error from " << fds[i].fd << endl;
                char errors[100];
                memset(errors,'\0',100);
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd,SOL_SOCKET,SO_ERROR,&errors,&length)<0)
                {
                    cout << "get socket option failed" << endl;
                }
                continue;
            }
            else if(fds[i].revents&POLLRDHUP)
            {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                i--;
                user_counter--;
                cout << "a client left" << endl;
            }
            else if(fds[i].revents&POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                cout << "get " << ret << " bytes of client data " << string(users[connfd].buf) << " from " << connfd << endl;
                if(ret<0)
                {
                    if(errno!=EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if(ret==0)
                {

                }
                else
                {
                    for (int j = 1; j <= user_counter;j++)
                    {
                        if(fds[j].fd==connfd)
                            continue;
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;

                    }
                }
            }
            else if(fds[i].revents&POLLOUT)
            {
                int connfd = fds[i].fd;
                if(!users[connfd].write_buf)
                    continue;
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = nullptr;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete [] users;
    close(listenfd);
    return 0;
}
