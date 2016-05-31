#ifndef _MYEAPPLICATION_H_
#define _MYEAPPLICATION_H_


#include "myeHttpUtil.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct _myeHttpConnect; 


    typedef struct _myeApplication
    {           
        int mao;

    }myeApplication;

    myeApplication* myeApplication_new();

    enum HTTP_CODE myeApplication_process(myeApplication* me,struct _myeHttpConnect* connect);

    int myeApplication_delete(myeApplication* me);


#ifdef __cplusplus
}
#endif


#endif 
