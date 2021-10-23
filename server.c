#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_NAME "Comanda"

int este_comanda(char comanda[], char username[]);

int main(int argc, char *argv[])
{
    int num, fd;
    char comanda[20], username[20];

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    printf("Asteptare client ...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    printf("Client conectat\n");

    do
    {
        if ((num = read(fd, comanda, 300)) == -1)
            perror("Eroare la citire din pipe");
        else
        {
            //comanda[num] = '\0';
            printf("S-a citit din pipe comanda %s\n", comanda);
            este_comanda(comanda, username);
        }
    } while (num > 0);
}

int este_comanda(char comanda[], char username[])
{
    if (strncmp(comanda, "login : ", 7) == 0)
    {
        int j = 0;
        for (int i = 8; i <= strlen(comanda) - 1; i++)
        {
            username[j] = comanda[i];
            ++j;
        }

        printf("S-a dat comanda login cu username: %s\n", username);
        return 1;
    }
    else if (strncmp(comanda, "get-logged-users", 15) == 0)
    {

        printf("S-a dat comanda %s", comanda);
        return 1;
    }
    else if (strncmp(comanda, "get-proc-info", 12) == 0)
    {
        printf("S-a dat comanda %s\n", comanda);
        return 1;
    }
    else if (strncmp(comanda, "logout", 5) == 0)
    {
        printf("S-a dat comanda %s\n", comanda);
        return 1;
    }
    else if (strncmp(comanda, "exit", 3) == 0)
    {
        printf("S-a dat comanda %s\n", comanda);
        return 1;
    }
    else
    {
        printf("Eroare: comanda gresita! Introduceti o comanda corecta!\n%s\n", comanda);
        return 0;
    }
}