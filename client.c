/* client.c */

#include <stdio.h> // biblioteca standard pentru intrare/iesire
#include <sys/types.h> // include definitii pentru tipurile de date folosite in programarea cu socket-uri
#include <sys/socket.h> // include functii si constante pentru lucrul cu socket-uri
#include <stdlib.h> // include functii pentru gestionarea memoriei si iesirea din program
#include <string.h> // include functii pentru lucrul cu siruri de caractere
#include <netdb.h> // include functii pentru manipularea adreselor de retea
#include <memory.h> // include functii pentru lucrul cu memorii
#include <unistd.h> // include functia close() pentru descriptorii de fisiere

// functie care deschide un fisier in modul scriere
FILE* open_file(const char* filename) {
    FILE *fptr = fopen(filename, "w"); // deschide fisierul in modul scriere
    if(fptr == NULL) { // verifica daca fisierul a putut fi deschis
        printf("Fisierul nu a fost deschis.\n"); // daca nu a fost deschis afiseaza un mesaj de eroare
        exit(-1); // si iese din program cu un cod de eroare
    }
    return fptr; // functia returneaza pointer-ul catre fisier
}

// functie care creeaza un socket
int create_socket() {
    int our_socket = socket(PF_INET6, SOCK_STREAM, 0); // creeaza un socket folosind IPv6 si TCP
    if (our_socket == -1) { // verifica daca socket-ul nu a fost creat
        printf("Socket-ul nu a fost creat.\n"); // daca nu a fost creat se afiseaza un mesaj de eroare
        exit(-1); // si iese din program cu un cod de eroare
    } else {
        printf("Socket-ul a fost creat cu succes.\n"); // daca socket-ul a fost creat se afiseaza un mesaj de succes
    }
    return our_socket; // functia returneaza descriptorul socket-ului
}

// functie care pregateste un hostname si un port intr-o structura de adresa
struct addrinfo* resolve_address(const char* hostname, const char* port) {
    struct addrinfo hints, *res; // definim structurile pentru cerere si rezultat
    memset(&hints, 0, sizeof hints); // initializam structura hints cu valori de zero
    hints.ai_family = PF_INET6; // utilizam IPv6
    //hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM; // utilizam TCP
    int status = getaddrinfo(hostname, port, &hints, &res); // apelul metodei getaddrinfo care pregateste detaliile necesare despre hostname si port pentru a fi folosite

    if(status != 0) { // verificam daca in apelul functiei anterioare a aparut vreo eroare
        printf("Error!\n"); // daca a intervenit o eroare afisam un mesaj de eroare
        exit(-1); // si iesim din program cu un cod de eroare
    }
    return res; // returnam informatia despre adresa
}

// functie utilizata pentru conectarea la un server folosind un socket
void connection_to_server(int socket_fd, struct addrinfo* res) {
    int connect1 = connect(socket_fd, res->ai_addr, res->ai_addrlen); // realizam conexiunea folosind socket-ul dat ca parametru
    if(connect1 == -1) { // verificam daca conexiunea a esuat
        printf("Conexiunea nu a fost realizata.\n"); // in caz afirmativ afisam un mesaj de eroare
        exit(-1); // si iesim din program cu un cod de eroare
    } else {
        printf("Conexiune realizata cu succes!\n"); // altfel afisam un mesaj de succes
    }
}

// functie utilizata pentru trimiterea unei cereri catre server
void send_request(int socket_fd, const char* request) {
    int send_request = send(socket_fd, request, strlen(request), 0); // trimitem cererea utilizand socket-ul dat ca parametru
    if(send_request != -1) { // verificam daca cererea a fost trimisa cu succes
        printf("Request-ul a fost realizat cu succes.\n"); // in caz afirmativ afisam un mesaj de succes
    } else {
        printf("A intervenit o eroare.\n"); // altfel afisam un mesaj de eroare
        exit(-1); // si iesim din program cu un cod de eroare
    }
}

// functie utilizata pentru primirea datelor de la server si scrierea acestora in fisier
void receive_data(int socket_fd, FILE* file) {
    char memorie[512]; // buffer pentru a stoca datele primite
    int recv_request; // variabila folosita pentru stocarea valorii returnate de functia recv

    while((recv_request = recv(socket_fd, memorie, sizeof(memorie), 0)) > 0) { // bucla care continua atata timp cat inca se mai primesc date
        for(int i = 0; i < recv_request; i++) { // bucla care scrie in fisier fiecare caracter
            fprintf(file, "%c", memorie[i]);
        }
    }

    if(recv_request == -1) { // verificam daca functia recv a esuat
        printf("A intervenit o eroare. Nu s-au primit datele!\n"); // in caz de eroare se afiseaza un mesaj sugestiv
        exit(-1); // si iesim din program cu un cod de eroare
    } else if(recv_request == 0) { // verificam daca conexiunea a fost inchisa de server
        printf("Conexiunea a fost inchisa de catre server!\n"); // afisam un mesaj sugestiv
    }
}

// functie folosita pentru inchiderea socket-ului
void close_socket(int socket_fd) {
    int close_request = close(socket_fd); // apelam functia close
    if(close_request == 0) { // verificam daca inchiderea a fost efectuata cu succes
        printf("Socket-ul a fost inchis cu succes!\n"); // daca socket-ul a fost inchis afisam un mesaj de succes
    } else {
        printf("A intervenit o eroare, socket-ul nu a fost inchis.\n"); // altfel afisam un mesaj de informare
    }
}
