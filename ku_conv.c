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

typedef struct{
	long id;
	int value;
} Mymsg;

int** mallocMatrix( int size);
void makeMatrix(int **matrix, int X, int Y);
void showMatrix(int **matrix, int size);
int **divideMatrix(int **matrix, int size, int index);
int** dividePooling(int **matrix, int size, int index);
int **convMatrix(int mqdes1,int mqdes2, int **matrix,int size);
int *maxPooling(int mqdes1, int mqdes2, int **result, int size);
void printResult(int *result, int size);

int main(int argc, char** argv){
   
    key_t ipckey1, ipckey2,ipckey3,ipckey4;
    int mqdes1,mqdes2,mqdes3,mqdes4;
    int size = atoi(argv [1]);
    int** firstMatrix;
    int **conv; 
    int *result;
   
    
    ipckey1 = ftok("./",2001);
    mqdes1 = msgget(ipckey1, IPC_CREAT|0600);
    if(mqdes1 < 0){
        perror("msgget()");
        exit(0);
    }

    ipckey2 = ftok("./",2002);
    mqdes2 = msgget(ipckey2, IPC_CREAT|0600);
    if(mqdes2 < 0){
        perror("msgget()");
        exit(0);
    }

    ipckey3 = ftok("./",2003);
    mqdes3 = msgget(ipckey3, IPC_CREAT|0600);
    if(mqdes3 < 0){
        perror("msgget()");
        exit(0);
    }

    ipckey4 = ftok("./",2004);
    mqdes4 = msgget(ipckey4, IPC_CREAT|0600);
    if(mqdes4 < 0){
        perror("msgget()");
        exit(0);
    }

    firstMatrix = mallocMatrix(size);
    makeMatrix(firstMatrix,size,size);
    conv = convMatrix(mqdes1,mqdes2,firstMatrix,size); 
    result = maxPooling(mqdes3,mqdes4,conv,size);
    printResult(result,size); 
    
    free(firstMatrix);
    free(conv);
    free(result);

    msgctl(mqdes1,IPC_RMID,0);
    msgctl(mqdes2,IPC_RMID,0);
    msgctl(mqdes3,IPC_RMID,0);
    msgctl(mqdes4,IPC_RMID,0);
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

void printResult(int *result, int size){
	
  	for(int x = 0; x<(size-2)*(size-2)/4; x++){
		printf("%d",result[x]);
		if(x< -1 +(size-2)*(size-2)/4){
			printf(" ");
		}
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

int **convMatrix(int mqdes1,int mqdes2, int **matrix,int size){
	pid_t *pid;
	pid_t wpid;
	Msg msg;
	Msg rcv;
	Mymsg mymsg;
	int i,j,tmp,child_status;
	size_t buf_len = 8;
	int **result;
	
	result = mallocMatrix(size-2);
	pid = (pid_t *)malloc(sizeof(pid_t) * (size-2) * (size-2));
	i=0;
	j=0;
	tmp = 0;
	while(1){
		if(i == (size-2)*(size-2)) break;
		if(j < 4){
			msg.id = i+1;
			msg.matrix = divideMatrix(matrix,size,i);
			if(msgsnd(mqdes1,&msg,buf_len,1) == -1){
				perror("msgsnd()");
				exit(0);
			}
			i++;
			j++;
		}
		 if(j == 4){
			for(int x=0; x<4; x++){
		   		if((pid[i+x] = fork()) == 0){
					if(msgrcv(mqdes1,&rcv,buf_len,0,0) == -1){
						perror("msgrcv()");
						exit(0);
					}
					else{
					  	for(int y = 0; y<3; y++){
							for(int z = 0; z<3; z++){
								if(y == 1 && z==1){
									tmp += 8 * rcv.matrix[y][z];
								}
								else{
									tmp -= rcv.matrix[y][z];
								}
							}
						}
						mymsg.id = rcv.id;
						mymsg.value = tmp;
						if(msgsnd(mqdes2,&mymsg,buf_len,1) == -1){
							perror("msgnsd()");
							exit(0);
						}
						else{
							free(msg.matrix);
						}
						exit(0);
					}
				}
				else{
						wpid = wait(&child_status);			
						if(msgrcv(mqdes2,&mymsg,buf_len,0,0) == -1){
							perror("msgrcv()");
							exit(0);
						}
						else{
						  	mymsg.id -= 1;
						  	result[mymsg.id/(size-2)][mymsg.id%(size-2)] = mymsg.value;
						}


				}
			}

		  	j = 0;
		}
	}

	free(pid);
	return result;
}

int *maxPooling(int mqdes1, int mqdes2, int **result, int size){

	pid_t *pid;
	pid_t wpid;
	Msg msg;
	Msg rcv;
	Mymsg mymsg;
	int i,j,tmp,child_status;
	size_t buf_len = 8;
	
	pid = (pid_t *)malloc(sizeof(pid_t) * (size-2) * (size-2));
	i=0;
	j=0;

  	int *rs = (int *)malloc(sizeof(int) * (size-2)*(size-2)/4);
	while(1){
		if(i == (size-2)*(size-2)/4) break;
		if(j < 4){
			msg.id = i+1;
			msg.matrix = dividePooling(result,size,i);
			if(msgsnd(mqdes1,&msg,buf_len,1) == -1){
				perror("msgsnd()");
				exit(0);
			}
			i++;
			j++;
		}
		 if(j == 4 || i == (size-2)*(size-2)/4){
			for(int x=0; x<j; x++){
		   		if((pid[i+x] = fork()) == 0){
			
					if(msgrcv(mqdes1,&rcv,buf_len,0,0) == -1){
						perror("msgrcv()");
						exit(0);
					}
					else{
					  	
						mymsg.id = rcv.id;
						for(int a = 0; a <2; a++){
							for(int b = 0; b<2; b++){
								if(a == 0 && b == 0){
									tmp = rcv.matrix[a][b];
								}	
								else{
									if(rcv.matrix[a][b] > tmp){
										tmp = rcv.matrix[a][b];
									}	
								}
							}
						}

						mymsg.value = tmp;
						if(msgsnd(mqdes2,&mymsg,buf_len,1) == -1){
							perror("msgnsd()");
							exit(0);
						}
						else{
							free(msg.matrix);
						}
						exit(0);
					}
				}
				else{
						wpid = wait(&child_status);			
						if(msgrcv(mqdes2,&mymsg,buf_len,0,0) == -1){
							perror("msgrcv()");
							exit(0);
						}
						else{
							rs[mymsg.id-1] = mymsg.value;
						}


				}
			}

		  	j = 0;
		}
	}

	free(pid);
	return rs;
}
