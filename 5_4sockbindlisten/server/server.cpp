#include<iostream>
#include<sys/socket.h>
#include<sys/signal.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string>
#include<assert.h>
using namespace std;

static bool stop = false;

static void handle_term(int sig)
{
    stop = true;
}

int main(int argc,char* argv[])
{
    signal(SIGTERM,handle_term);

    if(argc<3)
    {
        cout<<"usage:" +string(argv[0])+" ip_address port_num backlog"<<endl;
        return 1;
    }

    const char* ip =argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    sockaddr_in sockAddr;
    sockAddr.sin_family=AF_INET;
    sockAddr.sin_port=htons(port);
    inet_aton(ip,&sockAddr.sin_addr);
    // inet_pton(AF_INET,ip,&sockAddr.sin_addr);


    auto sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    int ret=bind(sock,(sockaddr*)&sockAddr,sizeof(sockAddr));
    cout<<errno<<endl;

    assert(ret!=-1);

    ret = listen(sock,backlog);
    assert(ret!=-1);

    sleep(20);
    sockaddr_in client;
    socklen_t client_addr_len=sizeof(sockaddr_in);
    int clientfd = accept(sock,(sockaddr*)&client,&client_addr_len);
    if(clientfd<0)
        cout<<"errno is "<<errno<<endl;
    else
    {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&client.sin_addr,client_ip,INET_ADDRSTRLEN);
        int client_port = ntohs(client.sin_port);
        cout<<"ip:"<<string(client_ip)<<" port:"<<client_port<<endl;
        close(clientfd);
    }
    close(sock);
    return 0;
    // while(!stop)
    // {
    //     sleep(1);
    // }
    // close(sock);
    return  0;


}