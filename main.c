/*

    juni task packet sniffer

*/

#include "common.h"
#include "data_structure.h"

volatile sig_atomic_t stop = 0;

void sig_handler(int signum);
int create_socket(const char * device_name);
FILE* create_logfile(const char* file_name);
void reset_packet(struct sockaddr_in* src, struct sockaddr_in* dest, uint32_t saddr, uint32_t daddr);
void print_help_message();

int main (int argc, char *argv[]) {

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) { // to make such signal
		perror ("\ncan't catch SIGUSR1\n");
    }

    int pid = getpid();
    printf ("getpid(): %d\n", pid);

    char char_buffer[BUFFER_SIZE];
    memset(char_buffer, 0, BUFFER_SIZE);
    sprintf(char_buffer, "%d", pid);
    char * dfifo = DFIFO;

    mkfifo(dfifo, 0666);
    printf("dfifo created!\n");

    int fd = open(dfifo, O_WRONLY);
    printf("dfifo opened!\n");

    write(fd, char_buffer, BUFFER_SIZE);
    printf("written in dfifo\n");
    sleep(TIMEOUT);

    char * cfifo = CFIFO;
    int fc = open(cfifo, O_RDONLY);
    memset(char_buffer, 0, BUFFER_SIZE);
    read(fc, char_buffer, BUFFER_SIZE);
    printf("Received:\n%s\n", char_buffer);

    // take name of network interface device
    printf("Starting...\n");
    const char * iface;
    if (argc < 2) {
        printf ("You can enter network device as first argument!\n");
        iface = DEFAULT_IFACE;
    } else {
        printf ("argv[1] = %s\n", argv[1]);
        iface = argv[1];
    }

    int sock_raw = create_socket(iface);

    //     --help
    printf ("Usage: # %s eth0\n", argv[0]);

    // create logfile
    FILE * logfile = create_logfile(LOGFILE);
    fprintf (logfile, "Network device %s:\n", iface);

    // create table of ips
    ips_structure * ips = ips_structure_create();
    if (ips == NULL) {
        perror ("Can't create ips_structure!");
        exit(EXIT_FAILURE);
    }

    // start sniff
    //     start
    //     stop = stop sniff save table to log file
    //     stat iface = print ip table for given iface
    //     show ip = print matching ip from table
    //     select iface

    unsigned char * buffer = (unsigned char *) malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        printf ("Can't create buffer!");
        //return EXIT_FAILURE;
        exit (EXIT_FAILURE);
    }


    //memset(char_buffer, 0, BUFFER_SIZE); // redirect stdout
    //freopen("/dev/null", "a", stdout);
    //setbuf(stdout, char_buffer);

    //struct sockaddr saddr;
    struct sockaddr_in source, dest;
    ipaddr_node * ex_node;

    int choice = -1;
    //for(;;) {
    while(choice != STOP) {

        //print_help_message();

        //scanf("%d", &choice);
        memset(char_buffer, 0, BUFFER_SIZE);
        read(fc, char_buffer, BUFFER_SIZE);
        choice = atoi(char_buffer);
        //printf ("choice = %d\n", choice);

        switch (choice) {
            case STAT:
                memset(char_buffer, 0, BUFFER_SIZE);
                ips_structure_print(ips);
                write(fd, char_buffer, BUFFER_SIZE);
                break;
            case SHOW_IP: {
                //char line[20];
                //printf ("Enter ip (e.g. 127.0.0.1): ");
                sprintf(char_buffer, MSG_IP);
                write(fd, char_buffer, BUFFER_SIZE);
                memset(char_buffer, 0, BUFFER_SIZE);
                read(fc, char_buffer, BUFFER_SIZE);
                //sscanf(line, "%19[^\n]", char_buffer);
                ipaddr_node * node = ipaddr_create(inet_addr(char_buffer));
                if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                    memset(char_buffer, 0, BUFFER_SIZE);
                    ipaddr_print(ex_node);
                    write(fd, char_buffer, BUFFER_SIZE);
                } else {
                    //printf("%s - no such ip occured\n", line);
                    sprintf(char_buffer, INVALID_IP);
                    write(fd, char_buffer, BUFFER_SIZE);
                }
            }
                break;
            case SELECT_IFACE:
                break;
            case HELP:
                //print_help_message();
                break;

            case CONT:
            case START:
                while (!stop) {
                    //int saddr_size = sizeof(struct sockaddr);
                    //int data_size = recvfrom(sock_raw , buffer , BUFFER_SIZE , 0 , &saddr , (socklen_t*)&saddr_size);
                    int data_size = recv(sock_raw , buffer , BUFFER_SIZE , 0);
                    if (data_size < 0) {
                        printf ("Recvfrom error , failed to get packets\n");
                        exit (EXIT_FAILURE);
                    }
                    struct iphdr *iph = (struct iphdr *)(buffer  + sizeof(struct ethhdr) );
                    reset_packet(&source, &dest, iph->saddr, iph->daddr);
                    // memset (&source, 0, sizeof(source));
                    // source.sin_addr.s_addr = iph->saddr;
                    // memset (&dest, 0, sizeof(dest));
                    // dest.sin_addr.s_addr = iph->daddr;
                    ipaddr_node * node = ipaddr_create(iph->daddr);
                    //printf("node = %p\n", node);
                    if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                        ipaddr_increment_total_packets(ex_node);
                    } else {
                        ips_structure_insert(ips, node);
                        ipaddr_increment_total_packets(node);
                        //printf("ips = %p\n", ips);
                    }
                }
                stop = 0;
            default:
                break;
            }
    }

    //ips_structure_print(ips);
    ips_structure_persist(ips, logfile);
    //fflush(stdout);

    //char * msg = "Hello, World!";
    //sprintf(char_buffer, msg);
    //printf("char_buffer is %s\n", char_buffer);
    //write(fd, char_buffer, BUFFER_SIZE);
    //printf("written in fifo\n");

    ips_structure_destroy(ips);
    free(buffer);
    close (sock_raw);
    fclose(logfile);
    close(fc);
    close(fd);
    //printf("closed dfifo\n");
    unlink(dfifo);

    //printf ("Finished\n");
    exit (EXIT_SUCCESS);
}

int create_socket(const char * iface) {
    int socket_raw = socket( AF_PACKET , SOCK_RAW ,
        htons(ETH_P_ALL));
    setsockopt(socket_raw , SOL_SOCKET ,
        SO_BINDTODEVICE , iface , strlen(iface)+ 1 );
    if(socket_raw < 0){
        perror ("Socket Error");
        exit(EXIT_FAILURE);
    }
    return socket_raw;
}

FILE* create_logfile(const char* file_name) {
    FILE * fp;
    fp = fopen (file_name, "w");
    if (fp == NULL) {
        perror ("Unable to create file.");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void reset_packet(struct sockaddr_in* src, struct sockaddr_in* dest,
     uint32_t saddr, uint32_t daddr) {
    memset (src, 0, sizeof(struct sockaddr_in));
    src->sin_addr.s_addr = saddr;
    memset (dest, 0, sizeof(struct sockaddr_in));
    dest->sin_addr.s_addr = daddr;
}

void print_help_message() {
    printf ("You have fallowing options to continue:\n");
    printf ("%d to start sniffing\n", START);
    printf ("%d to stop\n", STOP);
    printf ("%d to continue\n", CONT);
    printf ("%d to see all stats\n", STAT);
    printf ("%d to see given ip stats\n", SHOW_IP);
    printf ("%d to select interface\n", SELECT_IFACE);
    printf ("%d to print help message\n", HELP);
}

void sig_handler(int signum) {
    stop = 1;
}
