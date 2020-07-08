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

    if(connect(sock,(const sockaddr*)&address,sizeof(address))<0)
    {
        cout<<"connection failed"<<endl;
    }
    else
    {
        send(sock,"abc",3,0);
        send(sock,"123",3,MSG_OOB);
        send(sock,"abc",3,0);
    }

    close(sock);
    return 0;
    


}