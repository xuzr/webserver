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
    if(argc<=3)
    {
        cout<<"usage : "<<string(argv[0])<<"ip_address port_num filename"<<endl;
        return 1;
    }

    const char* ip =argv[1];
    int port =atoi(argv[2]);
    const char* file_name=argv[3];

    int filefd = open(file_name,O_RDONLY);
    assert(filefd>0);
    struct stat file_stat;
    fstat(filefd,&file_stat);

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
        //ssize_t sendfile(int out_fd,int in_fd,off_t* offset,size_t count)
        //out_fd 必须为socket
        //in_fd 必须支持mmap
        sendfile(connfd,filefd,nullptr,file_stat.st_size);   
        close(connfd);
        close(filefd);
    }
    close(sock);
    return 0;
}


