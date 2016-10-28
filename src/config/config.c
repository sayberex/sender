/*
 * config.c
 *
 *  Created on: 27 окт. 2016 г.
 *      Author: root
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	"config.h"

unsigned char	iface_MAC_DST[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//network interface MAC
char			iface_name[IF_NAME_LEN] = {0};							//network interface name

unsigned long	f_iface_name_set;										//is name readed from config file
unsigned long	f_iface_MAC_set;										//is MAC readed from config file

void 			config_exclude	(char *str, char sym);					//exclude symbols(sym) from string(str)
void 			config_getparam	(char *str);							//parse config file and set params

int config_LoadFromFile(void) {
	#define	MAX_STR_LEN	32												//Maximal string length readed from file
	FILE	*fp;														//File pointer
	char	str[MAX_STR_LEN];											//buffer string for content readed from file

	f_iface_name_set = 0;												//network interface name was not readed
	f_iface_MAC_set  = 0;												//network interface MAC was not readed

	if ((fp = fopen(CFG_FILE_NAME, "r")) != NULL) {						//open config file

		while (fgets(str, MAX_STR_LEN, fp) != NULL) {					//read strings from file
			config_exclude(str,' ');									//exclude all spaces from string
			config_exclude(str,'\n');									//exclude all spaces from string
			config_exclude(str,'\r');									//exclude all spaces from string
			config_getparam(str);										//parse config params
		}

		if ((f_iface_name_set) && (f_iface_MAC_set)) {					//check is destenation network interface name & MAC set

			printf("iface name: %s\n", iface_name);						//show network interface name readed from config file

			printf("DST MAC   : %02x %02x %02x %02x %02x %02x\n",		//show destenation MAC
					iface_MAC_DST[0],iface_MAC_DST[1],iface_MAC_DST[2],
					iface_MAC_DST[3],iface_MAC_DST[4],iface_MAC_DST[5]);
			return 1;
		}

		fclose(fp);
		return 0;
	} else {
		printf("Error: can't open file %s",CFG_FILE_NAME);
	}

	return 0;
}

void config_exclude(char *str, char sym) {
	int i,j;

	if (strlen(str) != 0) {
		for (i = j = 0; str[i] != '\0'; i++) {
			if (str[i] != sym) str[j++] = str[i];
		}
		str[j] = '\0';
	}
}

void config_getparam(char *str) {
	char	tmpstr[3] = {0, 0, 0};

	if (strncmp(str, "iface:", 6) == 0) {
		f_iface_name_set = 1;
		strcpy(iface_name, str + 6);
	}
	if (strncmp(str, "mac:", 4) == 0) {
		if (strlen(str) >= 16) {
			f_iface_MAC_set  = 1;
			tmpstr[0] = *(str + 4); tmpstr[1] = *(str + 5); iface_MAC_DST[0] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
			tmpstr[0] = *(str + 6); tmpstr[1] = *(str + 7); iface_MAC_DST[1] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
			tmpstr[0] = *(str + 8); tmpstr[1] = *(str + 9); iface_MAC_DST[2] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
			tmpstr[0] = *(str +10); tmpstr[1] = *(str +11); iface_MAC_DST[3] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
			tmpstr[0] = *(str +12); tmpstr[1] = *(str +13); iface_MAC_DST[4] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
			tmpstr[0] = *(str +14); tmpstr[1] = *(str +15); iface_MAC_DST[5] = strtol(tmpstr,NULL,16); //atoi(tmpstr);
		}
	}
}
