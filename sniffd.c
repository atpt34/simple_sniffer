/*
    Simple sniffer.

    Creates daemon process that sniffs ip packets.
    Takes network interface device name as an argument.
    If no arguments given opens DEFAULT_IFACE.
    After entering main loop starts fill ips_structure with data,
    and only after cli sends signal it
    reads from cpipe following commands:
    start, continue - continue sniffing;
    stop            - terminate daemon & save ips_structure data;
    stat            - writes to dfifo current ips_structure data.
    show            - search ips_structure data for given ip address;
    select          - change interface for sniffing;
 */

#include "common.h"
#include "data_structure.h"

// Variable to stop sniff loop.
static volatile sig_atomic_t stop = FALSE;

static void daemonize();
static void sig_handler(int signum);
static int create_socket(const char * device_name);
static struct iphdr* get_ipheader(int socket_raw, unsigned char* buffer);
static FILE* create_logfile(const char* file_name);
static unsigned char * create_buffer(size_t);

int main (int argc, char *argv[]) {

    daemonize();

    // set up handler
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		//perror ("\ncan't catch SIGUSR1\n");
        exit (EXIT_FAILURE);
    }



    // take name of network interface device
    //printf("Starting...\n");
    const char * iface;
    if (argc < 2) {
        //printf ("Usage: # %s <interface>\n", argv[0]);
        iface = DEFAULT_IFACE;
    } else {
        //printf ("argv[1] = %s\n", argv[1]);
        iface = argv[1];
    }

    // create interface socket
    int sock_raw = create_socket(iface);

    // create logfile
    FILE * logfile = create_logfile(LOGFILE);
    fprintf (logfile, "Network device %s:\n", iface);

    // create table of ips
    ips_structure * ips = ips_structure_create();
    if (ips == NULL) {
        //perror ("Can't create ips_structure!");
        exit(EXIT_FAILURE);
    }

    // create daemon & cli fifos
    char char_buffer[BUFFER_SIZE];
    char * dfifo = DFIFO;
    char * cfifo = CFIFO;
    int fd, fc;
    mkfifo(dfifo, 0666); // S_IFIFO 0644

    unsigned char * buffer = create_buffer(BUFFER_SIZE);

    //struct sockaddr saddr;
    //struct sockaddr_in source, dest;
    ipaddr_node * ex_node;

    int new_conn = TRUE;
    int choice = START;

    do {
        switch (choice) {
            case STAT:
                memset(char_buffer, 0, BUFFER_SIZE);
                ips_structure_print_buf(ips, char_buffer);
                write(fd, char_buffer, BUFFER_SIZE);
                break;
            case SHOW_IP: {
                read(fc, char_buffer, BUFFER_SIZE);
                ipaddr_node * node = ipaddr_create(inet_addr(char_buffer));
                if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                    memset(char_buffer, 0, BUFFER_SIZE);
                    ipaddr_print_buf(ex_node, char_buffer);
                    write(fd, char_buffer, BUFFER_SIZE);
                } else {
                    sprintf(char_buffer, INVALID_IP);
                    write(fd, char_buffer, BUFFER_SIZE);
                }
            }
                break;
            case SELECT_IFACE:
                //TODO start sniffing new interface in separate thread
                read(fc, char_buffer, BUFFER_SIZE);
                int socket_raw = socket( AF_PACKET, SOCK_RAW,
                    htons(ETH_P_ALL));
                setsockopt(socket_raw , SOL_SOCKET ,
                    SO_BINDTODEVICE , char_buffer , strlen(char_buffer)+ 1 );
                if(socket_raw < 0){
                    sprintf(char_buffer, INVALID_IFACE);
                    write(fd, char_buffer, BUFFER_SIZE);
                    //perror ("Socket Error");
                } else {
                    sprintf(char_buffer, VALID_IFACE);
                    write(fd, char_buffer, BUFFER_SIZE);
                    ips_structure_destroy(ips);
                    ips = ips_structure_create();
                    close(sock_raw);
                    sock_raw = socket_raw;
                }
                break;

            case CONT:
            case START:
                while (!stop) {
                    struct iphdr *iph = get_ipheader(sock_raw, buffer);
                    ipaddr_node * node = ipaddr_create(iph->daddr);
                    //printf("node = %p\n", node);
                    if ( (ex_node = ips_structure_query(ips, node)) != NULL ) {
                        ipaddr_increment_total_packets(ex_node);
                        ipaddr_destroy(node);
                    } else {
                        ips_structure_insert(ips, node);
                        ipaddr_increment_total_packets(node);
                        //printf("ips = %p\n", ips);
                    }
                }
                stop = FALSE;
            default:
                break;
            }

            if (new_conn) {

                fd = open(dfifo, O_WRONLY);
                //sleep(TIMEOUT);
                fc = open(cfifo, O_RDONLY);
                new_conn = FALSE;
            }

            read(fc, char_buffer, BUFFER_SIZE);
            choice = atoi(char_buffer);
            //printf ("choice = %d\n", choice);

    } while (choice != STOP);

    //ips_structure_print(ips);
    ips_structure_persist(ips, logfile);

    // clean up
    ips_structure_destroy(ips);
    free(buffer);
    close(sock_raw);
    fclose(logfile);
    close(fc);
    close(fd);
    unlink(dfifo);

    //printf ("Finished\n");
    exit(EXIT_SUCCESS);
}

// Creates new socket for a given interface.
static int create_socket(const char * iface) {
    int socket_raw = socket( AF_PACKET, SOCK_RAW,
        htons(ETH_P_ALL));
    setsockopt(socket_raw , SOL_SOCKET ,
        SO_BINDTODEVICE , iface , strlen(iface)+ 1 );
    if(socket_raw < 0){
        //perror ("Socket Error");
        exit(EXIT_FAILURE);
    }
    return socket_raw;
}

// Opens logfile to save ips_structure data.
// Requires name of the log file.
static FILE* create_logfile(const char* file_name) {
    FILE * fp;
    fp = fopen (file_name, "w");
    if (fp == NULL) {
        //perror ("Unable to create file.");
        exit(EXIT_FAILURE);
    }
    return fp;
}

// Creates buffer to store sniffed packets.
static unsigned char * create_buffer(size_t sz) {
    unsigned char * buffer = (unsigned char *) malloc(sz);
    if (buffer == NULL) {
        //printf ("Can't create buffer!");
        exit (EXIT_FAILURE);
    }
    return buffer;
}

// Locates pointer to start of ip header.
static struct iphdr* get_ipheader(int socket_raw, unsigned char* buffer) {
    int data_size = recv(socket_raw , buffer , BUFFER_SIZE , 0);
    if (data_size < 0) {
        //printf ("Recvfrom error , failed to get packets\n");
        exit (EXIT_FAILURE);
    }
    return (struct iphdr *)(buffer  + sizeof(struct ethhdr));
}

// Handler for cli program signal.
// Terminates sniffing loop.
static void sig_handler(int signum) {
    stop = TRUE;
}

// Turns this process to daemon process.
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

    char str[LINE_SIZE];
    /* Get and format PID */
    sprintf(str,"%d\n", getpid());
    /* write pid to lockfile */
    write(pidFilehandle, str, strlen(str));
    //write(pidFilehandle, &getpid(), sizeof(int));
}
