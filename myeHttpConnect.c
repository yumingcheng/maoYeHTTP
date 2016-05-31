#include "myeHttpConnect.h"

const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";


static enum LINE_STATUS parse_line(myeHttpConnect* me);
static enum HTTP_CODE parse_request_line(myeHttpConnect* me,char* text );
static enum HTTP_CODE parse_headers( myeHttpConnect* me,char* text );
static enum HTTP_CODE parse_content( myeHttpConnect* me, char* text );
static int add_content(myeHttpConnect* me, const char* content );
static int add_blank_line(myeHttpConnect* me);
static int add_linger(myeHttpConnect* me);
static int add_content_length(myeHttpConnect* me, int content_len );
static int add_headers(myeHttpConnect* me,int content_len);
static int add_status_line(myeHttpConnect* me, int status, const char* title );
static int add_response( myeHttpConnect* me,const char* format, ... );
static void unmap(myeHttpConnect* me);
static void connect_memset(myeHttpConnect* me);

myeHttpConnect* myeHttpConnect_new(myeApplication* application)
{
    myeHttpConnect* me = (myeHttpConnect*)calloc(1,sizeof(myeHttpConnect));
    me->m_application = application;
    return me;
}

myeHttpConnect* myeHttpConnect_new_array(myeApplication* application,int count)
{
    myeHttpConnect* me_array = (myeHttpConnect*)calloc(count,sizeof(myeHttpConnect));
    int i = 0 ;
    for(i = 0 ; i < count;i++)
    {
        (me_array+i)->m_application = application;
    }
    return me_array;
}

int myeHttpConnect_connect_init(myeHttpConnect* me,int sockfd, const struct sockaddr_in addr)
{
    connect_memset(me);
    me->m_sockfd = sockfd;
    me->m_address = addr;
    return 0;
}

enum CONNECT_CODE myeHttpConnect_connect_read(myeHttpConnect* me)
{

    int bytes_read = 0;
    while(1)
    {
        if( me->m_read_idx  >= READ_BUFFER_SIZE )
        {
            return BUFFER_FULL;
        }
        bytes_read = recv(me->m_sockfd,me->m_read_buf + me->m_read_idx,READ_BUFFER_SIZE - me->m_read_idx,0);
        if ( bytes_read == -1 )
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK )
            {
                break;
            }
            return IOERR;
        }
        else if ( bytes_read == 0 )
        {
            return CLOSED;
        }
        me->m_read_idx += bytes_read;
    }
    printf("(%s)[%d]\n",me->m_read_buf,me->m_read_idx);
    return OK;
}

enum CONNECT_CODE myeHttpConnect_connect_write(myeHttpConnect* me)
{
    printf("%s() BEGIN \n",__func__);
    int temp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = me->m_write_idx + me->m_file_length;
    if ( bytes_to_send == 0 )
    {
        connect_memset(me);
        return BUFFER_EMPTY;
        //        modfd( me->m_epollfd, sockfd, EPOLLIN );
    }

    while( 1 )
    {
        printf("bytes_to_send = [%d] bytes_to_send = [%d]\n",bytes_have_send,bytes_to_send);
        if(bytes_have_send == bytes_to_send) 
        {
            unmap(me);
            connect_memset(me);
            if( me->m_linger)
            {
                return BUFFER_EMPTY;
            }
            else
            {
                return BUFFER_EMPTY;
            }
        }

        temp = writev( me->m_sockfd, me->m_iv, me->m_iv_count );
        if ( temp <= -1 )
        {
            if( errno == EAGAIN )
            {
                //              modfd( me->m_epollfd, sockfd, EPOLLOUT ); 
                return TRY_AGAIN;
            }
            unmap(me);
            return CLOSED;
        }
        bytes_have_send += temp;
    }
    printf("%s() END \n",__func__);
    return CLOSED;
}

enum HTTP_CODE myeHttpConnect_process(myeHttpConnect* me)
{
    printf("myeHttpConnect_process() \n");
    enum HTTP_CODE code = myeHttpConnect_process_read(me);
    if(code == NO_REQUEST)
    {
        printf("NO_REQUEST\n");
        return NO_REQUEST;
    }
    printf("OK！！！！！\n");
    

    int write_ret = myeHttpConnect_process_write(me,code);

    if( !write_ret )
    {
        return CLOSED_CONNECTION;
    }
    return GET_REQUEST;
}

