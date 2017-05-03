#include "common.h"

void print_help_message();

int main(int argc, char* argv[]) {

    // kill -USR1 main.pid
    // kill(pid, SIGUSR1);

    char * dfifo = DFIFO;
    char * cfifo = CFIFO;
    char char_buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    /* open, read, and display the message from the FIFO */
    //int fd = open(myfifo, O_RDONLY | O_NONBLOCK);
    int fd = open(dfifo, O_RDONLY);
    read(fd, char_buffer, BUFFER_SIZE);
    printf("Received:\n%s\n", char_buffer);
    int dpid = atoi(char_buffer);
    printf("Daemon pid is %d\n", dpid);


    mkfifo(cfifo, 0666);
    printf("cfifo created!\n");
    int fc = open(cfifo, O_WRONLY);
    printf("cfifo opened!\n");
    write(fc, char_buffer, BUFFER_SIZE);

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
            memset(char_buffer, 0, BUFFER_SIZE);
            read(fd, char_buffer, BUFFER_SIZE);
            printf("%s\n", char_buffer);
            fgets(command, sizeof(command), stdin);
            write(fc, command, BUFFER_SIZE);
            memset(char_buffer, 0, BUFFER_SIZE);
            read(fd, char_buffer, BUFFER_SIZE);
            printf("%s\n", char_buffer);
        }
        else if (!strncmp(command, MSG_SELECT_IFACE, strlen(MSG_SELECT_IFACE))) {
            sprintf(char_buffer, "%d", SELECT_IFACE);
            write(fc, char_buffer, BUFFER_SIZE);
            printf("Enter interface name:\n");
        }
        else if (!strncmp(command, MSG_STAT, strlen(MSG_STAT))) {
            printf("Current IP stats\n");
            memset(char_buffer, 0, BUFFER_SIZE);
            sprintf(char_buffer, "%d", STAT);
            write(fc, char_buffer, BUFFER_SIZE);
            memset(char_buffer, 0, BUFFER_SIZE);
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
            sprintf(char_buffer, "%d", STOP);
            write(fc, char_buffer, BUFFER_SIZE);
            break; //break infinite loop
        } else {
            printf("Unknown command: %s\n", command);
        }
        sleep(TIMEOUT);
    }
//    sleep(5);
    close(fd);
    close(fc);
    printf("closed cfifo\n");
    unlink(cfifo);
	exit(EXIT_SUCCESS);
}

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
