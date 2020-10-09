#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
typedef struct {
        long id;
        int** matrix;
} Msg;
int** mallocMatrix( int size);
void makeMatrix(int **matrix, int X, int Y);
void showMatrix(int **matrix, int size);
int **divideMatrix(int **matrix, int size, int index);

void sendMsg1(int mqdes, int **matrix, int size);
pid_t* rcvMsg1(int mqdes, int size);
int **rcvConv(int *pid, int size);
void sendMsg2( int **matrix, int size);
int** dividePooling(int **matrix, int size, int index);
int* rcvMsg2(int size);
int* rcvPooling(pid_t* pid, int size);
int main(int argc, char** argv){
   
    pid_t* pid1;
    pid_t* pid2;
    int i, child_status;
    key_t ipckey;
    int mqdes;
    size_t buf_len;

        
    int size = atoi(argv [1]);
    int** firstMatrix;
    int **conv; 
    int **tmp;
    int *result;
   
    size = 2*size + 2;
    firstMatrix = mallocMatrix(size);
    tmp = mallocMatrix(3);
    
    makeMatrix(firstMatrix,size,size);
    //showMatrix(firstMatrix,size);
    
    ipckey = ftok("./",1999);
    mqdes = msgget(ipckey, IPC_CREAT|0600);
    if(mqdes < 0){
        perror("msgget()");
        exit(0);
    }

    //printf("send msg\n");
    sendMsg1(mqdes,firstMatrix,size);
    pid1 = rcvMsg1(mqdes, size);   
    
    //printf("rcvConv\n");
    conv = rcvConv(pid1,size); 
    //printf("convolution matrix\n");
    //showMatrix(conv,(size-2));
    sendMsg2(conv,size);
    pid2 = rcvMsg2(size);
    result = rcvPooling(pid2,size);
    for(int r =0; r<(size-2)*(size-2)/4; r++){
    	printf("%d ",result[r]);
    }
    printf("\n");
   
    return 0;

}

void showMatrix(int **matrix,int size){
	for(int i = 0; i<size; i++){
        	for(int j=0; j < size; j++){
            		printf("%d", matrix[i][j]);
        	}
        	printf("\n");
    	}

}

int** mallocMatrix(int size){
	int** matrix;
    	matrix = (int **)malloc(sizeof(int *) * size);
    	for(int i =0; i< size; i++){
        	matrix[i] = (int *)malloc(sizeof(int)* size);
    	}
	return matrix;

}

int** divideMatrix(int **matrix, int size, int index){

	int** tmp;
	int i,j;
	tmp = mallocMatrix(3);
	size -= 2;
	for(i =0; i<3; i++){
		for(j=0; j<3; j++){
			tmp[i][j] = matrix[i + (index/size)][j + (index%size)];
		}
	}

	return tmp;
}

void sendMsg1(int mqdes, int **matrix, int size){
	
   
  	Msg msg;
	int i,j;
	size_t buf_len;
	msg.matrix = mallocMatrix(3);
	buf_len = 36;
	for(i=0; i<(size-2)*(size-2); i++){
	  	msg.id = i+1;
            	msg.matrix = divideMatrix(matrix,size,i);
		if(msgsnd(mqdes,&msg,buf_len,1) == -1){
                	perror("msgsnd()");
                	exit(0);
            	}
		
    	}
}
pid_t* rcvMsg1(int mqdes, int size){

  struct {
        long id;
        int value;
  } mymsg;
  Msg msg;
  int tmp = 0;
  size_t buf_len = 36; 
  pid_t* pid;

  key_t ipckey1;
  int mqdes1;
  pid = (pid_t *)malloc(sizeof(pid_t) * (size-2) * (size-2));
  for(int i=0; i<(size-2)*(size-2); i++){
        if((pid[i] = fork()) == 0){
            if(msgrcv(mqdes, &msg,buf_len,i+1,0) == -1){
                perror("msgrcv()");
                exit(0);
            }
            else{
		for(int x = 0; x <3; x++){
			for(int y = 0; y<3; y++){
				if(x == 1 && y == 1){
					tmp += 8 * msg.matrix[x][y];
				}	
				else{
				
					tmp -= msg.matrix[x][y];	
				}
			}
		}
		ipckey1 = ftok("./",2000);
                mqdes1 = msgget(ipckey1, IPC_CREAT|0600);
                buf_len = 4;
		if(mqdes1 < 0){
                    perror("msgget()");
                    exit(0);
                }
                else{
                    mymsg.value = tmp;
                    mymsg.id = i+1;
                    if(msgsnd(mqdes1,&mymsg,buf_len,1) == -1){
                        perror("msgsnd()");
                        exit(0);
                    }
                    else exit(0);
                }
            }
        }
    }

  return pid;
}

