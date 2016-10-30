/*
 ============================================================================
 Name        : Sender.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include	"config/config.h"
#include	"net/net.h"

void *fnThread_KbdProcess(void *args);

int main(void) {
	char c;
	int id1;
	#define	BUFFER_SIZE  (1514 - 18)
	char	InCmd[BUFFER_SIZE];

	if (config_LoadFromFile()) {
		if (net_init()) {

			pthread_t	hThread;

			int err = pthread_create(&hThread,					//properties->c/c++Build->setting->GCC C++ linker->libraries in top part add "pthread"
									NULL,
									fnThread_KbdProcess,
									&id1);
			printf("%d\n",err);

			//sleep(20);

			//net_send("cmd");

			//do {
				/*c = getchar();
				c = getchar();
				c = getchar();
				c = getchar();*/

				/*do {
					printf("cmd>");
					while ((c = getchar()) != '\n'/*EOF/){
					//puts("cmd>");
					if (c == '\n') printf("cmd");
					//putchar('c');putchar('m');putchar('d');
					putchar(c);
					}
					putchar('\n');
				}while (1);*/

			do {
				printf("cmd>");
				if (fgets(InCmd, BUFFER_SIZE - 1/*-1 for null*/, stdin) != NULL) {
					puts(InCmd);
					//system(InCmd);
					net_send(InCmd);
					if (strcmp(InCmd,"exit\n") == 0) break;
				}
			} while(1);
		}
	}
	puts("Terminated");
	return EXIT_SUCCESS;
}


void *fnThread_KbdProcess(void *args) {
	//int	i;
	//for (i = 0; i<10; i++) {
	//	sleep(1);
		puts("Thread...");
	//}

	pthread_exit(NULL);
	return 0;
}
