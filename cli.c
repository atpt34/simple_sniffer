/*
    Command line interface to sniffer daemon.

    Opens communication to sniffer daemon program by
    reading PIDFILE with daemon pid, then verifies
    if daemon is running, if not starts the daemon.
    After establishing connection with pipes it
    read following user commands:
    start, continue - start sniffing;
    stop            - stop sniffing;
    stat            - prints ip packets statistic;
    show            - prints given ip number of packets;
    select          - select new interface for sniffing;
    help            - prints help message;
    exit            - terminates cli & sniffer daemon;
 */
#include "common.h"

static void print_help_message();

int main(int argc, char* argv[]) {

    char * dfifo = DFIFO;
    char * cfifo = CFIFO;
    char char_buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Try to get file with pid of the running daemon.
    FILE* fp = fopen(PIDFILE, "r");
    if (fp == NULL) {
        printf("Can't open pidfile\n");
        exit(EXIT_FAILURE);
    }
    if (!fgets(char_buffer, BUFFER_SIZE, fp)) {
        printf("Can't read pidfile\n");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    int dpid = atoi(char_buffer);
    printf("PIDFILE data %d\n", dpid);

    // check first if daemon is working
    //    if not start the daemon
    //    else continue
    if (kill(dpid, 0) == 0) {
        printf ("Process is running(maybe not sniffd!) or a zombie!\n");
    } else if (errno == ESRCH) {
        printf ("No such process with the given id is running! Starting sniffer!\n");
        // start sniffer daemon
        char *argsv[] = {DAEMON_NAME, NULL};
        execv (DAEMON_NAME, argsv);
        perror ("execv() failed");
        printf ("%s started!\n", DAEMON_NAME);
        exit(EXIT_FAILURE);
    } else {
        printf ("Some other error... use perror() or strerror(errno) to report");
        exit(EXIT_FAILURE);
    }

    kill(dpid, SIGUSR1);  // notify daemon
    mkfifo(cfifo, 0666);
    //printf("cfifo created!\n");

    //sleep(TIMEOUT);
    int fd = open(dfifo, O_RDONLY);
    printf("dfifo opened!\n");


    //read(fd, char_buffer, BUFFER_SIZE);
    //printf("Received:\n%s\n", char_buffer);

    //sleep(TIMEOUT - 1);
    int fc = open(cfifo, O_WRONLY);
    printf("cfifo opened!\n");
    //write(fc, char_buffer, BUFFER_SIZE);

    print_help_message();

    for(;;) {

        printf ("> ");
        fgets(command, sizeof(command), stdin);

        if (!strncmp(command, MSG_START, strlen(MSG_START))) {
            printf("Starting sniffer\n");
            sprintf(char_buffer, "%d", START);
            write(fc, char_buffer, BUFFER_SIZE);
        }
        else if (!strncmp(command, MSG_CONT, strlen(MSG_CONT))) {
            printf("Continue sniffing...\n");
            sprintf(char_buffer, "%d", CONT);
            write(fc, char_buffer, BUFFER_SIZE);
        }
        else if (!strncmp(command, MSG_SHOW_IP, strlen(MSG_SHOW_IP))) {
            sprintf(char_buffer, "%d", SHOW_IP);
            write(fc, char_buffer, BUFFER_SIZE);
            printf("Enter ip address:\n");
            printf(MSG_IP);
            fgets(command, sizeof(command), stdin);
            write(fc, command, BUFFER_SIZE);
            read(fd, char_buffer, BUFFER_SIZE);
            printf("%s\n", char_buffer);
        }
        else if (!strncmp(command, MSG_SELECT_IFACE, strlen(MSG_SELECT_IFACE))) {
            memset(char_buffer, 0, BUFFER_SIZE);
            sprintf(char_buffer, "%d", SELECT_IFACE);
            write(fc, char_buffer, BUFFER_SIZE);
            printf("Enter interface name(e.g. eth0): ");
            fgets(command, sizeof(command), stdin);
            write(fc, command, BUFFER_SIZE);
            read(fd, char_buffer, BUFFER_SIZE);
            printf("%s\n", char_buffer);
        }
        else if (!strncmp(command, MSG_STAT, strlen(MSG_STAT))) {
            printf("Current IP stats\n");
            memset(char_buffer, 0, BUFFER_SIZE);
            sprintf(char_buffer, "%d", STAT);
            write(fc, char_buffer, BUFFER_SIZE);
            //memset(char_buffer, 0, BUFFER_SIZE);
            read(fd, char_buffer, BUFFER_SIZE);
            printf("%s\n", char_buffer);
        }
        else if (!strncmp(command, MSG_HELP, strlen(MSG_HELP))) {
            //printf("Help\n");
            print_help_message();
        }
        else if (!strncmp(command, MSG_STOP, strlen(MSG_STOP))) {
            printf("Stoping sniffer...\n");
            kill(dpid, SIGUSR1);
            //sprintf(char_buffer, "%d", STOP);
            //write(fc, char_buffer, BUFFER_SIZE);
        }
        else if (!strncmp(command, MSG_EXIT, strlen(MSG_EXIT))) {
            printf ("Goodbye!\n");
            kill(dpid, SIGUSR1);
            sprintf(char_buffer, "%d", STOP);  // START
            write(fc, char_buffer, BUFFER_SIZE);
            break; //break infinite loop
        } else {
            printf("Unknown command: %s\n", command);
        }
        sleep(TIMEOUT);
    }

    // clean up
    close(fd);
    close(fc);
    printf("closed cfifo\n");
    unlink(cfifo);
	exit(EXIT_SUCCESS);
}

// Prints available cli user commands.
void print_help_message() {
    printf ("You have fallowing options:\n");
    printf ("%s - to start sniffing\n", MSG_START);
    printf ("%s - to stop\n", MSG_STOP);
    printf ("%s - to continue\n", MSG_CONT);
    printf ("%s - to see all stats\n", MSG_STAT);
    printf ("%s - to see given ip stats\n", MSG_SHOW_IP);
    printf ("%s - to select interface\n", MSG_SELECT_IFACE);
    printf ("%s - to exit cli\n", MSG_EXIT);
    printf ("%s - to print help message\n", MSG_HELP);
}
