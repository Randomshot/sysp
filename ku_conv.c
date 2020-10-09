#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//void makeMatrix(int **matrix, int X, int Y);
void sendMsg(int mqdes, Msg msg, int size);
int main(int argc, char** argv){
   
    pid_t pid[5];
    int i, child_status;
    key_t ipckey;
    int mqdes;
    size_t buf_len;
    struct {
        long id;
        int value;
    } mymsg;

    typedef struct {
        long id;
        int** matrix;
    } Msg;
    /*
    int size = atoi(argv [1]);
    int** firstMatrix;
    firstMatrix = (int **)malloc(sizeof(int *) * size);
    for(int i =0; i< size; i++){
        firstMatrix[i] = (int *)malloc(sizeof(int)* size);
    }
    makeMatrix(firstMatrix,size,size);

    for(int i = 0; i<size; i++){
        for(int j=0; j < size; j++){
            printf("%d", firstMatrix[i][j]);
        }
        printf("\n");
    }*/

    /*
    buf_len = sizeof(mymsg.value);
    ipckey = ftok("./",1946);
    mqdes = msgget(ipckey, IPC_CREAT|0600);
    if(mqdes < 0){
        perror("msgget()");
        exit(0);
    }
    for(i=0; i<5; i++){
            mymsg.id = i+1;
            mymsg.value = i*3;
            //printf("Sending a message (val : %d, id : %ld)\n"
            //        ,mymsg.value,mymsg.id);
            if(msgsnd(mqdes,&mymsg,buf_len,0) == -1){
                perror("msgsnd()");
                exit(0);
            }
    }
    
    for(i=0; i<5; i++){
        if((pid[i] = fork()) == 0){
            if(msgrcv(mqdes, &mymsg,buf_len,i+1,0) == -1){
                perror("msgrcv()");
                exit(0);
            }
            else{
                //printf("%d child Received a message (val : %d, id: %ld)\n"
                //           ,i,mymsg.value,mymsg.id);
                ipckey = ftok("./",1947);
                mqdes = msgget(ipckey, IPC_CREAT|0600);
                if(mqdes < 0){
                    perror("msgget()");
                    exit(0);
                }
                else{
                    mymsg.value *= 10;
                    mymsg.id += 10;
                    //printf("%d child Sending a message (val : %d, id : %ld)\n"
                    //    ,i,mymsg.value,mymsg.id);
                    if(msgsnd(mqdes,&mymsg,buf_len,0) == -1){
                        perror("msgsnd()");
                        exit(0);
                    }
                    else exit(0);

                }
            }
        }
    }

   ipckey = ftok("./",1947);
   mqdes = msgget(ipckey, IPC_EXCL|0600);
   if(mqdes < 0){
           perror("msgget()");
           exit(0);
   }

   for(i=0; i<5; i++){
        if(pid[i] != 0){
            if(msgrcv(mqdes, &mymsg,buf_len,i+11,0) == -1){
                perror("msgrcv()");
                exit(0);
            }
            else{
                printf("%d Main Received a message (val : %d, id: %ld)\n"
                    ,i,mymsg.value,mymsg.id);

            }
        }    
   }
    */
   return 0;

}
