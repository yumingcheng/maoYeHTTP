#ifndef _MYE_HTTP_UTIL_H_
#define _MYE_HTTP_UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdarg.h>

#define FILENAME_LEN  200
#define READ_BUFFER_SIZE  4096
#define WRITE_BUFFER_SIZE  1024

#define LISTEN_QUEUE_NUM 65536

#ifdef __cplusplus
extern "C"
{
#endif

    enum CONNECT_CODE{ OK = 0, NOTHING = 1, IOERR = -1, CLOSED = -2, BUFFER_FULL = -3, BUFFER_EMPTY = -4, TRY_AGAIN=-5 };
    /*http请求的方法,暂时支持GET和POST请求*/
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };
    /*主状态机的三种可能状态,分别表示:当前正在分析请求行，当前正在分析请求头部,正在分析请求内容*/
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    /*服务器http请求的结果:NO_REQUEST 表示请求不完整,需要继续读取客户数据:GET_REQUEST表示获得一个完整的客户请求：BAD_REQUEST表示客户端请求有语法错误：FORBIDDEN_REQUEST表示客户对资源没有足够的访问权限:INTERNAL_ERROR表示服务器内部错误：CLOSED_CONNECTION表示客户端已经关闭链接了*/
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    /*从状态机的三种可能状态,即行的读取状态,分别表示：读取到一个完整的行,行出错和行数据尚且不完整*/
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };


    typedef struct _myeHttpConnect{

        int m_sockfd;
        struct sockaddr_in m_address;

        char m_read_buf[ READ_BUFFER_SIZE ];
        int m_read_idx;
        int m_checked_idx;
        int m_start_line;
        char m_write_buf[ WRITE_BUFFER_SIZE ];
        int m_write_idx;

        enum CHECK_STATE m_check_state;
        enum METHOD m_method;

        char m_real_file[ FILENAME_LEN ];
        char* m_url;
        char* m_version;
        char* m_host;
        int m_content_length;
        int m_linger;

        int m_file_flag;
        char* m_file_address;
        int m_file_length;
        struct stat m_file_stat;

        struct iovec m_iv[2];
        int m_iv_count;

        void* m_application;

    }myeHttpConnect;




#ifdef __cplusplus
}
#endif

#endif 
