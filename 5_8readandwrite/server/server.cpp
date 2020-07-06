#include<iostream>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<memory.h>

using namespace std;

constexpr int BUF_SIZE = 1024;

int main(int argc,char * argv[])
{
    if(argc<=2)
    {
        cout<<"usage: "<<string(argv[0])<< " ip_address port_number"<<endl;
        return 1;
    }

    //get ip and port
    const char* ip = argv[1];
    int port =atoi(argv[2]);

    //convert ip and port into sockaddr_in
    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);

    //get sockfd
    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    //bind port to fd
    int server = bind(sock,(const sockaddr*)&address,sizeof(address));
    assert(server!=-1);

    //start listen
    server = listen(sock,5);
    assert(server!=-1);

    //get client
    sockaddr_in client_addr;
    socklen_t client_addrlen=sizeof(client_addr);
    int client = accept(sock,(sockaddr*)&client_addr,&client_addrlen);
    if(client<0)
        cout<<"errno is :"<<errno<<endl;
    else
    {
        char buffer[BUF_SIZE];

        memset(buffer,'\0',BUF_SIZE);
        int rec = recv(client,buffer,BUF_SIZE-1,0);
        cout<< "got" << rec<<"bytes of normal data: "<<string(buffer)<<endl;

        memset(buffer,'\0',BUF_SIZE);
        rec = recv(client,buffer,BUF_SIZE-1,MSG_OOB);
        cout<< "got" << rec<<"bytes of obb data: "<<string(buffer)<<endl;

        memset(buffer,'\0',BUF_SIZE);
        rec = recv(client,buffer,BUF_SIZE-1,0);
        cout<< "got" << rec<<"bytes of normal data: "<<string(buffer)<<endl;

        close(client);

    }

    close(sock);
    return 0;

}