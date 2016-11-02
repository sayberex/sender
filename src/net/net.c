/*
 * net.c
 *
 *  Created on: 28 окт. 2016 г.
 *      Author: root
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	<sys/socket.h>
#include	<sys/ioctl.h>

#include	<net/if.h>
//#include	<net/if_packet.h>
#include	<linux/if_packet.h>
#include	<net/ethernet.h>
#include	<net/if_arp.h>
//#include	<linux/if_ether.h>

#include	<linux/ppp_defs.h>
//#include	<net/ppp_defs.h>

#include 	<arpa/inet.h>

#include	"../config/config.h"
#include	"../crc/crc.h"
#include	"net.h"


int				iface_index;
unsigned char	iface_MAC_SRC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//network interface MAC
int				net_snd_socketfd;
int				net_rcv_socketfd;

int 			net_getMAC		(unsigned char mac[6]);					//get current interface MAC
int 			net_createsocket(int *socketfd);

int	net_init(void) {
	if (net_getMAC(iface_MAC_SRC) != 0) {							//check is source network interface MAC set
		printf("SRC MAC   : %02x %02x %02x %02x %02x %02x\n",		//show source MAC
				iface_MAC_SRC[0],iface_MAC_SRC[1],iface_MAC_SRC[2],
				iface_MAC_SRC[3],iface_MAC_SRC[4],iface_MAC_SRC[5]);


		if ((iface_index = if_nametoindex(iface_name)) != 0) {
			printf("%s index: %d\n", iface_name, iface_index);

			if (net_createsocket(&net_snd_socketfd) && net_createsocket(&net_rcv_socketfd)) {

				crcInit();
				//close(net_socketfd);
				return 1;
			}
		}
		else
			printf("iface %s not found\n", iface_name);
	}
	//close(net_socketfd);
	return 0;
}


int net_getMAC(unsigned char mac[6]) {
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_ifrn.ifrn_name,iface_name);
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		int	i;
		for (i = 0; i < 6; ++i) mac[i] = s.ifr_ifru.ifru_addr.sa_data[i];
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

int net_createsocket(int *socketfd) {
	int sockfd;
	/*
	 * PF_PACKET - Protocol family (PF_INET PF_INET6 )
	 * SOCK_RAW  - Socket Type
	 * ETH_P_ALL - Protocol ID
	 *
	 * PF_PACKET - low level packet
	 * SOCK_RAW	 - Socket Type RAW(direct access to network interface)
	 * ETH_P_ALL - receive all ethernet frames
	 */

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd == -1) {
		puts("socket not created\n");
		return 0;
	}
	else {
		printf("socket created fd = %d\n", sockfd);
		*socketfd = sockfd;
		return 1;
	}
}

union {
	unsigned long	ulValue;
	unsigned char	Bytes[4];
} uSndCRC, uRcvCRC, uRcvCalculatedCRC;

