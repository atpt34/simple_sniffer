#ifndef COMMON_H_
#define COMMON_H_

#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //strlen
#include <stdint.h>
#include <signal.h>
#include <fcntl.h> /* pipe */
#include <sys/stat.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netinet/if_ether.h>  //For ETH_P_ALL
#include <net/ethernet.h>  //For ether_header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DAEMON_NAME "sniffd"
#define DEFAULT_IFACE "eth0"
#define BUFFER_SIZE 1024
#define LINE_SIZE 32
#define LINE_FORMAT "%s\t%u\n"
#define TIMEOUT 2
#define RUN_DIR "/tmp/"
#define LOGFILE "/tmp/log.txt"
#define PIDFILE "/tmp/sniffd.pid"
#define DFIFO "/tmp/sniff_dfifo"
#define CFIFO "/tmp/sniff_cfifo"

#define START 0
#define STOP 1
#define CONT 2
#define SHOW_IP 3
#define SELECT_IFACE 4
#define STAT 5
#define HELP 6

#define MSG_START "start"
#define MSG_STOP "stop"
#define MSG_CONT "cont"
#define MSG_SHOW_IP "show"
#define MSG_SELECT_IFACE "select"
#define MSG_STAT "stat"
#define MSG_HELP "help"
#define MSG_EXIT "exit"
#define MSG_IP "Enter ip (e.g. 127.0.0.1): "
#define INVALID_IP "No such ip!\n"


#endif /* #ifndef COMMON_H_ */
