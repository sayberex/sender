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


int				iface_index;
unsigned char	iface_MAC_SRC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//network interface MAC
int				net_snd_socketfd;
int				net_rcv_socketdf;

int 			net_getMAC		(unsigned char mac[6]);					//get current interface MAC
int 			net_createsocket(int *socketfd);

int	net_init(void) {
	if (net_getMAC(iface_MAC_SRC) != 0) {							//check is source network interface MAC set
		printf("SRC MAC   : %02x %02x %02x %02x %02x %02x\n",		//show source MAC
				iface_MAC_SRC[0],iface_MAC_SRC[1],iface_MAC_SRC[2],
				iface_MAC_SRC[3],iface_MAC_SRC[4],iface_MAC_SRC[5]);


		if ((iface_index = if_nametoindex(iface_name)) != 0) {
			printf("%s index: %d\n", iface_name, iface_index);

			if (net_createsocket(&net_snd_socketfd)) {

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
} uSndCRC;

void net_snd(int socketfd, char *sndstr) {
	//target address
	struct sockaddr_ll socket_addr = {
		PF_PACKET,			/*sll_family*/					//Protocol family PF_PACKET - Device level Packet Socket(AF_INET(IPv4), AF_INET6(IPv6))
		htons(ETH_P_ALL),	/*sll_protocol*/				//low level protocol ID (like CAN IrDA and so on)
		0,					/*sll_ifindex*/					//communication interface index(eth0)
		ARPHRD_ETHER,		/*sll_hatype*/					//ARP Protocol Hardware ID(Ethernet 10Mbps)
		PACKET_OTHERHOST,	/*sll_pkttype*/					//PACKET Type(to all broadcast/to group multicast/to user space /to kernel space/ and so on)
		ETH_ALEN,			/*sll_halen*/					//Ethernet address length(MAC - length)
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}	//Physical layer MAC
	};

	socket_addr.sll_ifindex = iface_index;					//network interface index

	void 		  *pvSndBuf 	= (void *)malloc(ETH_FRAME_LEN);//Send Packet Buffer
	unsigned char *etherhead	= pvSndBuf;					//Ethernet header pointer
	unsigned char *data 		= pvSndBuf + 14;			//Ethernet packet data pointer
	struct ethhdr *eh 			= (struct ethhdr *)etherhead;//Structure pointer to Ethernet header
	int	iResult;

	//Set the frame header
	memcpy((void *)pvSndBuf, 			(void *)iface_MAC_DST, ETH_ALEN);
	memcpy((void *)pvSndBuf + ETH_ALEN, (void *)iface_MAC_SRC, ETH_ALEN);
	eh->h_proto = 0;

	memcpy((void *)data, (void *)sndstr, strlen(sndstr));		//fill frame with some data


	uSndCRC.ulValue = crcFast(pvSndBuf, ETH_FRAME_LEN - 4);
	printf("Send Packet CRC = %x\n", uSndCRC.ulValue);

	etherhead[ETH_FRAME_LEN - 4] = uSndCRC.Bytes[0];
	etherhead[ETH_FRAME_LEN - 3] = uSndCRC.Bytes[1];
	etherhead[ETH_FRAME_LEN - 2] = uSndCRC.Bytes[2];
	etherhead[ETH_FRAME_LEN - 1] = uSndCRC.Bytes[3];

	if ( strlen(sndstr) < (ETH_FRAME_LEN - (6 + 6 + 2 + 4)) ) {
		iResult = sendto(socketfd, pvSndBuf, ETH_FRAME_LEN, 0,
				(struct sockaddr *)&socket_addr, sizeof(socket_addr));

		if (iResult == -1) 	puts	("error sendto");
		else 				printf	("%d bytes sended\n", iResult);
	}
	else puts("packet too long");
	free(pvSndBuf);
}

void net_send(char *sndstr) {
	net_snd(net_snd_socketfd, sndstr);
}

void net_recv(void) {
	unsigned long	ulRcvLen;
	unsigned char 	ucRcvBuf[ETH_FRAME_LEN];

	ulRcvLen = 0;
	ulRcvLen = recvfrom(net_rcv_socketdf ,ucRcvBuf, ETH_FRAME_LEN, 0. NULL, NULL);
	//check if its our mac
	//check msg identifier(signature)
	//maby give eth struct to function and checm mac there
	if (ulRcvLen != -1) {
		//put output
	}
}