void net_snd(int socketfd, char *sndstr) {

	//check if data length less or equal that frame can contain
	if (strlen(sndstr) < MY_PROTO_MAX_DATA_LEN) {

		struct sockaddr_ll socket_addr = {
				PF_PACKET,			/*sll_family*/					//Protocol family PF_PACKET - Device level Packet Socket(AF_INET(IPv4), AF_INET6(IPv6))
				htons(ETH_P_ALL),	/*sll_protocol*/				//low level protocol ID (like CAN IrDA and so on)
				0,					/*sll_ifindex*/					//communication interface index(eth0)
				ARPHRD_ETHER,		/*sll_hatype*/					//ARP Protocol Hardware ID(Ethernet 10Mbps)
				PACKET_OTHERHOST,	/*sll_pkttype*/					//PACKET Type(to all broadcast/to group multicast/to user space /to kernel space/ and so on)
				ETH_ALEN,			/*sll_halen*/					//Ethernet address length(MAC - length)
				{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}	//Physical layer MAC
		};

		socket_addr.sll_ifindex = iface_index;						//network interface index

		void 		  *pvSndBuf 	= (void *)malloc(ETH_FRAME_LEN);//Send Packet Buffer
		unsigned char *etherhead	= pvSndBuf;						//Ethernet header pointer
		unsigned char *data 		= pvSndBuf + 14;				//Ethernet packet data pointer

		int	iResult;
		stMyProtoHdr_t	*pstMyProtoHdr = (stMyProtoHdr_t *)pvSndBuf;

		//Set the frame header
		//struct ethhdr *eh 			= (struct ethhdr *)etherhead;//Structure pointer to Ethernet header
		//memcpy((void *)pvSndBuf, 			(void *)iface_MAC_DST, ETH_ALEN);
		//memcpy((void *)pvSndBuf + ETH_ALEN, (void *)iface_MAC_SRC, ETH_ALEN);
		//eh->h_proto = 0;

																	// fill ethernet header
		memcpy(pstMyProtoHdr->h_dest, 	iface_MAC_DST, ETH_ALEN);	// fill with dst mac
		memcpy(pstMyProtoHdr->h_source, iface_MAC_SRC, ETH_ALEN);	// fill with src mac
		pstMyProtoHdr->h_proto = 0;									// fill with L3 protocol type

																	// fill myprotocol header
		pstMyProtoHdr->h_len = strlen(sndstr);						// fill myprotocol data length
		memcpy(pstMyProtoHdr->h_sign,MY_PROTO_SIGN, strlen(MY_PROTO_SIGN)); // fill with myprotocol signature
		memcpy(pstMyProtoHdr->data, (void *)sndstr, strlen(sndstr));		// fill with muprotocol data

																	// Calculate amd set packet crc
		uSndCRC.ulValue = crcFast(pvSndBuf, ETH_FRAME_LEN - 4);
		//printf("Send Packet CRC = %x\n", uSndCRC.ulValue);
																	//Fill packet with CRC
		((unsigned char *)pvSndBuf)[ETH_FRAME_LEN - 4] = uSndCRC.Bytes[0];
		((unsigned char *)pvSndBuf)[ETH_FRAME_LEN - 3] = uSndCRC.Bytes[1];
		((unsigned char *)pvSndBuf)[ETH_FRAME_LEN - 2] = uSndCRC.Bytes[2];
		((unsigned char *)pvSndBuf)[ETH_FRAME_LEN - 1] = uSndCRC.Bytes[3];



		iResult = sendto(socketfd, pvSndBuf, ETH_FRAME_LEN, 0,
				(struct sockaddr *)&socket_addr, sizeof(socket_addr));

		if (iResult == -1) 	puts	("error sendto");
		//else 				printf	("%d bytes sended\n", iResult);

		free(pvSndBuf);
	}
}

void net_send(char *sndstr) {
	net_snd(net_snd_socketfd, sndstr);
}

//memcmp

void net_recv(void) {
	unsigned long			ulRcvLen;
	unsigned char 			ucRcvBuf[ETH_FRAME_LEN];
	stMyProtoHdr_t			*pstMyProtoHdr = (stMyProtoHdr_t *)ucRcvBuf;

	ulRcvLen = 0;
	ulRcvLen = recvfrom(net_rcv_socketfd ,ucRcvBuf, ETH_FRAME_LEN, MSG_DONTWAIT, NULL, NULL);

	//if data received ok
	if (ulRcvLen != -1) {

		//check packet crc
		//check for minimal packet size
		//if (ulRcvLen >= MY_PROTO_MIN_LEN) {
		if (ulRcvLen == ETH_FRAME_LEN) {

			//get packet CRC
			uRcvCRC.Bytes[0] = ((unsigned char *)ucRcvBuf)[ETH_FRAME_LEN - 4];
			uRcvCRC.Bytes[1] = ((unsigned char *)ucRcvBuf)[ETH_FRAME_LEN - 3];
			uRcvCRC.Bytes[2] = ((unsigned char *)ucRcvBuf)[ETH_FRAME_LEN - 2];
			uRcvCRC.Bytes[3] = ((unsigned char *)ucRcvBuf)[ETH_FRAME_LEN - 1];

			//calculate packet CRC
			uRcvCalculatedCRC.ulValue = crcFast(ucRcvBuf, ETH_FRAME_LEN -4);

			//compare CRC
			if (uRcvCRC.ulValue == uRcvCalculatedCRC.ulValue) {

				//check if its our mac
				if (memcmp(pstMyProtoHdr->h_dest, iface_MAC_SRC, 6) == 0) {

					//check for protocol signature
					if (memcmp(pstMyProtoHdr->h_sign, MY_PROTO_SIGN, strlen(MY_PROTO_SIGN)) == 0) {

						//check correct packet length
						if ((pstMyProtoHdr->h_len > 0) && (pstMyProtoHdr->h_len < MY_PROTO_MAX_DATA_LEN)) {

							//output received data to stdout
							(pstMyProtoHdr->data)[pstMyProtoHdr->h_len] = 0;
							printf("%scmd>", pstMyProtoHdr->data);
							fflush(stdout);
							//fwrite(pstMyProtoHdr->data , sizeof(unsigned char), pstMyProtoHdr->h_len, stdout);
							//fwrite("\ncmd>", sizeof(unsigned char), 0, stdout);
						}
					}
				}
			}
		}
	}
}
/*
void net_recv(void) {
	//
}*/
