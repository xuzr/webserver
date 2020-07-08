#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdlib.h>
#include<string>
#include<errno.h>
#include<iostream>
#include<unistd.h>
using namespace std;


int main(int argc, char * argv[])
{
    if(argc<=2)
    {
        cout<<"usage : "<<string(argv[0])<<"ip_address port_num "<<endl;
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
        close(STDOUT_FILENO);
        dup(connfd);
        string tmp;
        while(cin>>tmp)
        {
            cout<<tmp<<endl;
        }
        close(connfd);
    }
    close(sock);
    return 0;
    

}