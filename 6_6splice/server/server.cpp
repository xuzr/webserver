#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/uio.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<cassert>
#include<stdlib.h>
#include<string>
#include<errno.h>
#include<iostream>
#include<unistd.h>
#include<memory.h>
using namespace std;

constexpr int BUFF_SIZE=1024;
static const char* status_line[2]={"200 OK","500 Internal server error"};

int main(int argc, char * argv[])
{
    if(argc<=2)
    {
        cout<<"usage : "<<string(argv[0])<<"ip_address port_num"<<endl;
        return 1;
    }

    const char* ip =argv[1];
    int port =atoi(argv[2]);

    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);

    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    int ret = bind(sock,(sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret = listen(sock,5);
    assert(ret!=-1);

    sockaddr_in client;
    socklen_t client_addr_len=sizeof(client);
    int connfd = accept(sock,(sockaddr*)&client,&client_addr_len);

    if(connfd<0)
    {
        cout<<"connection fail!"<<endl;
    }
    else
    {
        int pipefd[2];
        assert(ret!=-1);
        ret = pipe(pipefd);
        ret = splice(connfd,nullptr,pipefd[1],nullptr,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);

        ret = splice(pipefd[0],nullptr,connfd,nullptr,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);
        close(connfd);

    }
    close(sock);
    return 0;
}