int **rcvConv(int *pid, int size){
  	struct {
        	long id;
        	int value;
  	} mymsg;

	int **tmp;
  	key_t ipckey;

    	int mqdes;
	size_t buf_len = 4;
	tmp = mallocMatrix((size-2));
	ipckey = ftok("./",2000);
   	mqdes = msgget(ipckey, IPC_CREAT|0600);
   	if(mqdes < 0){
           perror("msgget()");
           exit(0);
   	}

	//printf("---\n");
   	for(int i=0; i<(size-2)*(size-2); i++){
        	if(pid[i] != 0){
            		if(msgrcv(mqdes, &mymsg,buf_len,i+1,0) == -1){
                	perror("msgrcv()");
                	exit(0);
            		}
            		else{

			  tmp[i/(size-2)][i%(size-2)] = mymsg.value;
            		  //printf("%d\n",tmp[i/(size-2)][i%(size-2)]);
			}
        	}    
   	}
	
	return tmp;
}

int** dividePooling(int **matrix, int size, int index){
	int **tmp;
	size = (size-2)/2;
	tmp = mallocMatrix(2);
	for(int i=0; i<2; i++){
		for(int j=0; j<2; j++){
			tmp[i][j] = matrix[i + (index/size)*2][j + (index%size)*2];
		}
	}

	return tmp;
}

void sendMsg2(int **matrix, int size){
	Msg msg;
	int i,j;
	size_t buf_len;
	key_t ipckey;

    	int mqdes;
	ipckey = ftok("./",2001);
   	mqdes = msgget(ipckey, IPC_CREAT|0600);
   	if(mqdes < 0){
           perror("msgget()");
           exit(0);
   	}
	buf_len = 4 * (size-2)*(size-2);
	for(i=0; i<((size-2)*(size-2))/4; i++){
	  	msg.id = i+1;
		msg.matrix = dividePooling(matrix,size,i);
		//printf("%d poolling\n",i);
		//showMatrix(msg.matrix,2);
		if(msgsnd(mqdes,&msg,buf_len,1) == -1){
                	perror("msgsnd()");
                	exit(0);
            	}
		
    	}

}

int* rcvMsg2(int size){

  struct {
        long id;
        int value;
  } mymsg;
  Msg msg;
  int tmp = 0;
  size_t buf_len = 4 * (size-2)*(size-2); 
  pid_t* pid;

  key_t ipckey1;
  int mqdes1;
  ipckey1 = ftok("./",2001);
  mqdes1 = msgget(ipckey1, IPC_CREAT|0600);

  pid = (pid_t *)malloc(sizeof(pid_t) * (((size-2) * (size-2))/4));
  for(int i=0; i<(size-2)*(size-2)/4; i++){
        
    if((pid[i] = fork()) == 0){

            if(msgrcv(mqdes1, &msg,buf_len,i+1,0) == -1){
                perror("msgrcv()");
                exit(0);
            }
            else{

		for(int x = 0; x <2; x++){
			for(int y = 0; y<2; y++){
				if(x == 0 && y == 0){
					tmp = msg.matrix[x][y];
				}	
				else{
					if(msg.matrix[x][y] > tmp){
						tmp = msg.matrix[x][y];
					}	
				}
			}
		}
		//printf("%d max : %d\n",i,tmp);
		
		ipckey1 = ftok("./",2002);
                mqdes1 = msgget(ipckey1, IPC_CREAT|0600);
                buf_len = 4;
		if(mqdes1 < 0){
                    perror("msgget()");
                    exit(0);
                }
                else{
                    mymsg.value = tmp;
                    mymsg.id = i+1;
                    if(msgsnd(mqdes1,&mymsg,buf_len,1) == -1){
                        perror("msgsnd()");
                        exit(0);
                    }
                    else exit(0);
                }
            }
        }
    }

    return pid;
 }


int* rcvPooling(pid_t* pid, int size){
	int *tmp;
	key_t ipckey1;
  	int mqdes1;
struct {
        long id;
        int value;
  } mymsg;
  size_t buf_len; 


	tmp = (int *)malloc(sizeof(int) * (size-2)*(size-2)/4);
	ipckey1 = ftok("./",2002);
                mqdes1 = msgget(ipckey1, IPC_CREAT|0600);
                buf_len = 4;
		if(mqdes1 < 0){
                    perror("msgget()");
                    exit(0);
                }

	for(int i=0; i<(size-2)*(size-2)/4; i++){
        	if(pid[i] != 0){
            		if(msgrcv(mqdes1, &mymsg,buf_len,i+1,0) == -1){
                	perror("msgrcv()");
                	exit(0);
            		}
            		else{

			  tmp[i] = mymsg.value;
            		  //printf("%d\n",tmp[i/(size-2)][i%(size-2)]);
			}
        	}    
   	}

	return tmp;
}
