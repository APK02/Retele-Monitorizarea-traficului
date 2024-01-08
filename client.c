#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#define SIZE 1000

const int timer_length = 1000;
time_t start_time = time(NULL);

extern int errno;

int port;

typedef struct user {
    int viteza = 0;
    int str_id = 0;
    int viteza_max = 0;
    bool vreme = 0;
    bool sporturi = 0;
    bool combustibil = 0;
    char nume_strada[100];
}user;

int sd;
struct sockaddr_in server;
user u;
void getUser() {
    if (read(sd, &u, sizeof(user)) < 0 ) {
        perror ("[Client]Eroare primul read() user.\n");
        return;
    }
}

void print_user() {
    printf("Viteza: %d\n", u.viteza);
    printf("str_id: %d\n", u.str_id);
    printf("Viteza maxima: %d\n", u.viteza_max);
    printf("Strada: %s\n", u.nume_strada);
    printf("Vreme: %d\n", u.vreme);
    printf("sporturi: %d\n", u.sporturi);
    printf("peco: %d\n", u.combustibil);
}

void TrimiteComanda(char comanda[], char raspuns[]) {
    //printf("%s", comanda);
    int nr_bytes = strlen(comanda);
    //printf("%d\n", nr_bytes);

    if (write (sd, &nr_bytes, sizeof(nr_bytes)) < 0) {
        perror ("[client]Eroare la write() spre server.\n");
        return;
    }
    
    if (write (sd, comanda, nr_bytes) < 0) {
        perror ("[client]Eroare la write() spre server.\n");
        return;
    }

    size_t marime_mesaj;
    if (-1 == read(sd, &marime_mesaj, sizeof(marime_mesaj))) {
        perror ("[client]Eroare la primul read().\n");
        return;
    }

    if (-1 == read(sd, raspuns, marime_mesaj)) {
        perror ("[client]Eroare la al doilea read().\n");
        return;
    }

    raspuns[marime_mesaj]='\0';

    if (strcmp(raspuns, "quit\0") == 0) {
        printf("[client]: Ati inchis conexiunea!\n");
        exit(1);
    }

    getUser();

    if(u.viteza > u.viteza_max && u.viteza_max != 0)
        strcat(raspuns, "\nATENTIE! Ati depasit limita legala de viteza!");
    
    //print_user();
}

void IntroducereDate () {
    char comanda[SIZE];
    char raspuns[SIZE];
    printf("Introduceti id-ul strazii pe care va aflati: ");
    fflush(stdout);
    char buffer[SIZE];
    bzero(buffer, SIZE);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return;
    }
    //printf("%s", buffer);
    strcpy(comanda, "info ");
    strcat(comanda, buffer);
    TrimiteComanda(comanda, raspuns);
    printf ("[client]: %s\n", raspuns);
    bzero(comanda, SIZE);
    bzero(raspuns, SIZE);
    printf("Introduceti viteza cu care va deplasati: ");
    fflush(stdout);
    bzero(buffer, SIZE);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return;
    }
    strcpy(comanda, "speed inceput ");
    strcat(comanda, buffer);
    TrimiteComanda(comanda, raspuns);
    printf ("[client]: %s\n", raspuns);
}

int main (int argc, char *argv[]) {
    //bool a = 0; //pentru anunt
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi (argv[2]);
    
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("[Client]Eroare la socket");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror("[Client]Erroare la connect().\n");
        return errno;
    }

    IntroducereDate();
    //a=1;
    char comanda[SIZE], raspuns[SIZE];
    //int pid;
    //if ((pid = fork()) < 0) {
    //    perror("[Client]Eroare la fork().\n");
    //    exit(1);
    //}

    //if (pid != 0) { //tatal
        //printf("tatal\n");
        while(1) {
            bzero(comanda, SIZE);
            bzero(raspuns, SIZE);
            struct timeval tv;
            tv.tv_sec = 10; 
            tv.tv_usec = 0;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(STDIN_FILENO, &read_fds);
            FD_SET(sd, &read_fds);

            if (select(sd + 1, &read_fds, nullptr, nullptr, &tv) < 0 ) {
                perror("[Client]Eroare la select()\n");
                exit(1);
            } 
            else if (FD_ISSET(sd, &read_fds) != 0 && FD_ISSET(STDIN_FILENO, &read_fds) == 0) {
                size_t n;
                char avertisment[SIZE];
                if (-1 == read(sd, &n, sizeof(size_t))) {
                    perror ("[client]Eroare la anunt read().\n");
                    exit(1);
                }

                if (-1 == read(sd, avertisment, n)) {
                    perror ("[client]Eroare la anunt read().\n");
                    exit(2);
                }

                avertisment[n]='\0';
                printf("%s\n",avertisment);
            } 
            else if (FD_ISSET(STDIN_FILENO, &read_fds) != 0 && FD_ISSET(sd, &read_fds) == 0){
                if (fgets(comanda, sizeof(comanda), stdin) == NULL) {
                    break;
                }
                TrimiteComanda(comanda, raspuns);
                printf ("[client]: %s\n", raspuns);
            }
        }
    //}

    /*else if (pid == 0) { //copil
        //printf("copil\n");
        while(1) {
            double secunde_trecute = difftime(time(NULL), start_time);

            if (secunde_trecute >= timer_length) {
                start_time = time(NULL);
                printf("%ld\n", start_time%1000);
                sprintf(comanda, "Auto speed : %d\n", u.viteza);
                TrimiteComanda(comanda, raspuns);
                printf ("[client]: %s\n", raspuns);
            }
        }
        exit(1);
        //}
    }*/
    close (sd);
    return 0;
}