enum HTTP_CODE myeHttpConnect_process_read(myeHttpConnect* me)
{
    printf("%s()\n",__func__);
    enum LINE_STATUS line_status = LINE_OK;
    enum HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while ( ( ( me->m_check_state == CHECK_STATE_CONTENT ) && ( line_status == LINE_OK  ) )
            || ( ( line_status = parse_line(me) ) == LINE_OK ) )
    {
        text =  me->m_read_buf + me->m_start_line;
        me->m_start_line = me->m_checked_idx;

        switch ( me->m_check_state )
        {
        case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line( me,text );
                if ( ret == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                break;
            }
        case CHECK_STATE_HEADER:
            {
                ret = parse_headers( me,text );
                if ( ret == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                else if ( ret == GET_REQUEST )
                {
                    return myeApplication_process((myeApplication*)(me->m_application),me);
                }
                break;
            }
        case CHECK_STATE_CONTENT:
            {
                ret = parse_content( me,text );
                if ( ret == GET_REQUEST )
                {
                    return myeApplication_process((myeApplication*)(me->m_application),me);
                }
                line_status = LINE_OPEN;
                break;
            }
        default:
            {
                return INTERNAL_ERROR;
            }
        }
    }

    return NO_REQUEST;
}


int myeHttpConnect_process_write(myeHttpConnect* me,enum HTTP_CODE ret)
{
    printf("%s()\n",__func__);
    switch ( ret )
    {
        add_status_line(me, 500, error_500_title );
    case INTERNAL_ERROR:
        {
            add_status_line(me, 500, error_500_title );
            add_headers( me,strlen( error_500_form ) );
            if ( ! add_content(me, error_500_form ) )
            {
                return 0;
            }
            break;
        }
    case BAD_REQUEST:
        {
            add_status_line(me, 400, error_400_title );
            add_headers( me,strlen( error_400_form ) );
            if ( ! add_content(me, error_400_form ) )
            {
                return 0;
            }
            break;
        }
    case NO_RESOURCE:
        {
            add_status_line(me, 404, error_404_title );
            add_headers( me,strlen( error_404_form ) );
            if ( ! add_content(me, error_404_form ) )
            {
                return 0;
            }
            break;
        }
    case FORBIDDEN_REQUEST:
        {
            add_status_line(me, 403, error_403_title );
            add_headers( me,strlen( error_403_form ) );
            if ( ! add_content(me, error_403_form ) )
            {
                return 0;
            }
            break;
        }            
    case FILE_REQUEST:
        {
            add_status_line(me, 200, ok_200_title );
            if ( me-> m_file_stat.st_size != 0 )
            {
                add_headers( me,me-> m_file_length);
                me->m_iv[ 0 ].iov_base = me->m_write_buf;
                me->m_iv[ 0 ].iov_len = me->m_write_idx;
                me->m_iv[ 1 ].iov_base = me->m_file_address;
                me->m_iv[ 1 ].iov_len = me-> m_file_length;
                me->m_iv_count = 2;
                return 1;
            }
            else
            {
                const char* ok_string = "<html><body></body></html>";
                add_headers( me,strlen( ok_string ) );
                if ( ! add_content(me, ok_string ) )
                {
                    return 0;
                }
            }
        }
    default:
        {
            return 0;
        }
    }

    me->m_iv[ 0 ].iov_base = me->m_write_buf;
    me->m_iv[ 0 ].iov_len = me->m_write_idx;
    me->m_iv_count = 1;
    return 0;
}

int myeHttpConnect_delete(myeHttpConnect* me)
{
    if(me)
        free(me);
    return 0;
}
int myeHttpConnect_delete_array(myeHttpConnect* me)
{
    if(me)
        free(me);
    return 0;
}

static void connect_memset(myeHttpConnect* me)
{

    memset( me->m_read_buf, '\0', READ_BUFFER_SIZE );
    me->m_read_idx = 0;
    me->m_checked_idx = 0;
    me->m_start_line = 0;
    memset( me->m_write_buf, '\0', WRITE_BUFFER_SIZE );
    me->m_write_idx = 0;
    
    me->m_check_state = CHECK_STATE_REQUESTLINE;
    me->m_method = GET;

    memset( me->m_real_file, '\0', FILENAME_LEN );
    me->m_url = NULL;
    me->m_version = NULL;
    me->m_host = NULL;
    me->m_content_length = 0;
    me->m_linger = 0;

    me->m_file_flag = 0;
    me->m_file_address = NULL;
    me->m_file_length = 0;
}

static enum LINE_STATUS parse_line(myeHttpConnect* me)
{
    char temp;
    for ( ; me->m_checked_idx < me->m_read_idx; me->m_checked_idx++ )
    {
        temp = me->m_read_buf[ me->m_checked_idx ];
        if ( temp == '\r' )
        {
            if ( ( me->m_checked_idx + 1 ) == me->m_read_idx )
            {
                return LINE_OPEN;
            }
            else if ( me->m_read_buf[ me->m_checked_idx + 1 ] == '\n' )
            {
                me->m_read_buf[ me->m_checked_idx++ ] = '\0';
                me->m_read_buf[ me->m_checked_idx++ ] = '\0';
                return LINE_OK;
            }

            return LINE_BAD;
        }
        else if( temp == '\n' )
        {
            if( ( me->m_checked_idx > 1 ) && ( me->m_read_buf[ me->m_checked_idx - 1 ] == '\r' ) )
            {
                me->m_read_buf[ me->m_checked_idx-1 ] = '\0';
                me->m_read_buf[ me->m_checked_idx++ ] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

static enum HTTP_CODE parse_request_line(myeHttpConnect* me,char* text )
{
    me->m_url = strpbrk( text, " \t" );
    if ( ! me->m_url )
    {
        return BAD_REQUEST;
    }
    *me->m_url++ = '\0';

    char* method = text;
    if ( strcasecmp( method, "GET" ) == 0 )
    {
        me->m_method = GET;
    }
    else if(strcasecmp( method, "POST" ) == 0)
    {
        me->m_method = POST;
    }
    else
    {
        return BAD_REQUEST;
    }

    me->m_url += strspn( me->m_url, " \t" );
    me->m_version = strpbrk( me->m_url, " \t" );
    if ( ! me->m_version )
    {
        return BAD_REQUEST;
    }
    *(me->m_version)++ = '\0';
    me->m_version += strspn( me->m_version, " \t" );
    if ( strcasecmp( me->m_version, "HTTP/1.1" ) != 0 )
    {
        return BAD_REQUEST;
    }

    if ( strncasecmp( me->m_url, "http://", 7 ) == 0 )
    {
        me->m_url += 7;
        me->m_url = strchr( me->m_url, '/' );
    }

    if ( ! me->m_url || me->m_url[ 0 ] != '/' )
    {
        return BAD_REQUEST;
    }

    me->m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

static enum HTTP_CODE parse_headers( myeHttpConnect* me,char* text )
{
    if( text[ 0 ] == '\0' )
    {
        if ( me->m_method == HEAD )
        {
            return GET_REQUEST;
        }

        if ( me->m_content_length != 0 )
        {
            me->m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }

        return GET_REQUEST;
    }
    else if ( strncasecmp( text, "Connection:", 11 ) == 0 )
    {
        text += 11;
        text += strspn( text, " \t" );
        if ( strcasecmp( text, "keep-alive" ) == 0 )
        {
            me->m_linger = 1;
        }
    }
    else if ( strncasecmp( text, "Content-Length:", 15 ) == 0 )
    {
        text += 15;
        text += strspn( text, " \t" );
        me->m_content_length = atol( text );
    }
    else if ( strncasecmp( text, "Host:", 5 ) == 0 )
    {
        text += 5;
        text += strspn( text, " \t" );
        me->m_host = text;
    }
    else
    {
        //        printf( "oop! unknow header %s\n", text );
    }

    return NO_REQUEST;

}

static enum HTTP_CODE parse_content( myeHttpConnect* me, char* text )
{
    if ( me->m_read_idx >= ( me->m_content_length + me->m_checked_idx ) )
    {
        text[ me->m_content_length ] = '\0';
        printf("content = [%s]\n",text);
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

static void unmap(myeHttpConnect* me)
{
    printf("%s() \n",__func__);
    if( me->m_file_address)
    {
        if(me->m_file_flag == 1)
        {
            munmap( me->m_file_address, me-> m_file_stat.st_size );
        }
        else
        {
            free(me->m_file_address);
        }
    }
    me->m_file_address = NULL;
    printf("%s() \n",__func__);
}

static int add_response( myeHttpConnect* me,const char* format, ... )
{
    if( me->m_write_idx >= WRITE_BUFFER_SIZE )
    {
        return 0;
    }
    va_list arg_list;
    va_start( arg_list, format );
    int len = vsnprintf( me->m_write_buf + me->m_write_idx, WRITE_BUFFER_SIZE - 1 - me->m_write_idx, format, arg_list );
    if( len >= ( WRITE_BUFFER_SIZE - 1 - me->m_write_idx ) )
    {
        return 0;
    }
    me->m_write_idx += len;
    va_end( arg_list );
    return 1;
}

static int add_status_line(myeHttpConnect* me, int status, const char* title )
{
    return add_response(me, "%s %d %s\r\n", "HTTP/1.1", status, title );
}

static int add_headers(myeHttpConnect* me,int content_len)
{
    add_content_length(me,content_len);
    add_linger(me);
    add_blank_line(me);
}

static int add_content_length(myeHttpConnect* me, int content_len )
{
    return add_response(me, "Content-Length: %d\r\n", content_len );
}

static int add_linger(myeHttpConnect* me)
{
    return add_response(me, "Connection: %s\r\n", ( me->m_linger == 1 ) ? "keep-alive" : "close" );
}

static int add_blank_line(myeHttpConnect* me)
{
    return add_response(me, "%s", "\r\n" );
}

static int add_content(myeHttpConnect* me, const char* content )
{
    return add_response(me, "%s", content );
}
