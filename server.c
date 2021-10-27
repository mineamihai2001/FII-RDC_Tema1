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
#include <time.h>
#include <dirent.h>

#define FIFO_NAME "Comanda"
#define FIFO_2_NAME "Raspuns"

int is_logged = 0;
time_t logged_time;
char *time_str;

void este_comanda(int fd, int fd2, char comanda[], char username[]);
int login(char username[]);
int get_logged_users(int fd, char username[], char buff[]);
int get_proc_info(int ppid, char buff[], char username[]);
void logout(char username[]);
void quit();
void fifo_write(int fd, char text[]);
void fifo_read(int fd, int fd2, char text[], char username[]);
void get_stats(char buff[]);

int main(int argc, char *argv[])
{
    int num, fd, fd2;
    char comanda[30], username[20], raspuns[100];

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    printf("Asteptare client ...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    fd2 = open(FIFO_2_NAME, O_WRONLY);
    printf("Client conectat\n");

    fifo_read(fd, fd2, comanda, username);

    return 0;
}
void este_comanda(int fd, int fd2, char comanda[], char username[])
{
    int pid, pfd[2];
    char buff[1000];
    if (pipe(pfd) == -1)
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
            char raspuns[100];

            printf("S-a dat comanda login cu username:%s\n", username);
            printf("Login : %d\n", login(username));
            if (login(username) == 0)
            {
                printf("Username-ul nu exista: %s\n", username);
                if (write(pfd[1], "Username-ul nu exista", 1024) < 0)
                {
                    printf("Eroare la scriere in pipe!\n");
                    exit(1);
                }
                close(pfd[1]);
            }
            else
            {
                is_logged = 1;
                //get the login time
                logged_time = time(0);
                time_str = ctime(&logged_time);
                time_str[strlen(time_str) - 1] = '\0';

                printf("Logare cu succes cu username-ul: %s, %d\n", username, is_logged);

                if (write(pfd[1], "Connected", 1024) < 0)
                {
                    printf("Eroare la scriere in pipe!\n");
                    exit(1);
                }
                close(pfd[1]);
            }
        }
        else if (strncmp(comanda, "get-logged-users", 15) == 0) //get-logged-users
        {
            printf("S-a dat comanda %s, cu userame %s, %d\n", comanda, username, is_logged);

            get_logged_users(fd, username, buff);
            if (write(pfd[1], buff, 1024) < 0)
            {
                printf("Eroare la scriere in pipe\n");
                exit(2);
            }
            close(pfd[1]);
            printf("is_logged = %d\n", is_logged);
        }
        else if (strncmp(comanda, "get-proc-info : ", 15) == 0) //get-proc-info
        {
            int ppid = 0;
            for (int i = 16; comanda[i] >= '0' && comanda[i] <= '9'; i++) //construire pid
            {
                ppid = ppid * 10 + (comanda[i] - '0');
            }
            printf("S-a dat comanda %s cu pid: %d\n", comanda, ppid);
            get_proc_info(ppid, buff, username);

            if (write(pfd[1], buff, 10000000) < 0)
            {
                printf("Eroare la scriere in pipe");
                exit(2);
            }
            close(pfd[1]);
        }
        else if (strncmp(comanda, "logout", 5) == 0) //logout
        {
            printf("S-a dat comanda %s\n", comanda);
            logout(username);
            strcpy(buff, "Disconnected");
            strcat(buff, username);
            is_logged = 0;
            if (write(pfd[1], buff, 1024) < 0)
            {
                printf("Eroare la scriere in pipe");
                exit(2);
            }
            close(pfd[1]);
        }
        else if (strncmp(comanda, "quit", 4) == 0) //exit
        {
            printf("S-a dat comanda %s\n", comanda);
            if (write(pfd[1], comanda, 1024) < 0)
            {
                printf("Eroare la scriere in pipe");
                exit(2);
            }
            close(pfd[1]);
            quit();
        }
        else //comanda gresita
        {
            char eroare[] = "Eroare: comanda gresita! Introduceti o comanda corecta!";
            printf("Eroare: comanda gresita! Introduceti o comanda corecta!\n%s\n", comanda);
            if (write(pfd[1], eroare, 1024) < 0)
            {
                printf("Eroare la scriere in pipe");
                exit(2);
            }
            close(pfd[1]);
        }
    }
    else //parinte
    {
        is_logged = 1;

        //get the login time
        logged_time = time(0);
        time_str = ctime(&logged_time);
        time_str[strlen(time_str) - 1] = '\0';

        char buff[100000];
        printf("Am intrat in parinte %s, %d\n", username, is_logged);

        close(pfd[1]);
        if (read(pfd[0], buff, sizeof(buff)) < 0)
        {
            printf("Eroare la citire din pipe!\n");
            exit(1);
        }
        else
        {
            //scrire in FIFO2
            printf("S-a citit cu succes in pipe %s\n", buff);
            int num;
            if ((num = write(fd2, buff, strlen(buff))) == -1)
            {
                perror("Eroare la scriere in FIFO2\n");
                exit(0);
            }
            else
            {
                printf("S-a scris in FIFO2: %s\n", buff);
            }
        }
    }
}

int login(char username[])
{
    char name[20];
    FILE *users;
    users = fopen("users.txt", "r");
    while (fgets(name, sizeof(name), users) != NULL)
    {
        if (strstr(name, username) != NULL)
        {
            printf("user gasit : %s\n", name);
            //is_logged = 1;
            return 1;
        }
    }
    fclose(users);
    return 0;
}

