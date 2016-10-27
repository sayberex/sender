/*
 * config.h
 *
 *  Created on: 27 окт. 2016 г.
 *      Author: root
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

	#define IF_NAME_LEN		8
	#define	CFG_FILE_NAME	"config.conf"

	extern unsigned char	iface_MAC_DST[6];
	extern char				iface_name[IF_NAME_LEN];

	extern	int config_LoadFromFile(void);

#endif /* CONFIG_CONFIG_H_ */
