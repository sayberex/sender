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

#include	"config/config.h"

int main(void) {

	if (config_LoadFromFile()) {
		//code
	}
	puts("Terminated");
	return EXIT_SUCCESS;
}
