/*

    juni task packet sniffer.

*/

#include "common.h"
#include "data_structure.h"

volatile sig_atomic_t stop = 0;

static void daemonize();
void sig_handler(int signum);
int create_socket(const char * device_name);
FILE* create_logfile(const char* file_name);
void reset_addresses(struct sockaddr_in* src, struct sockaddr_in* dest, uint32_t saddr, uint32_t daddr);
void print_help_message();

int main (int argc, char *argv[]) {

    daemonize();

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) { // to make such signal
		perror ("\ncan't catch SIGUSR1\n");
    }

    char char_buffer[BUFFER_SIZE];
    char * dfifo = DFIFO;
    char * cfifo = CFIFO;
    int fd, fc;

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

    //struct sockaddr saddr;
    struct sockaddr_in source, dest;
    ipaddr_node * ex_node;

    int new_conn = 0;
    int choice = START;
    //for(;;) {
    do {
        switch (choice) {
            case STAT:
                memset(char_buffer, 0, BUFFER_SIZE);
                ips_structure_print_buf(ips, char_buffer);
                write(fd, char_buffer, BUFFER_SIZE);
                break;
            case SHOW_IP: {
                memset(char_buffer, 0, BUFFER_SIZE);
                read(fc, char_buffer, BUFFER_SIZE);
                ipaddr_node * node = ipaddr_create(inet_addr(char_buffer));
                if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                    memset(char_buffer, 0, BUFFER_SIZE);
                    //ipaddr_print(ex_node);
                    ipaddr_print_buf(ex_node, char_buffer);
                    write(fd, char_buffer, BUFFER_SIZE);
                } else {
                    sprintf(char_buffer, INVALID_IP);
                    write(fd, char_buffer, BUFFER_SIZE);
                }
            }
                break;
            case SELECT_IFACE:
                //TODO interface for sniffing
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
                    reset_addresses(&source, &dest, iph->saddr, iph->daddr);
                    ipaddr_node * node = ipaddr_create(iph->daddr);
                    //printf("node = %p\n", node);
                    if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                        ipaddr_increment_total_packets(ex_node);
                        //ipaddr_destroy(node);
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

            if (!new_conn)  {
                mkfifo(dfifo, 0666);
                printf("dfifo created!\n");
                fd = open(dfifo, O_WRONLY);
                printf("dfifo opened!\n");
                sprintf(char_buffer, "hello world!");
                write(fd, char_buffer, BUFFER_SIZE);
                printf("written in dfifo\n");
                sleep(TIMEOUT);
                fc = open(cfifo, O_RDONLY);
                memset(char_buffer, 0, BUFFER_SIZE);
                read(fc, char_buffer, BUFFER_SIZE);
                printf("Received:\n%s\n", char_buffer);
                new_conn = 1;

            }

            memset(char_buffer, 0, BUFFER_SIZE);
            read(fc, char_buffer, BUFFER_SIZE);
            choice = atoi(char_buffer);
            //printf ("choice = %d\n", choice);

    } while (choice != STOP);

    //ips_structure_print(ips);
    ips_structure_persist(ips, logfile);
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

void reset_addresses(struct sockaddr_in* src, struct sockaddr_in* dest,
     uint32_t saddr, uint32_t daddr) {
    memset (src, 0, sizeof(struct sockaddr_in));
    src->sin_addr.s_addr = saddr;
    memset (dest, 0, sizeof(struct sockaddr_in));
    dest->sin_addr.s_addr = daddr;
}

void sig_handler(int signum) {
    stop = 1;
}

static void daemonize()  {
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir(RUN_DIR);

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
        close (x);
    }


    /* Ensure only one copy */
    int pidFilehandle = open(PIDFILE, O_RDWR|O_CREAT, 0600);
    if (pidFilehandle == -1 ) {
        /* Couldn't open lock file */
        exit(EXIT_FAILURE);
    }
    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
        /* Couldn't get lock on lock file */
        exit(EXIT_FAILURE);
    }

    char str[32];
    /* Get and format PID */
    sprintf(str,"%d\n", getpid());
    /* write pid to lockfile */
    write(pidFilehandle, str, strlen(str));
    //write(pidFilehandle, &getpid(), sizeof(int));
}
