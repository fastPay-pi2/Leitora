#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argv, char **argc){
	char *tags[12];
	tags[2] = malloc(100);
	tags[10] = malloc(100);
	tags[0] = "python3";
	tags[1] = "send.py";
	//~ sprintf(tags[2],"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",30,00,00,00,00,00,00,00,00,00,00,00,00,00);
	tags[2] = "30-00-e2-80-11-60-60-00-02-09-59-7b-5c-04";
	tags[3] = "30-00-e2-80-11-60-60-00-02-09-59-7b-48-44";
	tags[4] = "30-00-e2-80-11-60-60-00-02-09-59-7b-52-54";
	tags[5] = "30-00-e2-80-11-60-60-00-02-09-59-7b-48-53";
	tags[6] = "30-00-e2-80-11-60-60-00-02-09-59-7b-52-f4";
	tags[7] = "30-00-e2-80-11-60-60-00-02-09-59-7b-48-e4";
	tags[8] = "30-00-e2-80-11-60-60-00-02-09-59-7b-52-34";
	tags[9] = "30-00-e2-80-11-60-60-00-02-09-59-7b-48-63";
	sprintf(tags[10],"%d",getpid());
	tags[11] = NULL;
	pid_t pid = fork();
	if(pid==0){
		printf("return: %d\n",execvp(tags[0],tags));
		exit(1);
	}
	printf("Pai morrendo!\n");
	while(1);
}
