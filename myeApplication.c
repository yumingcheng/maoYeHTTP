#include "myeApplication.h"
const char* doc_root = "/var/www/html/";

myeApplication* myeApplication_new()
{
     myeApplication* me = (myeApplication*)calloc(1,sizeof(myeApplication));
     return me;
}

enum HTTP_CODE myeApplication_process(myeApplication* me,myeHttpConnect* connect)
{
    printf("%s()\n",__func__); 
    strcpy( connect->m_real_file, doc_root );
    printf("11 = %s\n",connect->m_real_file);
    int len = strlen( doc_root );
    strncpy( connect->m_real_file + len, connect->m_url, FILENAME_LEN - len - 1 );
    if ( stat( connect->m_real_file, &connect-> m_file_stat ) < 0 )
    {
        return NO_RESOURCE;
    }

    if ( ! ( connect-> m_file_stat.st_mode & S_IROTH ) )
    {
        return FORBIDDEN_REQUEST;
    }

    if ( S_ISDIR( connect-> m_file_stat.st_mode ) )
    {
        return BAD_REQUEST;
    }

    connect->m_file_flag = 1;
    int fd = open( connect->m_real_file, O_RDONLY );
    connect->m_file_length = connect-> m_file_stat.st_size;
    connect->m_file_address = ( char* )mmap( 0, connect-> m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
    close( fd );
    return FILE_REQUEST;
}

int myeApplication_delete(myeApplication* me)
{
    if(me)
        free(me);
    return 0;
}
