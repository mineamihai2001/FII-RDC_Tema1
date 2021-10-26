#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define FIFO_NAME "Comanda"
#define FIFO_2_NAME "Raspuns"

void fifo_write(int fd, char text[]);
void fifo_read(int fd, char text[]);

int main(int argc, char *argv[])
{

    char comanda[30], raspuns[100];
    int num, fd, fd2;
    //mknod(FIFO_2_NAME, S_IFIFO | 0666, 0);
    printf("Asteptare server ...\n");
    fd = open(FIFO_NAME, O_WRONLY);
    fd2 = open(FIFO_2_NAME, O_RDONLY);

    printf("Server conectat, introduceti o comanda ...\n\n");

    while (fgets(comanda, sizeof(comanda), stdin), !feof(stdin))
    {
        fifo_write(fd, comanda);
        if (strncmp(comanda, "quit", 3) == 0)
        {
            exit(3);
        }
        fifo_read(fd2, raspuns);
        // read(fd2, raspuns, 1024);
    }
}

void fifo_write(int fd, char text[])
{
    int num;
    if ((num = write(fd, text, strlen(text))) == -1)
    {
        perror("Eroare la scriere in FIFO\n");
        exit(0);
    }
    else
    {
        //printf("S-a scris in FIFO: %s\n", text);
    }
}

void fifo_read(int fd, char text[])
{
    int num;
    do
    {
        if ((num = read(fd, text, 10000)) == -1)
            perror("Eroare la citire din FIFO");
        else
        {
            text[num] = '\0';
            // printf("S-a citit din FIFO2 comanda:\n%s\n", text);
            printf("%s\n\n", text);
            break;
        }
    } while (num > 0);
}