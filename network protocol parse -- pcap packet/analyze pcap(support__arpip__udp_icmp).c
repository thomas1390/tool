#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pcap.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <signal.h>

/* 所有ICMP处理的函数都拷贝了sniffer.c的代码，将函数分开就是功能模块清晰
 *  下面是拷贝该程序的说明
 *  libpcap demostration code for Webb.it 2004 Padova
 *  coded by: Paolo Carpo <snifth@spine-group.org>
 *  compile with: gcc -Wall -O2 sniffer.c -o sniffer -lpcap
*/

/* Define header protocol lenghts */
#define ICMPV4_H         0x04    /**< ICMP header base:     4 bytes */
#define IPV4_H           0x14    /**< IPv4 header:         20 bytes */

//拷贝sniffer.c的ICMP头部
/*
 *  ICMP header
 *  Internet Control Message Protocol
 *  Base header size: 4 bytes
 */
struct icmpv4_hdr
{
   u_int8_t icmp_type;       /* ICMP type */
#define     ICMP_ECHOREPLY                  0
#define     ICMP_UNREACH                    3
#define     ICMP_SOURCEQUENCH               4
#define     ICMP_REDIRECT                   5
#define     ICMP_ECHO                       8
#define     ICMP_ROUTERADVERT               9
#define     ICMP_ROUTERSOLICIT              10
#define     ICMP_TIMXCEED                   11
#define     ICMP_PARAMPROB                  12
#define     ICMP_TSTAMP                     13
#define     ICMP_TSTAMPREPLY                14
#define     ICMP_IREQ                       15
#define     ICMP_IREQREPLY                  16
#define     ICMP_MASKREQ                    17
#define     ICMP_MASKREPLY                  18

   u_int8_t icmp_code;       /* ICMP code */

#define     ICMP_UNREACH_NET                0
#define     ICMP_UNREACH_HOST               1
#define     ICMP_UNREACH_PROTOCOL           2
#define     ICMP_UNREACH_PORT               3
#define     ICMP_UNREACH_NEEDFRAG           4
#define     ICMP_UNREACH_SRCFAIL            5
#define     ICMP_UNREACH_NET_UNKNOWN        6
#define     ICMP_UNREACH_HOST_UNKNOWN       7
#define     ICMP_UNREACH_ISOLATED           8
#define     ICMP_UNREACH_NET_PROHIB         9
#define     ICMP_UNREACH_HOST_PROHIB        10
#define     ICMP_UNREACH_TOSNET             11
#define     ICMP_UNREACH_TOSHOST            12
#define     ICMP_UNREACH_FILTER_PROHIB      13
#define     ICMP_UNREACH_HOST_PRECEDENCE    14
#define     ICMP_UNREACH_PRECEDENCE_CUTOFF  15
#define     ICMP_REDIRECT_NET               0
#define     ICMP_REDIRECT_HOST              1
#define     ICMP_REDIRECT_TOSNET            2
#define     ICMP_REDIRECT_TOSHOST           3
#define     ICMP_TIMXCEED_INTRANS           0
#define     ICMP_TIMXCEED_REASS             1
#define     ICMP_PARAMPROB_OPTABSENT        1

   u_int16_t icmp_sum;   /* ICMP Checksum */

   union
     {
        struct
          {
             u_int16_t id; /* ICMP id */
             u_int16_t seq;/* ICMP sequence number */
          }
        echo;

#undef icmp_id
#undef icmp_seq
#define icmp_id     hun.echo.id
#define icmp_seq    hun.echo.seq

        u_int32_t gateway; /* gateway host */
        struct
          {
             u_int16_t pad;/* padding */
             u_int16_t mtu;/* MTU size */
          }
        frag;
     }
   hun;
};

/* prototipes of functions */
void dump_icmp(struct icmpv4_hdr *packet, int offset);
  
