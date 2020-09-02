#define _GNU_SOURCE 
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<iostream>
#include<sys/epoll.h>
#include<signal.h>
#include"sortTime.h"

constexpr int FD_LIMIT = 65535;
constexpr int MAX_EVENT_NUMBER = 1024;
constexpr int TIMESLOT = 5;

static int pipefd[2];
static sort_timer_lst timer_lst;
static int epollfd = 0;

using namespace std;


int setNonBlocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

int addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setNonBlocking(fd);
}

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void  addsig(int sig)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void timer_handler()
{
    timer_lst.tick();
    alarm(TIMESLOT);
}

void cb_func(client_data* user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    cout << "close fd: " << user_data->sockfd << endl;
}



int main(int argc,char* argv[])
{
    if(argc<=2)
    {
        cout << "Usage: " << argv[0] << " ip_address port_number" << endl;
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    
}