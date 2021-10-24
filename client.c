#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define FIFO_NAME "Comanda"

void fifo_write(int fd, char text[]);
void fifo_read(int fd, char text[], char username[]);

int main(int argc, char *argv[])
{

    char comanda[20];
    int num, fd;

    printf("Asteptare server ...\n");
    fd = open(FIFO_NAME, O_WRONLY);

    printf("Server conectat, introduceti o comanda ...\n");

    while (fgets(comanda, sizeof(comanda), stdin), !feof(stdin))
    {
        fifo_write(fd, comanda);
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
        printf("S-a scris in FIFO: %s\n", text);
    }
}

void fifo_read(int fd, char text[], char username[])
{
    int num;
    do
    {
        if ((num = read(fd, text, 300)) == -1)
            perror("Eroare la citire din FIFO");
        else
        {
            text[num] = '\0';
            printf("S-a citit din FIFO comanda %s\n", text);
            //este_comanda(text, username);
        }
    } while (num > 0);
}