main (int argc, char **argv[])
{
  struct pcap_file_header
  {
    bpf_u_int32 magic;
    u_short version_major;
    u_short version_minor;
    bpf_int32 thiszone;		/* gmt to local correction */
    bpf_u_int32 sigfigs;	/* accuracy of timestamps */
    bpf_u_int32 snaplen;	/* max length saved portion of each pkt */
    bpf_u_int32 linktype;	/* data link type (LINKTYPE_*) */
  };

  struct pcap_pkthdr
  {
    struct timeval ts;		/* time stamp */
    bpf_u_int32 caplen;		/* length of portion present */
    bpf_u_int32 len;		/* length this packet (off wire) */
  };

  struct EthernetPacket
  {
    unsigned char MacDst[6];	/* 目的网卡物理地址 */
    unsigned char MacSrc[6];	/* 源网卡物理地址 */
    unsigned short PacketType;	/* 包类型， ip或ARP等 */
  };

  struct pcap_file_header pcap_hdr;
  struct pcap_pkthdr pkt_hdr;
  struct EthernetPacket eth_hdr;
  struct pcap_file_header *pcap_p;
  struct pcap_pkthdr *pkt_p;
  struct EthernetPacket *eth_p;


  FILE *fp;
  //i是计数器，代表读到了第几个包
  int i = 1;

  long offset;
  //c专用于while(1)中判断是否读到了文件末尾
  char c;
  if (argc != 2)
    {
      printf ("usage:%s file-name\n", argv[0]);
      exit (0);
    }

  fp = fopen (argv[1], "r");
  if (fp == NULL)
    {
      printf ("fopen error:%s\n", strerror (errno));
      exit (0);
    }

  pcap_p = &pcap_hdr;
  pkt_p = &pkt_hdr;
  eth_p = &eth_hdr;

//IP协议的结构体和声明
  struct ip ip_hdr;
  struct ip *ip_p;
  ip_p = &ip_hdr;

  //UDP协议的结构体和声明
  struct udphdr udp_hdr;
  struct udphdr *udp_p;
  udp_p = &udp_hdr;


    //ICMP协议的结构体和声明，自定义
  struct icmpv4_hdr icmp_hdr;
  struct icmpv4_hdr *icmp_p;
  icmp_p = &icmp_hdr;


  //ARP协议的结构体和声明
  struct ether_arp arp_hdr;
  struct ether_arp *arp_p;
  arp_p = &arp_hdr;

/*
判断链路类型，对于每个.cap 文件，该值只有一个，在该.cap文件的文件头里
linktype很重要，不同的网络环境下抓包，数据帧的帧头是不一样的。
比如，在局域网内抓包，linktype为1 ( DLT_EN10MB, Ethernet (10Mb) )，以太网的帧头就是两个网卡物理地址；
如果直接用ADSL拨号上线，则linktype为9 ( DLT_PPP, Point-to-point Protocol)，数据帧头为PPP协议。
*/
//以下我们假设都是以太网的，其他情况没有考虑，故改程序可能只适用于10Mb 以太网
  fread (pcap_p, sizeof (struct pcap_file_header), 1, fp);
  printf ("linktype is:%d\n", (*pcap_p).linktype);

  if (-1 == fseek (fp, sizeof (struct pcap_file_header), SEEK_SET))
    {
      printf ("1st fseek error:%s\n", strerror (errno));
    }
//因为后面两个结构体pcap_pkthdr和EtherPacket的每个值我们都读了，所以我们写个死循环
//就很简单了
  while (1)
    {
//判断文件是否结束
      c = fgetc (fp);
      if (c == EOF)
	break;
      else
	//如果没有到文件结尾，则指针往后移了1 byte，所以要移回来
	fseek (fp, -1, SEEK_CUR);

      printf ("--------------------%d packet--------------------\n", i);
      fread (pkt_p, sizeof (struct pcap_pkthdr), 1, fp);
      printf ("capture time is: %d.%d\n", (*pkt_p).ts.tv_sec,
	      (*pkt_p).ts.tv_usec);
      printf ("caplen is:%d bytes\n", (*pkt_p).caplen);
      printf ("len is:%d bytes\n", (*pkt_p).len);


      fread (eth_p, sizeof (struct EthernetPacket), 1, fp);
      printf ("destination mac address:%02X:%02X:%02X:%02X:%02X:%02X\n",
	      (*eth_p).MacDst[0], (*eth_p).MacDst[1], (*eth_p).MacDst[2],
	      (*eth_p).MacDst[3], (*eth_p).MacDst[4], (*eth_p).MacDst[5]);
      printf ("source mac address:%02X:%02X:%02X:%02X:%02X:%02X\n",
	      (*eth_p).MacSrc[0], (*eth_p).MacSrc[1], (*eth_p).MacSrc[2],
	      (*eth_p).MacSrc[3], (*eth_p).MacSrc[4], (*eth_p).MacSrc[5]);
      printf ("Packet Type is:0x%04x\n", ntohs ((*eth_p).PacketType));
//如果是IP协议，则继续解包；否则就不解了，直接走下一个包
      switch (ntohs ((*eth_p).PacketType))
	{
//开始IP解包
	case 0x0800:
//IP结构体和变量的声明放在上边
//注意ntohs，从网络顺序(big endian) 主机顺序(little edian)的转换
	  fread (ip_p, sizeof (struct ip), 1, fp);
	  printf ("IP type of servie is:%d\n", (*ip_p).ip_tos);
	  printf ("IP total length is:%d bytes\n", ntohs ((*ip_p).ip_len));
	  printf ("IP identification is:0x%04X\n", ntohs ((*ip_p).ip_id));
	  printf ("IP fragment offset field is:%d\n", (*ip_p).ip_off);
	  printf ("IP TTL is:%d\n", (*ip_p).ip_ttl);
	  printf ("IP protocol is:0x%02x\n", (*ip_p).ip_p);
	  printf ("IP checksum is:0x%04x\n", ntohs ((*ip_p).ip_sum));
	  printf ("IP source address is:%s\n",
		  inet_ntoa ((*ip_p).ip_src.s_addr));
	  printf ("IP destination address is:%s\n",
		  inet_ntoa ((*ip_p).ip_dst.s_addr));

//嵌套循环,对TCP UDP ICMP三种协议进行解包
	  switch ((*ip_p).ip_p)
	    {
	      //下面进行tcp解包
	    case 0x06:
	      fseek (fp,
		     (*pkt_p).caplen - sizeof (struct EthernetPacket) -
		     sizeof (struct ip), SEEK_CUR);
	      break;
	      //下面进行udp解包
	    case 0x11:
	      fread (udp_p, sizeof (struct udphdr), 1, fp);
	      printf ("UDP source port is:%d\n", ntohs ((*udp_p).source));
	      printf ("UDP destincation post is:%d\n", ntohs ((*udp_p).dest));
	      printf ("UDP length is:%d bytes\n", ntohs ((*udp_p).len));
	      printf ("UDP checksum is:0x%04x\n", ntohs ((*udp_p).check));
	      fseek (fp,
		     (*pkt_p).caplen - sizeof (struct EthernetPacket) -
		     sizeof (struct ip) - sizeof (struct udphdr), SEEK_CUR);
	      break;
	      //下面进行icmp解包,拷贝sniffer.c的代码
	    case 0x01:
fread (icmp_p, sizeof (struct icmphdr), 1, fp);
		  //offset设置为0，因为不是分析pcap抓的包
	      //void dump_icmp(char *packet, int offset);
	      dump_icmp(icmp_p, 0);
	         fseek (fp,
	         (*pkt_p).caplen - sizeof (struct EthernetPacket) -
	         sizeof (struct ip)- sizeof (struct icmphdr), SEEK_CUR);
	       
	      /*如果不对ICMP解包，则用这个fseek
	      fseek (fp,
		     (*pkt_p).caplen - sizeof (struct EthernetPacket) -
		     sizeof (struct ip), SEEK_CUR);
		     */
	      break;
	      //如果不是这三种协议，则不再解包
	    default:
	      fseek (fp,
		     (*pkt_p).caplen - sizeof (struct EthernetPacket) -
		     sizeof (struct ip), SEEK_CUR);
	      break;
	    }

	  break;
	  //下面进行arp协议解包
	case 0x0806:
	  fread (arp_p, sizeof (struct ether_arp), 1, fp);
	  printf ("ARP hardware type is:0x%04x\n",
		  ntohs ((*arp_p).ea_hdr.ar_hrd));
	  printf ("ARP protocol type is:0x%04x\n",
		  ntohs ((*arp_p).ea_hdr.ar_pro));
	  printf ("ARP hardware size is:%d\n", (*arp_p).ea_hdr.ar_hln);
	  printf ("ARP protocol size is:%d\n", (*arp_p).ea_hdr.ar_pln);
	  printf ("ARP Opcode is:0x%04x bytes\n",
		  ntohs ((*arp_p).ea_hdr.ar_op));
	  printf ("ARP sender mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		  (*arp_p).arp_sha[0], (*arp_p).arp_sha[1],
		  (*arp_p).arp_sha[2], (*arp_p).arp_sha[3],
		  (*arp_p).arp_sha[4], (*arp_p).arp_sha[5]);
	  printf ("ARP sender IP address: %d.%d.%d.%d\n", (*arp_p).arp_spa[0],
		  (*arp_p).arp_spa[1], (*arp_p).arp_spa[2],
		  (*arp_p).arp_spa[3]);
	  printf ("ARP target mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		  (*arp_p).arp_tha[0], (*arp_p).arp_tha[1],
		  (*arp_p).arp_tha[2], (*arp_p).arp_tha[3],
		  (*arp_p).arp_tha[4], (*arp_p).arp_tha[5]);
	  printf ("ARP target IP address: %d.%d.%d.%d\n", (*arp_p).arp_tpa[0],
		  (*arp_p).arp_tpa[1], (*arp_p).arp_tpa[2],
		  (*arp_p).arp_tpa[3]);
	  fseek (fp,
		 (*pkt_p).caplen - sizeof (struct EthernetPacket) -
		 sizeof (struct ether_arp), SEEK_CUR);
	  break;
//如果不是IP和arp协议，则直接进入下一个 包，不再解包        
	default:
         printf("Other protocol , Not IP/ARP\n");
	  fseek (fp, (*pkt_p).caplen - sizeof (struct EthernetPacket),
		 SEEK_CUR);
	  //printf ("next seek position is %d\n", ftell (fp));
	  break;
	}

      i++;
      printf ("------------------------------------------------\n", i);
    }

  fclose (fp);
}

