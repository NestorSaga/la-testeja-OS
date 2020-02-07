//Authors: Sofian Ben Ayata && Erik Espuñes Juberó

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    char name_fifo_server[32] = "/tmp/fifo_server", name_fifo_client[32] = "/tmp/fifo_client";
    int fifo_server=-1, fifo_client=-1, file=-1;
    char srcfilename[100]="", dstfilename[100]="";
    int choice=0;

    //Ask for source filename, destination filename and which program use
    printf("Can you give me a source filename, please? ");
    scanf("%s", srcfilename);
    printf("Can you give me a destination filename, please? ");
    scanf("%s", dstfilename);

    printf("Choose the request to be sent to server from options below");
    printf("\n\t\t Enter 1 in order to send the content to Client.c\n \
                Enter 2 in order to recieve the content from Client.c\n");
    scanf("%d", &choice);

    //delete the previous pipes
    unlink(name_fifo_server);
    unlink(name_fifo_client);
    if (choice == 1 || choice == 2) {
        //if the program chosen ins between 1 or 2 means that are the correct values to choose the program, and creates the two pipes, one for to read from server and the other to read from client
        fifo_server = mkfifo(name_fifo_server, 0666);
        if (fifo_server < 0) {
            perror("Unable to create a fifo_server");
            exit(-1);
        }
        fifo_client = mkfifo(name_fifo_client, 0666);
        if (fifo_client < 0) {
            perror("Unable to create a fifo_client");
            exit(-1);
        }
        //We open the pipes
        fifo_server = open(name_fifo_server, O_RDWR);
        if (fifo_server < 0) {
            perror("Error in opening fifo_server");
            exit(-1);
        }
        fifo_client = open(name_fifo_client, O_RDWR);

        //We write the chosen program
        write(fifo_client, &choice, 1);

        if (fifo_client < 0) {
            perror("Error in opening fifo_client");
            exit(-1);
        }
        if (choice == 1) {
            //If program is 1 we open the source file and send the destination filename to the client
            if ((file = open(srcfilename, O_RDONLY)) < 0) {
                perror("Error! opening source file from Server.c");
                exit(-1);
            }
            write(fifo_client, dstfilename, sizeof(dstfilename));
            //We write to the client pipe the information that it's in the source file
            char line[256];
            int size;
            while ((size=read(file, line, sizeof(line))) > 0) {
                write(fifo_client, line, size* sizeof(char));
            }
            //We wait for the result from client
            char *buf = malloc(10 * sizeof(char));
            read(fifo_server, buf, 10 * sizeof(char));
            printf("\n ***Reply from client is %s***\n", buf);
        } else {
            //else it's the other program and we open destination file and we send to the client the source filename
            if ((file = open(dstfilename, O_WRONLY|O_CREAT|O_TRUNC)) < 0) {
                perror("Error! opening destination file from Server.c");
                exit(-1);
            }

            write(fifo_client, srcfilename, sizeof(srcfilename));
            //We write to the destination file what's inside the server pipe

            char line[256]="";
            int size;
            while ((size=read(fifo_server, line, sizeof(line))) > 0) {
                write(file, line, size* sizeof(char));
                if(size<256)
                    break;
            }
            write(fifo_client, "OK", 10 * sizeof(char));
            printf("OK\n");
        }
        close(file);
        close(fifo_server);
        close(fifo_client);
    } else {
        perror("Error! the number must be 1 or 2");
        exit(1);
    }
    return 0;
}