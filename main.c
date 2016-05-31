#include "myeApplication.h"
#include "myeHttpConnect.h"


int main(int argc,char* argv[])
{
     myeApplication* application = myeApplication_new();

     myeHttpConnect* connect  = myeHttpConnect_new(application);

     myeApplication_process(application,connect);





    return 0;
}
