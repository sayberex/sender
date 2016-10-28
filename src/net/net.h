/*
 * net.h
 *
 *  Created on: 28 окт. 2016 г.
 *      Author: root
 */

#ifndef NET_NET_H_
#define NET_NET_H_

	extern unsigned char	iface_MAC_SRC[6];

	extern	int				net_init(void);
	extern	void 			net_send(char *sndstr);

#endif /* NET_NET_H_ */
