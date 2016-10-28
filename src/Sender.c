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
#include	"net/net.h"

int main(void) {

	if (config_LoadFromFile()) {
		if (net_init()) {
			net_send("cmd");
		}
	}
	puts("Terminated");
	return EXIT_SUCCESS;
}
