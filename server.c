#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#define SIZE 1000

#define PORT 2077

extern int errno;

typedef struct thData {
    int idThread;
    int cl;
}thData;

int randomDay() {
    srand(time(0));
    int day = rand() % 7 + 1;
    return day;
}

static void *treat(void *);
void raspunde(void *);

typedef struct user {
    int viteza = 0;
    int str_id = 0;
    int viteza_max = 0;
    bool vreme = 0;
    bool sporturi = 0;
    bool combustibil = 0;
    char nume_strada[100];
    bool anunt = 0;
}user;

void sendUser(int sd, user *u) {
    if (write(sd, u, sizeof(user)) < 0) {
        perror("[server]Eroare la write() user catre client.\n");
		return;
    } 
}

void vitezaAutomata(user &u, char comanda[], char raspuns[]) {
    char *speed = comanda + strlen("Auto speed : ");
    u.viteza = atoi(speed);
    sprintf(raspuns, "Viteza a fost trimisa automat : %dkm/h", u.viteza);
}

sqlite3* db;
sqlite3_stmt* statement;
char *zErrMsg = 0;

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    for(int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i]);
    }
    printf("\n");
    return 0;
}

void updateEvenimente(user &u, int n) {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char sqlstatement[SIZE];

    switch(n) {
        case 1:
        {
            sqlite3_prepare_v2(db, "SELECT viteza_max FROM trafic WHERE str_id = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, u.str_id);
            while(sqlite3_step(statement) != SQLITE_DONE) {
                u.viteza_max = sqlite3_column_int(statement, 0) - 20;
            }
            sprintf(sqlstatement, "UPDATE trafic SET evenimente = 'Accident' WHERE str_id = %d", u.str_id);
            sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
            sqlite3_close(db);
            break;
        }
        case 2:
        {
            sqlite3_prepare_v2(db, "SELECT viteza_max FROM trafic WHERE str_id = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, u.str_id);
            while(sqlite3_step(statement) != SQLITE_DONE) {
                u.viteza_max = sqlite3_column_int(statement, 0) - 20;
            }
            sprintf(sqlstatement, "UPDATE trafic SET evenimente = 'Drum in lucru' WHERE str_id = %d", u.str_id);
            sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
            sqlite3_close(db);
            break;
        }
        case 3:
        {
            sprintf(sqlstatement, "UPDATE trafic SET evenimente = 'Politie' WHERE str_id = %d", u.str_id);
            sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
            sqlite3_close(db);
            break;
        }
    }
}

void deleteEvenimente() {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char sqlstatement[SIZE];
    int i;
    for (i=1; i<=7; i++) {
        sprintf(sqlstatement, "UPDATE trafic SET evenimente = NULL WHERE str_id = %d", i);
        sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    }
    sqlite3_close(db);
}

void deleteAnunt(user &u) {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char sqlstatement[SIZE];
    
    sqlite3_prepare_v2(db, "SELECT viteza_max FROM trafic WHERE str_id = ?", -1, &statement, NULL);
    sqlite3_bind_int(statement, 1, u.str_id);
    while(sqlite3_step(statement) != SQLITE_DONE) {
        u.viteza_max = sqlite3_column_int(statement, 0);
    }

    sprintf(sqlstatement, "UPDATE trafic SET evenimente = NULL WHERE str_id = %d", u.str_id);
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    sqlite3_close(db);
}

void infoStrada(user &u, char raspuns[]) {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char eveniment[100];
    sqlite3_prepare_v2(db, "SELECT nume_str, viteza_max, evenimente FROM trafic WHERE str_id = ?", -1, &statement, NULL);
    sqlite3_bind_int(statement, 1, u.str_id);
    while(sqlite3_step(statement) != SQLITE_DONE) {
        strcpy(u.nume_strada, (char *)sqlite3_column_text(statement, 0));
        u.viteza_max = sqlite3_column_int(statement, 1);
        sqlite3_snprintf(100, eveniment, "%s", sqlite3_column_text(statement, 2));
    }
    
    if (strcmp(eveniment, "Politie") == 0) {
        u.anunt = 1;
        sprintf(raspuns, "Va aflati pe Strada %s. Atentie %s! Viteza maxima este %dkm/h!", u.nume_strada, eveniment, u.viteza_max);
    }
    else if (strcmp(eveniment, "\0")) {
        u.anunt = 1;
        u.viteza_max = u.viteza_max - 20;
        sprintf(raspuns, "Va aflati pe Strada %s. Atentie %s! Viteza maxima este acum %dkm/h!", u.nume_strada, eveniment, u.viteza_max);
    }
    else {
    sprintf(raspuns, "Va aflati pe Strada %s, viteza maxima este de %d Km/h!", u.nume_strada, u.viteza_max);
    }
}

int ziua = randomDay();

void infoVreme(char raspuns[], int ziua){
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char zi[SIZE];
    char status[SIZE];

    sqlite3_prepare_v2(db, "SELECT * FROM vreme WHERE id_vreme = ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, ziua);
    while(sqlite3_step(statement) != SQLITE_DONE) {
        strcpy(zi, (char *)sqlite3_column_text(statement, 1));
        int temperatura = sqlite3_column_int(statement, 2);
        int temp_max = sqlite3_column_int(statement, 3);
        int temp_min = sqlite3_column_int(statement, 4);
        int temp_resimtita = sqlite3_column_int(statement, 5);
        strcpy(status, (char *)sqlite3_column_text(statement, 6));
        sprintf(raspuns, "Azi este: %s\nTemperatura este: %d°\t Tmax: %d°\tTmin: %d°\tSe simte ca: %d°\nStatus: %s", zi, temperatura, temp_max, temp_min, temp_resimtita, status);
    }
}

void infoSporturi(char raspuns[]) {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char string[SIZE];
    int n=rand()%7+1;
    sqlite3_prepare_v2(db, "SELECT * FROM sporturi WHERE id_eveniment = ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, n);
    while(sqlite3_step(statement) != SQLITE_DONE) {
        strcpy(string, (char *)sqlite3_column_text(statement, 1));
        strcat(raspuns, "Eveniment -> ");
        strcat(raspuns, string);
        strcpy(string, (char *)sqlite3_column_text(statement, 2));
        strcat(raspuns, " Ora: ");
        strcat(raspuns, string);
        strcpy(string, (char *)sqlite3_column_text(statement, 3));
        strcat(raspuns, " Data: ");
        strcat(raspuns, string);
    }
}

void infoCombustibil(char raspuns[], user &u) {
    int rc = sqlite3_open("pj_database.db", &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "[server]Eroare la deschidere baza de date %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    int n = u.str_id;
    char companie[100];
    char strada[100];
    char p_benzina[10];
    char p_motorina[10];
    sqlite3_prepare_v2(db, "SELECT * FROM peco WHERE id_peco = ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, n);
    while (sqlite3_step(statement) != SQLITE_DONE) {
        strcpy(companie, (char *)sqlite3_column_text(statement, 1));
        strcpy(strada, (char *)sqlite3_column_text(statement, 2));
        double benzina = sqlite3_column_double(statement, 3);
        sprintf(p_benzina, "%f", benzina);
        p_benzina[4]='\0';
        double motorina = sqlite3_column_double(statement, 4);
        sprintf(p_motorina, "%f", motorina);
        p_motorina[4]='\0';
        snprintf(raspuns, SIZE, "%s se afla pe Strada %s\npretul benzinei: %s\tpretul motorinei: %s.", companie, strada, p_benzina, p_motorina);
        printf("%s\n", raspuns);
    }
}

void toggleFalse(char raspuns[]) {
    strcpy(raspuns, "Nu ati bifat aceasta optiune!");
}

fd_set writefds;
int nfds = 0;

void trimiteAnunt(int nfds, fd_set &writefds, int client, char msg[]) {
    int fd;
    size_t n;

    if (select (nfds+1, NULL, &writefds, NULL, NULL) < 0)
	{
	  perror ("[server] Eroare la select().\n");
	  return;
	}

    for (fd = 0; fd <= nfds; fd++) {
        if(fd != client && FD_ISSET(fd, &writefds)) {
           n = strlen(msg);
           if (-1 == write(fd, &n, sizeof(size_t))) {
                perror("Eroare la anunt-write() catre client.\n");
           }

           if (-1 == write(fd, msg, n)) {
                perror("Eroare la 2anunt-write() catre client.\n");
           }

           else {
            printf ("[Thread ]Mesajul a fost trasmis cu succes.\n");
           }
        }
    }
}

int main() {
    //stergem evenimentele
    deleteEvenimente();
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;
    pthread_t th[100];
    int i=0;

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    int on=1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1){
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen (sd, 2) == -1){
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    FD_ZERO(&writefds);
    
    while (1){
        int client;
        thData * td;
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush (stdout);

        if ((client = accept (sd, (struct sockaddr *) &from, &length)) < 0){
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        td=(struct thData*)malloc(sizeof(struct thData));
        td->idThread=i++;
        td->cl=client;

        if (nfds < client) {
            nfds = client;
        }

        pthread_create(&th[i], NULL, &treat, td);
    }
};

static void *treat(void * arg){
    struct thData tdL;
    tdL = *((struct thData*)arg);
    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData*)arg);
    close ((intptr_t)arg);
    return(NULL);
};

void raspunde(void * arg){
    char raspuns[SIZE];
    char buffer[SIZE];
    char sql[SIZE];
    char sql_info[SIZE];
    user u;
    int nr_bytes;
    struct thData tdL;
    tdL = *((struct thData*)arg);
    bool flag = 0;

    while (1) {
        bzero(raspuns, SIZE);
        bzero(buffer, SIZE);

        if (read (tdL.cl, &nr_bytes, sizeof(nr_bytes)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la 1read() de la client.\n");
        }

        if (read (tdL.cl, buffer, nr_bytes) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la 2read() de la client.\n");
        }

        printf("[Thread %d]Mesajul a fost receptionat...%s", tdL.idThread, buffer);

        char *comanda = buffer;

        //anunt
        if ((comanda = strstr(buffer, "anunt")) != NULL && comanda-buffer == 0 && *(comanda+5) == '\n') {
            if (1<=u.str_id && u.str_id<=7) {
                strcpy(raspuns, "Selectati numarul tipului de anunt:\n1.Accident\n2.Drum in lucru\n3.Politie\n4.Scoate anunt");
                flag = 1;
            }
            else {
                strcpy(raspuns, "Nu puteti face un anunt deoarece nu va stim locatia exacta!");
            }
        }

        else if ((comanda = strstr(buffer, "1")) != NULL && comanda-buffer == 0 && *(comanda+1) == '\n' && flag == 1) {
            updateEvenimente(u, 1);
            char msg[SIZE];
            flag = 0;
            u.anunt = 1;
            sprintf(msg, "Accident pe Strada %s, viteza maxima este acum %dkm/h!", u.nume_strada, u.viteza_max);
            trimiteAnunt(nfds, writefds, tdL.cl, msg);
            strcpy(raspuns, "Am trimis mesajul celorlalti.");
        }

        else if ((comanda = strstr(buffer, "2")) != NULL && comanda-buffer == 0 && *(comanda+1) == '\n' && flag == 1) {
            updateEvenimente(u, 2);
            char msg[SIZE];
            flag = 0;
            u.anunt = 1;
            sprintf(msg, "Drum in lucru pe Strada %s, viteza maxima este acum %dkm/h!", u.nume_strada, u.viteza_max);
            trimiteAnunt(nfds, writefds, tdL.cl, msg);
            strcpy(raspuns, "Am trimis mesajul celorlalti.");
        }

        else if ((comanda = strstr(buffer, "3")) != NULL && comanda-buffer == 0 && *(comanda+1) == '\n' && flag == 1) {
            updateEvenimente(u, 3);
            char msg[SIZE];
            flag = 0;
            u.anunt = 1;
            sprintf(msg, "Atentie! Politie pe Strada %s!", u.nume_strada);
            trimiteAnunt(nfds, writefds, tdL.cl, msg);
            strcpy(raspuns, "Am trimis mesajul celorlalti.");
        }

        else if ((comanda = strstr(buffer, "4")) != NULL && comanda-buffer == 0 && *(comanda+1) == '\n' && flag == 1) {
            flag = 0;
            if (u.anunt == 1) {
                deleteAnunt(u);
                sprintf(raspuns, "Am scos anuntul de pe Strada %s. Viteza maxima a revenit la %dkm/h.", u.nume_strada, u.viteza_max);
                trimiteAnunt(nfds, writefds, tdL.cl, raspuns);
                u.anunt = 0;
            }
            else {
                sprintf(raspuns, "Nu exista anunt pe Strada %s.", u.nume_strada);
            }
        }

        else if ((comanda = strstr(buffer, "event")) != NULL && comanda-buffer == 0 && *(comanda+5) == '\n') {
            u.anunt = 1;
        }

        else if ((comanda = strstr(buffer, "event2")) != NULL && comanda-buffer == 0 && *(comanda+6) == '\n') {
            u.anunt = 0;
        }

        //toggle
        else if ((comanda = strstr(buffer, "toggle vreme")) != NULL && comanda-buffer == 0 && *(comanda+12) == '\n') {
            u.vreme = !u.vreme;
            if (u.vreme == 0) {
                strcpy(raspuns, "Ati debifat optiunea vreme.");
            }
            else {
            strcpy(raspuns, "Ati bifat optiunea vreme.");
            }
        }

        else if ((comanda = strstr(buffer, "toggle sporturi")) != NULL && comanda-buffer == 0 && *(comanda+15) == '\n') {
            u.sporturi = !u.sporturi;
            if (u.sporturi == 0) {
                strcpy(raspuns, "Ati debifat optiunea evenimente sportive.");
            }
            else {
            strcpy(raspuns, "Ati bifat optiunea evenimente sportive.");
            }
        }

        else if ((comanda = strstr(buffer, "toggle peco")) != NULL && comanda-buffer == 0 && *(comanda+11) == '\n') {
            u.combustibil = !u.combustibil;
            if (u.combustibil == 0) {
                strcpy(raspuns, "Ati debifat optiunea preturi la combustibil.");
            }
            else {
            strcpy(raspuns, "Ati bifat optiunea preturi la combustibil.");
            }
        }

        else if ((comanda = strstr(buffer, "check all")) != NULL && comanda-buffer == 0 && *(comanda+9) == '\n') {
            u.vreme = 1;
            u.sporturi = 1;
            u.combustibil = 1;
            strcpy(raspuns, "Ati bifat toate optiunile.");
        }

        else if ((comanda = strstr(buffer, "uncheck all")) != NULL && comanda-buffer == 0 && *(comanda+11) == '\n') {
            u.vreme = 0;
            u.sporturi = 0;
            u.combustibil = 0;
            strcpy(raspuns, "Ati debifat toate optiunile.");
        }
        //gata-toggle

        //show
        else if ((comanda = strstr(buffer, "show vreme")) != NULL && comanda-buffer == 0 && *(comanda+10) == '\n') {
            if(u.vreme)
                infoVreme(raspuns, ziua);
            else
                toggleFalse(raspuns);
        }

        else if ((comanda = strstr(buffer, "show sporturi")) != NULL && comanda-buffer == 0 && *(comanda+13) == '\n') {
            if(u.sporturi)
                infoSporturi(raspuns);
            else
                toggleFalse(raspuns);
        }
        
        else if ((comanda = strstr(buffer, "show peco")) != NULL && comanda-buffer == 0 && *(comanda+9) == '\n') {
            if(u.combustibil)
                infoCombustibil(raspuns, u);
            else
                toggleFalse(raspuns);
        }
        //end-show


        //quit
        else if ((comanda = strstr(buffer, "quit")) != NULL && comanda-buffer == 0 && *(comanda+4) == '\n') {
            FD_CLR(tdL.cl, &writefds);
            strcpy(raspuns, "quit");
            //printf("%s\n", raspuns);
            size_t marime_mesaj = strlen(raspuns);
            //printf("%ld\n", marime_mesaj);
            write(tdL.cl, &marime_mesaj, sizeof(size_t));
            write(tdL.cl, raspuns, strlen(raspuns));
            break;
        }
        //gata-quit

        else if ((comanda = strstr(buffer, "speed inceput ")) != NULL && comanda-buffer == 0) {
            char *speed = comanda + strlen("speed inceput ");
            int v =  atoi(speed);
            if (0<=v && v<=500) {
                u.viteza = v;
                sprintf(raspuns, "Viteza voastra este: %dkm/h", u.viteza);
            }
            else {
                strcpy(raspuns, "Introduceti o viteza valida! (intre 0 si 500)");
            }
            FD_SET(tdL.cl, &writefds); //dupa datele de inceput userul va putea primi anunturi
        }

        //userSpeed
        else if ((comanda = strstr(buffer, "speed ")) != NULL && comanda-buffer == 0) {
            char *speed = comanda + strlen("speed ");
            int v =  atoi(speed);
            if (0<=v && v<=500) {
                u.viteza = v;
                sprintf(raspuns, "Viteza voastra este: %dkm/h", u.viteza);
            }
            else {
                strcpy(raspuns, "Introduceti o viteza valida! (intre 0 si 500)");
            }
        }
        //gata-userSpeed

        //viteza-automata
        else if ((comanda = strstr(buffer, "Auto speed : ")) != NULL && comanda-buffer == 0) {
            vitezaAutomata(u, comanda, raspuns);
        }
        //gata viteza-automata

        //info
        else if ((comanda = strstr(buffer, "info ")) != NULL && comanda-buffer == 0) {
            char *id = comanda + 5;
            int nr = atoi(id);
            if (1<=nr && nr<=7) {
                u.str_id = nr;
                infoStrada(u, raspuns);
            }
            else {
                strcpy(raspuns, "Introduceti un id cuprins intre 1 si 7!");
            }
        }
        //gata-info

        else {
            strcpy(raspuns, "Introduceti o comanda valida!");
        }

        size_t marime_mesaj = strlen(raspuns);
        if (-1 == write(tdL.cl, &marime_mesaj, sizeof(size_t))) {\
            printf("[Thread %d] ", tdL.idThread);
            perror("Eroare la write() catre client.\n");
        }
        if (-1 == write(tdL.cl, raspuns, marime_mesaj)) {
            printf("[Thread %d] ", tdL.idThread);
            perror("Eroare la write() catre client.\n");
        }
        else {
            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }
        sendUser(tdL.cl, &u);
    }
}