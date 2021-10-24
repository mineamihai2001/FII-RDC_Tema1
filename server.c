#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <ctype.h>

#define FIFO_NAME "Comanda"

int is_logged;

int este_comanda(char comanda[], char username[]);
int login(char username[]);
int get_logged_users();
int get_proc_info();
int logout();
int exit1();
void fifo_write(int fd, char text[]);
void fifo_read(int fd, char text[], char username[]);

int main(int argc, char *argv[])
{
    int num, fd;
    char comanda[20], username[20];

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    printf("Asteptare client ...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    printf("Client conectat\n");

    fifo_read(fd, comanda, username);

    
}
int este_comanda(char comanda[], char username[])
{
    int pid, pfd[2];
    if(pipe(pfd) == -1)
    {
        printf("Eroare la deschide pipe!\n");
        exit(0);
    }
    pid = fork();
    if (pid == -1)
    {
        printf("Eroare la fork\n");
        exit(1);
    }
    if (pid == 0)
    {
        //implementare comenzi
        close(pfd[0]);

        printf("Am intrat in copil\n");
        if (strncmp(comanda, "login : ", 7) == 0) //login
        {
            int j = 0;
            for (int i = 8; i <= strlen(comanda) - 1; i++) //construire username
            {
                username[j] = comanda[i];
                ++j;
            }

            username[strlen(username) - 1] = '\0';
            printf("S-a dat comanda login cu username:%s\n", username);
            if (login(username) == 0)
            {
                printf("Username-ul nu exista: %s\n", username);
            }
            else
            {
                printf("Logare cu succes cu username-ul: %s\n", username);
                if(write(pfd[1], username, 1024) < 0)
                {
                    printf("Eroare la scriere in pipe!\n");
                    exit(1);
                }
                close(pfd[1]);
            }
        }
        else if (strncmp(comanda, "get-logged-users", 15) == 0) //get-logged-users
        {
            printf("S-a dat comanda %s", comanda);
            get_logged_users();
        }
        else if (strncmp(comanda, "get-proc-info", 12) == 0) //get-proc-info
        {
            printf("S-a dat comanda %s\n", comanda);
            get_proc_info();
        }
        else if (strncmp(comanda, "logout", 5) == 0) //logout
        {
            printf("S-a dat comanda %s\n", comanda);
            logout();
        }
        else if (strncmp(comanda, "exit", 3) == 0) //exit
        {
            printf("S-a dat comanda %s\n", comanda);
            exit1();
        }
        else //comanda gresita
        {
            printf("Eroare: comanda gresita! Introduceti o comanda corecta!\n%s\n", comanda);
        }
    }
    else{
        char buff[20];
        printf("Am intrat in parinte\n");
        // int status;
        // waitpid(0, &status, 0);
        close(pfd[1]);
        if(read(pfd[0], buff, sizeof(buff)) < 0)
        {
            printf("Eroare la citire din pipe!\n");
            exit(1);
        }
        else printf("S-a citit cu succes in pipe %s\n", buff);
    }
}

int login(char username[])
{
    char name[20];
    FILE *users;
    users = fopen("users.txt", "r");
    while (fgets(name, sizeof(name), users) != NULL)
    {
        if (strstr(name, username) == 0)
        {
            is_logged = 1;
            return 1;
        }
    }
    fclose(users);
    return 0;
}

int get_logged_users()
{
    if(is_logged == 0)
    {
        printf("Niciun user logat!\n");
        return 0;
    }

}
int get_proc_info()
{

}
int logout()
{

}
int exit1()
{

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
            este_comanda(text, username);
        }
    } while (num > 0);
}