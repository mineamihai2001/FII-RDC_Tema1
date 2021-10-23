#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define FIFO_NAME "Comanda"

int main(int argc, char *argv[])
{

    char comanda[20];
    int num, fd;

    printf("Asteptare server ...\n");
    fd = open(FIFO_NAME, O_WRONLY);
    printf("Server conectat, introduceti o comanda ...\n");

    while (fgets(comanda, sizeof(comanda), stdin), !feof(stdin))
    {
        if ((num = write(fd, comanda, strlen(comanda))) == -1)
        {
            perror("Eroare la scriere in pipe\n");
            exit(0);
        }
        else
        {
            printf("S-a scris comanda %s\n", comanda);
        }
    }
}

