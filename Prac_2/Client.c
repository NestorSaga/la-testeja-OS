//Authors: Sofian Ben Ayata && Erik Espuñes Juberó

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    char name_fifo_server[32] = "/tmp/fifo_server", name_fifo_client[32] = "/tmp/fifo_client", filename[100];
    int fifo_server=-1, fifo_client=-1, file=-1;
    int choice = 0;
    char *buf = "OK";

    //We open the pipes
    fifo_client = open(name_fifo_client, O_RDWR);
    if (fifo_client < 0) {
        perror("Error opening fifo_client");
        exit(-1);
    }
    fifo_server = open(name_fifo_server, O_RDWR);
    if (fifo_server < 0) {
        perror("Error opening fifo_server");
    }
    //Read the program chosen and the filename
    read(fifo_client, &choice, 1);
    read(fifo_client, filename, sizeof(filename));
    printf("Program chosen  is : %d\n", choice);
    if (choice == 1) {
        //if it's program 1 we open destination file
        if ((file = open(filename, O_WRONLY|O_CREAT|O_TRUNC)) < 0) {
            buf = "ERROR";
            write(fifo_client, buf, sizeof(buf));
            perror("Error! opening destination file from Client.c");
            exit(-1);
        }
        //we write to the destination file what it's inside the pipe
        char line[256]="";
        int size;
        while (((size=read(fifo_client, line, sizeof(line))) > 0)) {
            write(file, line, size* sizeof(char));
            if(size<256)
                break;
        }
        printf("%s\n", buf);
        //We tell the server if the program was successful or wrong
        write(fifo_server, buf, sizeof(buf));
    } else {
        //else it's program 2 and we open source file
        if ((file = open(filename, O_RDONLY)) < 0) {
            buf = "ERROR";
            write(fifo_server, buf, sizeof(buf));
            perror("Error! opening source file from Client.c");
            exit(-1);
        }
        //we write to the server pipe, what's in the source file
        char line[256]="";
        int size;
        while ((size=read(file, line, sizeof(line))) > 0) {
            write(fifo_server, line, size* sizeof(char));
        }
        //Finally we wait to the Server result
        buf = malloc(10 * sizeof(char));
        read(fifo_client, buf, 10 * sizeof(char));
        printf("\n ***Reply from server is %s***\n", buf);
    }
    close(file);
    close(fifo_server);
    close(fifo_client);
    return 0;
}