#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/uio.h>
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
        char header_buf[BUFF_SIZE];
        memset(header_buf,'\0',BUFF_SIZE);

        char* file_buf;
        struct stat file_stat;
        bool vaild=true;
        int len=0;
        if(stat(file_name,&file_stat)<0)    //目标文件不存在
        {
            vaild=false;
        }
        else
        {
            if(S_ISDIR(file_stat.st_mode))  //目标文件是一个目录
            {
                vaild=false;
            }
            else if(file_stat.st_mode&S_IROTH)  //当前用户有读取权限
            {
                int fd = open(file_name,O_RDONLY); //仅读打开
                file_buf = new char[file_stat.st_size+1];
                memset(file_buf,'\0',file_stat.st_size+1);
                if(read(fd,file_buf,file_stat.st_size)<0)
                {
                    vaild =false;
                }
            }
            else
            {
                vaild=false;
            }
            
        }
        if(vaild)
        {
            ret = snprintf(header_buf,BUFF_SIZE-1,"%s %s\r\n","HTTP/1.1",status_line[0]);
            len+=ret;
            ret=snprintf(header_buf+len,BUFF_SIZE-1-len,"Content-Length: %d\r\n",file_stat.st_size);
            len+=ret;
            ret=snprintf(header_buf+len,BUFF_SIZE-1-len,"%s","\r\n");

            struct iovec iv[2];
            iv[0].iov_base=header_buf;
            iv[0].iov_len=strlen(header_buf);
            iv[1].iov_base=file_buf;
            iv[1].iov_len=file_stat.st_size;
            ret=writev(connfd,iv,2); //通过writev 直接发送多个文件
        }
        else
        {
            ret = snprintf(header_buf,BUFF_SIZE-1,"%s %s\r\n","HTTP/1.1",status_line[1]);
            len+=ret;
            ret = snprintf(header_buf+len,BUFF_SIZE-1-len,"%s","\r\n");
            send(connfd,header_buf,strlen(header_buf),0);
        }
        close(connfd);
        delete[] file_buf;
        
        
    }
    close(sock);
    return 0;
}


