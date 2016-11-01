/*
 * net.h
 *
 *  Created on: 28 окт. 2016 г.
 *      Author: root
 */

#ifndef NET_NET_H_
#define NET_NET_H_
	#include	<linux/if_ether.h>

	#define	MY_PROTO_SIGN	"MY_PROTO_ID_SIGN"			// my protocol signature

	typedef struct __attribute__((packed)) {									// my protocol data structure
		unsigned char	h_dest	[ETH_ALEN];				// eth destination eth addr
		unsigned char	h_source[ETH_ALEN];				// eth source ether addr
		__be16			h_proto;						// packet type ID field
		unsigned short	h_len;							// my protocol packet data length
		unsigned char	h_sign[sizeof(MY_PROTO_SIGN)];	// my protocol packet signature
		unsigned char	data[];							// my protocol packet data

	}stMyProtoHdr_t;

	//typedef stMyP
														// my protocol must contain at least
														// eth hdr + my data length + signature + eth FCS
	#define MY_PROTO_MIN_LEN	(ETH_ALEN*2 + 2 + 2 + sizeof(MY_PROTO_SIGN) + 4)

														//max amount of data that can be transfer in single frame
	#define	MY_PROTO_MAX_DATA_LEN	(ETH_FRAME_LEN - MY_PROTO_MIN_LEN)


	extern unsigned char	iface_MAC_SRC[6];

	extern	int				net_init(void);
	extern	void 			net_send(char *sndstr);
	extern	void 			net_recv(void);

#endif /* NET_NET_H_ */
