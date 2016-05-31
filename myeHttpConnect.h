#ifndef _MYE_HTTP_CONNECT_H_
#define _MYE_HTTP_CONNECT_H_

#include "myeApplication.h"
#include "myeHttpUtil.h"


#ifdef __cplusplus
extern "C"
{
#endif




    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_new 建立并初始化一个连接对象
     *
     * @param application 回调处理句柄
     *
     * @return 一个连接
     */
    /* ----------------------------------------------------------------------------*/
    myeHttpConnect* myeHttpConnect_new(myeApplication* application);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_new_array 建立并初始化count 个对象
     *
     * @param application 回调处理句柄
     * @param count 创建个数
     *
     * @return 
     */
    /* ----------------------------------------------------------------------------*/
    myeHttpConnect* myeHttpConnect_new_array(myeApplication* application,int count);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_connect_init  对连接对象进行赋值
     *
     * @param me
     * @param sockfd    连接套接字
     * @param addr  连接地址
     *
     * @return 成功0 失败非0
     */
    /* ----------------------------------------------------------------------------*/
    int myeHttpConnect_connect_init(myeHttpConnect* me,int sockfd, const struct sockaddr_in addr);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_connect_read 从套接字中读取数据加入到自定义缓冲buf中(可以根据情况自己重写这个函数,暂时支持的是epoll的边缘出发模式读取)
     *
     * @param me myeHttpConnect 句柄
     *
     * @return  CONNECT_CODE 枚举
     */
    /* ----------------------------------------------------------------------------*/
    enum CONNECT_CODE myeHttpConnect_connect_read(myeHttpConnect* me);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_connect_write 从套接字中读取数据加入到自定义缓冲buf中(可以根据情况自己重写这个函数,暂时支持的是epoll的边缘出发模式写入)
     *
     * @param me myeHttpConnect 句柄
     *
     * @return 
     */
    /* ----------------------------------------------------------------------------*/
    enum CONNECT_CODE myeHttpConnect_connect_write(myeHttpConnect* me);


    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_process 数据处理过程
     *
     * @param me myeHttpConnect 句柄
     *
     * @return  
     */
    /* ----------------------------------------------------------------------------*/
    enum HTTP_CODE myeHttpConnect_process(myeHttpConnect* me);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_process_read 将buf中数据进行http解析,如果解析出一个完整的http请求，就会调用处理模块进行处理
     *
     * @param me myeHttpConnect 句柄
     *
     * @return 解析情况结果
     */
    /* ----------------------------------------------------------------------------*/
    enum HTTP_CODE myeHttpConnect_process_read(myeHttpConnect* me);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_process_write 根据HTTP_CODE 包装回复内容
     *
     * @param me myeHttpConnect 句柄
     * @param ret
     *
     * @return 
     */
    /* ----------------------------------------------------------------------------*/
    int myeHttpConnect_process_write(myeHttpConnect* me,enum HTTP_CODE ret);


    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeHttpConnect_delete 释放
     *
     * @param me
     *
     * @return 
     */
    /* ----------------------------------------------------------------------------*/
    int myeHttpConnect_delete(myeHttpConnect* me);

    int myeHttpConnect_delete_array(myeHttpConnect* me);









#ifdef __cplusplus
}
#endif



#endif 
