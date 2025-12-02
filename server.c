/* server.c */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

// declaram functiile pe care le vom utiliza ulterior
FILE* open_file(const char* filename);
int create_socket();
struct addrinfo* resolve_address(const char* hostname, const char* port);
void connection_to_server(int socket_fd, struct addrinfo* res);
void send_request(int socket_fd, const char* request);
void receive_data(int socket_fd, FILE* file);
void close_socket(int socket_fd);

int main() {

    int socket_server, bind_socket, socket_client, receive_command; // variabile necesare
    struct sockaddr_in server_address; // structura pentru adresa serverului
    int port = 22216; // portul pe care serverul va asculta conexiunile
    char buffer[512]; // buffer pentru stocarea comenzilor primite de la client
    const char *message = "Comanda neimplementata."; // mesaj de raspuns daca comanda primita nu este comanda 14
    struct sockaddr_storage their_addr; // structura pentru adresa clientului
    socklen_t addr_size = sizeof(their_addr); // dimensiunea adresei clientului

    socket_server = socket(PF_INET, SOCK_STREAM, 0); // apelam functia socket pentru a crea socket-ul serverului
    if(socket_server == -1) { // verificam daca socket-ul a fost creat
        printf("Socket-ul nu a fost creat.\n"); // daca nu a fost creat afisam un mesaj sugestiv
        return -1; // iesim din functie
    } else {
        printf("Socket-ul a fost creat.\n"); // altfel afisam un mesaj de succes
    }

    // setam adresa serverului
    server_address.sin_family = AF_INET; // folosim IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // acceptam conexiuni de la orice adresa
    server_address.sin_port = htons(port); // seteaza portul serverului

    // asociem socket-ul la adresa si port
    bind_socket = bind(socket_server, (struct sockaddr *)&server_address, sizeof(server_address));
    if(bind_socket != 0) { // verificam daca legatura nu a fost creata
        printf("Legatura nu a fost creata.\n"); // afisam un mesaj sugestiv
        close(socket_server); // inchidem socket-ul
        return -1; // iesim din functie
    } else {
        printf("Socket-ul a fost legat cu succes la port si adresa.\n"); // altfel afisam un mesaj de succes
    }

    if(listen(socket_server, 5) != 0) { // functia listen pune serverul sa asculte, avand maxim 5 conexiuni in coada de asteptare
        printf("Ascultarea nu este posibila.\n"); // daca a intervenit o eroare afisam un mesaj sugestiv
        close(socket_server); // inchidem socket-ul
        return -1; // iesim din functie
    } else {
        printf("Socket-ul asculta pe portul %d.\n", port); // altfel afisam portul pe care serverul asculta
    }

    // bucla principala pentru gestionarea conexiunilor clientilor
    while(1) {
        socket_client = accept(socket_server, (struct sockaddr *)&their_addr, &addr_size); // pentru fiecare conexiune acceptata se creeaza un socket nou

        if(socket_client == -1) { // daca socket-ul nu a fost creat afisam un mesaj sugestiv si asteptam alti clienti
            printf("Conexiunea cu clientul a esuat.\n");
            continue;
        } else {
            printf("Un client nou s-a conectat cu succes.\n"); // altfel afisam un mesaj de succes
        }

        // bucla pentru primirea si procesarea comenzilor de la client
        while(1) {
            receive_command = recv(socket_client, buffer, sizeof(buffer), 0); // folosim functia recv pentru a primi datele(comanda) de la client

            if(receive_command == -1) { // verificam daca a aparut o eroare la primirea comenzii
                printf("Comanda nu a fost primita.\n"); // afisam mesaj
                close(socket_client); // inchidem socket-ul
                return -1; // iesim din program
            } else if (receive_command == 0) { // daca functia recv returneaza 0 atunci inseamna ca un client s-a deconectat de la server
                printf("Client deconectat de la server.\n"); // afisam mesaj de informare
                break; // iesim din bucla
            } else {
                printf("S-a primit o comanda.\n"); // altfel succes si verificam comanda

                if(strstr(buffer, "14")) { // verificam daca comanda este valida, adica daca contine 14
                    printf("Comanda primita este valida.\n");

                    int client_socket = create_socket(); // apeland functia create_socket vom crea un socket pentru a trimite o cerere HTTP
                    struct addrinfo *addr = resolve_address("::1", "80"); // functia resolve_address determina si returneaza informatiile necesare pentru a putea fi folosite de catre socket pentru a ne conecta la server
                    connection_to_server(client_socket, addr); // apeam functia pentru a ne conecta la server

                    const char *http_request = "GET / HTTP/1.0\r\n\r\n"; // definim cererea HTTP GET
                    send_request(client_socket, http_request); // trimitem cererea catre server

                    FILE *output_file = open_file("output.html"); // deschidem fisierul pentru a salva raspunsul
                    receive_data(client_socket, output_file); // dupa ce primim datele le salvam in fisier
                    fclose(output_file); // inchidem fisierul
                    close_socket(client_socket); // inchidem socket-ul

                    const char *success = "Operatie realizata cu succes!\n"; // definim un mesaj de succes pe care il vom trimite clientului
                    if(send(socket_client, success, strlen(success), 0) == -1) { // verificam daca mesajul a fost trimis
                        printf("Eroare!\n"); // in caz ca nu a fost trimis afisam mesajul de eroare
                    }

                } else {
                    printf("Comanda neimplementata.\n"); // daca comanda nu este valida afisam un mesaj de informare
                    if(send(socket_client, message, strlen(message), 0) == -1) { // trimitem mesajul care contine: Comanda neimplementata.
                        printf("Eroare\n"); // daca trimiterea a esuat afisam un mesaj de eroare
                    }
                }
            }
        }

        // inchidem conexiunea cu clientul
        if(close(socket_client) != 0) { // verificam daca socket-ul a fost inchis
            printf("Conexiunea cu clientul nu a fost inchisa.\n"); // daca nu a fost inchis afisam mesajul informativ
        } else {
            printf("Conexiunea cu clientul a fost inchisa.\n"); // altfel afisam un mesaj de succes
        }
    }

    return 0; // iesim din program cu succes
}
