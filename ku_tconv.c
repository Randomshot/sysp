#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
typedef struct{
  	int size;
	int** matrix;
} MData;
int **conv(int **matrix, int size);
void *convM(void *arg);
int **max(int **matrix, int size);
void *maxM(void *arg);
MData readInput(char *inputName);
int writeResult(int **result, int resultSize, char *outputName);
int** dividePooling(int **matrix, int size, int index);
int** mallocMatrix( int size);
void showMatrix(int **matrix, int size);
int **divideMatrix(int **matrix, int size, int index);
int main(int argc, char** argv){
    int size;
    char *inputName = argv[1];
    char *outputName = argv[2];
    int **convolution; 
    int **result;
    MData input;

    input = readInput(inputName);
    size = input.size;
    convolution = conv(input.matrix,size);
    result = max(convolution,size);
    writeResult(result,size,outputName);
	
    free(input.matrix);
    free(convolution);
    free(result);
    return 0;
}
MData readInput(char *inputName){

  int fd;
  char value[2];
  char *buf;
  char *matrixSize;
  int size;
  int tmp;
  int sizeLength=0;
  MData result;

  buf = (char*)malloc(sizeof(char));
  fd = open(inputName,O_RDONLY,0644);
  if(fd>0){
    	while(1){
  		read(fd,buf,1);
		if(buf[0] == 10) break;
		else sizeLength++;
	}
	lseek(fd,SEEK_SET,0);
  	matrixSize = (char*)malloc(sizeof(char)*sizeLength);
	read(fd,matrixSize,sizeLength);
	size = atoi(matrixSize);
	result.size = size;
	result.matrix = mallocMatrix(size);
	lseek(fd,1,SEEK_CUR);
	
	for(int i=0; i<size; i++){
		for(int j=0; j<size; j++){
			read(fd,value,2);
			if((value[0] == ' ')){
			  	tmp = value[1] - '0';
				result.matrix[i][j] = tmp; 
			}
			else{
				tmp = (value[0] - '0')*10 + (value[1]-'0');
				result.matrix[i][j] = tmp;
			}
			lseek(fd,1,SEEK_CUR);
		}
	}
  }
  else{
  	printf("file open failed\n");
  }

  close(fd);
  free(matrixSize);
  free(buf);
  return result;

}
int writeResult(int **result, int resultSize, char* outputName){
	
  	int size = (resultSize-2)/2;
	int fd;
	char *tmp;
	char *space = " ";
	char *enter = "\n";

	tmp = (char *)malloc(sizeof(char)*4);
	fd = open(outputName,O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd>0){
		for(int i =0; i<size; i++){
			for(int j=0; j<size; j++){
				if(result[i][j] >= 0){
					if(result[i][j]/100 > 0){
						sprintf(tmp,"%d",result[i][j]);
						write(fd,space,strlen(space));
						write(fd,tmp,strlen(tmp));
					}
					else{
						if(result[i][j]/10 > 0){
							sprintf(tmp,"%d",result[i][j]);
							write(fd,space,strlen(space));
							write(fd,space,strlen(space));
							write(fd,tmp,strlen(tmp));
						}
						else{
							sprintf(tmp,"%d",result[i][j]);
							write(fd,space,strlen(space));
							write(fd,space,strlen(space));
							write(fd,space,strlen(space));
							write(fd,tmp,strlen(tmp));
						}
					}
				}
				else{
					if((-1)*result[i][j]/100 > 0){
						sprintf(tmp,"%d",result[i][j]);
						write(fd,tmp,strlen(tmp));
					}
					else{
						if((-1)*result[i][j]/10 > 0){
							sprintf(tmp,"%d",result[i][j]);
							write(fd,space,strlen(space));
							write(fd,tmp,strlen(tmp));
						}
						else{
							sprintf(tmp,"%d",result[i][j]);
							write(fd,space,strlen(space));
							write(fd,space,strlen(space));
							write(fd,tmp,strlen(tmp));
						}
					}
				}
				if(j < size-1) write(fd,space,strlen(space));
			}
			write(fd,enter,strlen(enter));
		}
			write(fd,enter,strlen(enter));
	}
	else{
		printf("file open failed");
	}

	free(tmp);
	close(fd);
	return 0;
  	
}
int **conv(int **matrix, int size){
	
  	int **result;
	int status;
	int stat;
	int **tmp;
	int *convValue;	
	pthread_t *thread_id,id;
	
	convValue = (int *)malloc(sizeof(int)*(size-2)*(size-2));
	thread_id = (pthread_t *)malloc(sizeof(pthread_t) * (size-2) * (size-2));
	result = mallocMatrix(size-2);

	for(int i=0; i<(size-2); i++){
		for(int j=0; j<(size-2); j++){
			tmp = divideMatrix(matrix,size,j+i*(size-2));
			status = pthread_create(&thread_id[j+i*(size-2)],NULL,convM,(void *)tmp);
			if(status < 0){
				perror("pthread_create");
				exit(1);
			}

		}
		
		for(int j=0; j<(size-2); j++){
			stat = pthread_join(thread_id[j+(i)*(size-2)],(void *)&convValue[j+i*(size-2)]);
			if(stat < 0){
				perror("pthread_join");
				exit(1);
			}
			result[i][j] = convValue[j+i*(size-2)];
		}
	}
	return result;
}

void *convM(void *arg){
	int **tmp = ((int **)arg);
	int result = 0;
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			if(i == 1 && j == 1) result += 8 * tmp[i][j];
			else result -= tmp[i][j];
		}
	}
	return (void *)(result);
}

int **max(int **matrix, int size){

  	int **result;
	int status;
	int stat;
	int **tmp;
	int *maxPoolingValue;
	pthread_t *thread_id,id;
	
	thread_id = (pthread_t *)malloc(sizeof(pthread_t) * (size-2) * (size-2)/4);
	result = mallocMatrix((size-2)/2);
	maxPoolingValue = (int *)malloc(sizeof(int) * (size-2) * (size-2)/4);
	
	for(int i =0; i<(size-2)/2; i++){
		for(int j=0; j<(size-2)/2; j++){
			tmp = dividePooling(matrix,size,j+i*(size-2)/2);
			status = pthread_create(&thread_id[j+i*(size-2)/2],NULL,maxM,(void *)tmp);
			if(status != 0){
				perror("pthread_create");
				exit(1);
			}
		}
		for(int j=0; j<(size-2)/2; j++){
			stat = pthread_join(thread_id[j+i*(size-2)/2],(void *)&maxPoolingValue[j+i*(size-2)/2]);
			if(status != 0){
				perror("pthread_join");
				exit(1);
			}
			result[i][j] = maxPoolingValue[j+i*(size-2)/2];
		}
	}
	return result;
}

void *maxM(void *arg){

	int **tmp = ((int **)arg);
	int result = 0;

	for(int i=0; i<2; i++){
		for(int j=0; j<2; j++){
			if(i == 0 && j == 0) result = tmp[i][j];
			else{
				if(result < tmp[j][i]) result = tmp[j][i];
			}
		}
	}
	return (void *)(result);
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