void dump_icmp(struct icmpv4_hdr *packet, int offset)
{
 //  struct ipv4_hdr *ip;
   struct icmpv4_hdr *icmp;
   u_short size_ip, size_icmp, size_buf;

   size_ip = IPV4_H;
   size_icmp = ICMPV4_H; /* base ICMP header lenght */
   size_buf = 0;


//下面强制类型转换
icmp = (struct icmpv4_hdr *) packet; 

   printf("[ ICMP ]----------------------------------------\n");
   printf("ICMP Type: ");

   switch((icmp->icmp_type))
     {
      case 0:
        printf("Echo reply\n");
        size_icmp+=4;
        break;
      case 3:
        printf("Dest. unreach: ");
        size_icmp+=4;
        switch (icmp->icmp_code)
          {
           case 0:
             printf("Network Unreachable\n");
             break;
           case 1:
             printf("Host Unreachable\n");
             break;
           case 2:
             printf("Protocol Unreachable\n");
             break;
           case 3:
             printf("Port Unreachable\n");
             break;
           case 4:
             printf("Fragmentation neded (DF)\n");
             break;
           case 5:
             printf("Source route failed\n");
             break;
           case 6:
             printf("Destination network unknown\n");
             break;
           case 7:
             printf("Destination host unknown\n");
             break;
           case 8:
             printf("Source host isolated\n");
             break;
           case 9:
             printf("Destination network administratively prohibited\n");
             break;
           case 10:
             printf("Destination host administratively prohibited\n");
             break;
           case 11:
             printf("Network unreacjable(tOS)\n");
             break;
           case 12:
             printf("Host Unreachable (tOS)\n");
             break;
           case 13:
             printf("Communication administratively prohibited\n");
             break;
           case 14:
             printf("Host precedence violation\n");
             break;
           case 15:
             printf("Precedence cutoff in effect\n");
             break;
           default:
             printf("Unknown - error?\n");
             break;
          }
        break;
      case 4:
        printf("Source quench\n");
        size_icmp+=4;
        break;
      case 5:
        printf("Redirect: ");
        size_icmp+=4;
        switch(icmp->icmp_code)
          {
           case 0:
             printf("Redirect for network\n");
             break;
           case 1:
             printf("Redirect for host\n");
             break;
           case 2:
             printf("Redircet for tos & network\n");
             break;
           case 3:
             printf("Redirect for tos & host\n");
             break;
           default:
             printf("Unknown - error?\n");
             break;
          }
        break;
      case 8:
        printf("Echo request\n");
        size_icmp+=4;
        break;
      case 11:
        printf("Time exceeded: ");
        size_icmp+=4;
        switch (icmp->icmp_code)
          {
           case 0:
             printf("TTL (0) during transit\n");
             break;
           case 1:
             printf("TTL (0) during reassembly\n");
             break;
           default:
             printf("Unknown - error?\n");
             break;
          }
        break;
      case 12:
        printf("Parameter problem: ");
        switch (icmp->icmp_code)
          {
           case 0:
             printf("IP header bad\n");
             break;
           case 1:
             printf("Requiring option missing\n");
             break;
          }
        break;
      case 13:
        printf("Timestamp\n");
        size_icmp+=16;
        break;
      case 14:
        printf("Timestamp reply\n");
        size_icmp+=16;
        break;
      case 15:
        printf("Information\n");
        break;
      case 16:
        printf("Information reply\n");
        break;
      case 17:
        printf("Address mask\n");
        size_icmp+=8;
        break;
      case 18:
        printf("Address mask reply\n");
        size_icmp+=8;
        break;
      default:
        printf("%i\n", icmp->icmp_type);
        break;
     }
}

