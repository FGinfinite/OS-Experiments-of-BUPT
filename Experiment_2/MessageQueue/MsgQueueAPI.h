#include <sys/types.h>
int msgget(key_t key, int msgflg);
int msgrcv(int msgid,void *msg,size_t msgsz,long msgtyp,int msgflg);
int msgsnd(int msgid,const void* msg,size_t msgsz,int msgflg);