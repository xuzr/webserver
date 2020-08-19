#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<string>
#include<fcntl.h>
#include<iostream>

using namespace std;

//缓冲区大小
constexpr int BUFFER_SIZE=4096;

//主状态机：正在分析请求行，正在分析头部字段
enum CHECK_STATE
{
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER
};

//从状态机：读取到一个完整的行，行出错，行数据尚不完整
enum LINE_STATUS
{
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};

//HTTP请求处理结果
enum HTTP_CODE
{
    NO_REQUEST,
    GET_REQUET,
    BAD_REQUEST,
    FORBIDDEN_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};

static const char *szret[] = {"I get a correct result\n", "Somethin wrong\n"};

//从转态机，用于解析出一行内容
LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index)
{
    char tmp;

    for (; checked_index < read_index;++checked_index)
    {
        tmp = buffer[checked_index];

        if(tmp=='\r')
        {
            if((checked_index+1)==read_index)
            {
                return LINE_OPEN;
            }
            else if(buffer[checked_index+1]=='\n')
            {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(tmp=='\n')
        {
            if((checked_index>1)&&buffer[checked_index-1]=='r')
            {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
        return LINE_BAD;
        }
    }
    return LINE_OPEN;
}


//分析请求行
HTTP_CODE parse_requestline(char* tmp, CHECK_STATE &checkstate)
{
    char *url = strpbrk(tmp, " \t");
    if(!url)
    {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    char *method = tmp;
    if(strcasecmp(method,"GET")==0)
    {
        cout << "The request method is GET" << endl;
    }
    else
    {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if(!version)
    {
        return BAD_REQUEST;
    }

    *version++ = '\0';
    version += strspn(version, " \t");
    if(strcasecmp(version,"HTTP/1.1")!=0)
    {
        return BAD_REQUEST;
    }

    if(strncasecmp(url,"http://",7)==0)
    {
        url += 7;
        url = strchr(url, '/');
    }

    if(!url||url[0]!='/')
    {
        return BAD_REQUEST;
    }

    cout << "The request URL is: " << url << endl;

    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}


HTTP_CODE parse_headers(char *tmp)
{
    if(tmp[0]=='\0')
        return GET_REQUET;
    else if(strncasecmp(tmp,"Host:",5)==0)
    {
        tmp += 5;
        tmp += strspn(tmp, " \t");
        cout << "the request host is: " << tmp << endl;
    }
    else
    {
        cout << "I can  not  handle this header" << endl;
    }
    return NO_REQUEST;
}

HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE &checkstate, int &read_index, int &start_line)
{
    LINE_STATUS linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;

    while((linestatus = parse_line(buffer,checked_index,read_index))==LINE_OK)
    {
        char *tmp = buffer + start_line;
        start_line = checked_index;
        switch(checkstate)
        {
            case CHECK_STATE_REQUESTLINE:
            {
                retcode = parse_requestline(tmp, checkstate);
                if(retcode==BAD_REQUEST)
                {
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:
            {
                retcode = parse_headers(tmp);
                if(retcode==BAD_REQUEST)
                {
                    return BAD_REQUEST;
                }
                else if(retcode==GET_REQUET)
                {
                    return GET_REQUET;
                }
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
    if(linestatus==LINE_OPEN)
    {
        return NO_REQUEST;
    }
    else
    {
        return BAD_REQUEST;
    }
}

int main(int argc, char* argv[])
{
    if(argc<=2)
    {
        cout << "Usage : " << argv[0] << " ip_address port_number" << endl;
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    // inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int fd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
    if(fd<0)
    {
        cout << "errno is :" << errno << endl;
    }
    else
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        int data_read = 0, read_index = 0, checked_index = 0, start_line = 0;
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        while(1)
        {
            data_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
            if(data_read==-1)
            {
                cout << "reading failed!" << endl;
                break;
            }
            else if(data_read==0)
            {
                cout << "remote client has closed the connection" << endl;
                break;
            }

            read_index += data_read;

            HTTP_CODE result = parse_content(buffer, checked_index, checkstate, read_index, start_line);
            if(result==NO_REQUEST)
            {
                continue;
            }
            else if(result ==GET_REQUET)
            {
                send(fd, szret[0], strlen(szret[0]), 0);
                break;
            }
            else
            {
                send(fd, szret[1], strlen(szret[1]), 0);
                break;
            }
        }
        close(fd);
    }
    close(listenfd);
    return 0;
}