int get_logged_users(int fd, char username[], char buff[])
{
    if (is_logged == 0 || login(username) == 0)
    {
        printf("Niciun user logat %d!\n", is_logged);
        strcpy(buff, "Niciun user logat!\n");
        return 0;
    }

    //logged time
    logged_time = time(0);
    time_str = ctime(&logged_time);
    time_str[strlen(time_str) - 1] = '\0';

    printf("Logged time : %s\n", time_str);
    char logged_time[100] = "Logged time : ";
    strcat(logged_time, time_str);
    logged_time[strlen(logged_time) - 1] = '\n';

    //hostname
    char hostname[_SC_HOST_NAME_MAX + 1];
    gethostname(hostname, _SC_HOST_NAME_MAX + 1);
    hostname[strlen(hostname) - 1] = '\n';

    //construire buff
    strcpy(buff, logged_time);
    strcat(buff, "Host name: ");
    strcat(buff, hostname);
    strcat(buff, "Username: ");
    strcat(buff, username);
    buff[strlen(buff)] = '\0';
    return 1;
}
int get_proc_info(int ppid, char buff[], char username[])
{
    if (is_logged == 0 || login(username) == 0)
    {
        printf("Niciun user logat %d!\n", is_logged);
        strcpy(buff, "Niciun user logat!\n");
        return 0;
    }

    int gasit = 0;
    char line[100];
    char word[10] = "";
    char path[30] = "/proc/";
    char str_ppid[10];
    sprintf(str_ppid, "%d", ppid);

    DIR *proc;
    struct dirent *entry1;
    int files1 = 0;
    proc = opendir(path);
    if (proc == NULL)
    {
        printf("Nu se poate citi din folder\n");
        return 0;
    }
    while ((entry1 = readdir(proc)))
    {
        ++files1;
        char file_name[20];
        strcpy(file_name, entry1->d_name);
        printf("Am gasit un fisier: %s\n", file_name);
        if (strcmp(file_name, str_ppid) == 0)
        {
            printf("Am gasit in /proc/ pid-ul %s\n", str_ppid);
            gasit = 1;
            break;
        }
    }

    if (gasit == 0)
    {
        strcpy(buff, "PID-ul nu exista\n");
        return 0;
    }

    strcat(path, str_ppid);
    printf("Folder path : %s\n", path);

    DIR *folder;
    struct dirent *entry;
    int files = 0;
    folder = opendir(path);
    if (folder == NULL)
    {
        printf("Nu se poate citi din folder\n");
        return 0;
    }
    while ((entry = readdir(folder)))
    {
        ++files;
        char file_name[20];
        strcpy(file_name, entry->d_name);
        printf("Am gasit un fisier: %s\n", file_name);
        if (strcmp(file_name, "stat") == 0)
        {
            printf("Am gasit in /proc/%s stat\n", str_ppid);
            break;
        }
    }
    closedir(folder);

    strcat(path, "/stat");

    FILE *fd_stat;
    fd_stat = fopen(path, "r");
    char c;
    int i = 0;
    if (fd_stat)
        while ((c = fgetc(fd_stat)) != EOF)
        {
            buff[i] = c;
            ++i;
            putchar(c);
        }
    else
    {
        printf("Eroare la descidere stats %s\n", path);
        return 0;
    }
    buff[strlen(buff) - 1] = '\0';

    get_stats(buff);
    printf("Staturile: %s\n", buff);
    fclose(fd_stat);
    return 1;
}
void logout(char username[])
{
    is_logged = 0;
    strcpy(username, "");
}
void quit()
{
    is_logged = 0;
    int quit = 0;
    int pid;
    int sockp[2];
    char msg[30] = "";
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        printf("Eroare socketpair\n");
        exit(0);
    }
    if ((pid = fork()) == -1)
    {
        printf("Eroare fork\n");
        exit(0);
    }
    else if (pid == 0)
    {
        close(sockp[0]);
        if (write(sockp[1], "Am iesit din porgram", sizeof("Am iesit din program")) < 0)
            perror("Eroare la scriere in socket\n");
        printf("[Copil] :  am transmis cu succes la parinte\n");
        close(sockp[1]);
    }
    else
    {
        close(sockp[1]);
        if (read(sockp[0], msg, 1024) < 0)
            perror("Eroare la citire din socket\n");
        printf("[Parinte]:  %s\n", msg);
        close(sockp[1]);
        exit(2);
    }
}

void get_stats(char buff[])
{
    int spaces = 0;
    char stats[100] = "";
    char new_buff[1000];
    int j = 0;
    char *p;
    strcpy(new_buff, buff);
    p = strtok(new_buff, " ");
    while (p)
    {
        if (spaces == 1)
        {
            strcpy(stats, "Name: ");
            strcat(stats, p);
            stats[strlen(stats)] = '\n';
        }
        else if (spaces == 2)
        {
            strcat(stats, "State: ");
            strcat(stats, p);
            stats[strlen(stats)] = '\n';
        }
        else if (spaces == 3)
        {
            strcat(stats, "PPID: ");
            strcat(stats, p);
            stats[strlen(stats)] = '\n';
        }
        else if (spaces == 22)
        {
            strcat(stats, "vmsize: ");
            strcat(stats, p);
            stats[strlen(stats)] = '\n';
        }
        spaces++;
        p = strtok(NULL, " ");
    }
    strcpy(buff, stats);
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

void fifo_read(int fd, int fd2, char text[], char username[])
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
            este_comanda(fd, fd2, text, username);
        }
    } while (num > 0);
}