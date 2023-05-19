/********************************************************
File name : serveur.c
Author : Ningxin YE, Yiqing CHEN, Yang YANG
Class : L3 MIAGE(université paris Dauphine_PSL)
Day : 2023/5/15
Compile command line : gcc serveur.c -o serveur
The command line for running the program ：
For stop the programme : ctrl +z（）or ctrl + c
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h> 
#include <arpa/inet.h>
#include <signal.h> 
#define NUM_MAX 5

int creat_socket();
int accept_client(int sev_socket);
void *se_connecter_client(void *arg);
void send_message_to_other(long id, char *serveur_reponse, char *sperotor);
void first_connect(long id, char *client_reponse, char *serveur_reponse);
void delet_client_information(long id);
int give_time(char *buf);
void sigintHandler(int sig_num);
void write_message_in_doc();

pthread_t threads[NUM_MAX];
pthread_mutex_t waitMemset;
#define MAX_MESSAGES 1000
#define MAX_MESSAGE_LENGTH 100


typedef struct
{
    char name[100];
    int client_socket;
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    int message_count;
     // Etape 2 : for the gestion of message for a client
} CLIENT;

CLIENT clients[NUM_MAX] = {0};

char ask_name[30] = "what's your name?";

int main(int argc, char const *argv[])
{
    // 添加信号处理
    signal(SIGINT, sigintHandler);
    /*Create socket*/
    int sev_socket = creat_socket();
    int client_socket;
    pthread_mutex_init(&waitMemset,NULL);
    /*Accepter les connexions des clients*/
    while (1)
    {
        client_socket = accept_client(sev_socket);
        printf("***********************************\n");
    }
    // printf("all the message");
    // close the socket
    for (int i = 0; i < NUM_MAX; i++)
    {
        if (clients[i].client_socket != 0)
            close(clients[i].client_socket);
    }
    pthread_mutex_destroy(&waitMemset);
    close(sev_socket);
    unlink("MySock");
    return 0;
}

int creat_socket()
{
    unlink("Mysock");
    // Initialisation struct addr serveur
    struct sockaddr_un addr ={0}; // Address Family UNIX
    memset(&addr, 0, sizeof(struct sockaddr_un)); // Chemin vers fichier socket local
    addr.sun_family = AF_UNIX;	
    strcpy(addr.sun_path, "./MySock");	

    // Creation socket serveur
    int sev_socket = -1;
    sev_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sev_socket < 0)
    {
        perror("error when we creat socket!!!\n");
        exit(1);
    }

    
    // Connection de la socket a l'addresse serveur
    int test_bind = bind(sev_socket, (struct sockaddr *)&addr, sizeof(addr));

    if (test_bind < 0)
    {
        perror("error when bind!\n");
        exit(-1);
    }
    // Socket en attente de connexion (a l'ecoute),
    int test_listen = listen(sev_socket, NUM_MAX);
    if (test_listen < 0)
    {
        perror("error whe listen");
        exit(-1);
    }
    printf("The server has been successfully created!\n");
    return sev_socket;
}

int accept_client(int sev_socket)
{
    // Initialisation struct addr client
    struct sockaddr_un client_addr = {0};
    socklen_t addrlen = sizeof(client_addr);
    printf("Waiting for client connection...\n");
    int client_socket = accept(sev_socket, (struct sockaddr *)&client_addr, &addrlen);
    if (client_socket < 0)
    {
        perror("Error when accepting client connection");
        exit(-1);
    }

    // Find an available slot in the clients array
    int client_index = -1;
    for (int i = 0; i < NUM_MAX; i++)
    {
        if (clients[i].client_socket == 0)
        {
            client_index = i;
            break;
        }
    }

    if (client_index != -1)
    {
        clients[client_index].client_socket = client_socket;
        pthread_create(&threads[client_index], NULL, se_connecter_client, (void *)(intptr_t)client_index);
        pthread_detach(threads[client_index]);
        //printf("Connected with client at address: %s\n", inet_ntoa(client_addr.sin_addr));
    }
    else
    {
        printf("Maximum number of clients reached. Connection refused.\n");
        close(client_socket);
    }

    return client_socket;
}

void *se_connecter_client(void *arg)
{
    long id = (long)arg;
    int read_size = 0;
    char sperotor[30] = "----------------\n";
    while (clients[id].client_socket!=0)
    {
        char client_reponse[100] = {0};
        char serveur_reponse[100] = {0};
            // if the client hasn't a name(the fist time of connexion)
            if (strlen(clients[id].name) == 0)
            {
                first_connect(id, client_reponse, serveur_reponse);
                memset(serveur_reponse, 0, sizeof(serveur_reponse));
                memset(client_reponse,0,sizeof(client_reponse));
                continue;
            }
            read_size = recv(clients[id].client_socket, client_reponse, 100, 0);
            client_reponse[read_size] = '\0';
            // this client is out of connexion or erro when read o
            if (read_size <= 0||strncmp(client_reponse, "quit", 4) == 0)
            {
                printf("%s is out of connexion\n", clients[id].name);
                sprintf(serveur_reponse,"%s is out of connexion\n",clients[id].name);
                send_message_to_other(id,serveur_reponse,sperotor);
                delet_client_information(id);
                pthread_exit(NULL);
                break;
            }
            // the read_size is not 0
            else
            {
                char messageTime[100] = {0};
                give_time(messageTime);
                printf("receive of the message of client %s : %s \n ", clients[id].name, client_reponse);
                sprintf(serveur_reponse, "%s %s : %s", messageTime, clients[id].name, client_reponse);
                send_message_to_other(id,serveur_reponse,sperotor);
                
                //将传送的message存入client的数组中
                if (clients[id].message_count < MAX_MESSAGES) {
                strncpy(clients[id].messages[clients[id].message_count], client_reponse, MAX_MESSAGE_LENGTH);
                clients[id].message_count++;
            }
            }
        }
    
    pthread_exit(NULL);
}

/*用于将消息发送给其他clients*/
void send_message_to_other(long id, char *serveur_reponse, char *sperotor)
{
    pthread_mutex_lock(&waitMemset);
    serveur_reponse[strlen(serveur_reponse)]='\0';
    for (int i = 0; i < NUM_MAX; i++)
    {
        if (i != id && clients[i].client_socket != 0)
        {
             char message_to_send[1000];  // 创建一个新的缓冲区
            strcpy(message_to_send, serveur_reponse);  // 复制要发送的消息到新的缓冲区

            if(sperotor!=NULL){ send(clients[i].client_socket, sperotor, strlen(sperotor), 0);}
            send(clients[i].client_socket, message_to_send, strlen(message_to_send), 0);
        }
    }
    memset(serveur_reponse,0,strlen(serveur_reponse));
    pthread_mutex_unlock(&waitMemset);
    
}

/*用于第一次连接的时候分配一个名字*/
void first_connect(long id, char *client_reponse, char *serveur_reponse)
{
    send(clients[id].client_socket, ask_name, strlen(ask_name), 0);
    int read_size = recv(clients[id].client_socket, client_reponse, 100, 0);
    client_reponse[read_size] = '\0';
    // enter the information of this clients
    sprintf(clients[id].name, "%s", client_reponse);
    pthread_mutex_lock(&waitMemset);
    sprintf(serveur_reponse, "$%s(id=%ld) is comming!", clients[id].name, id);
    serveur_reponse[strlen(serveur_reponse)] = '\0';
    printf("%s\n", serveur_reponse);
     pthread_mutex_unlock(&waitMemset);
    send_message_to_other(id, serveur_reponse,NULL);
}

/*用于删除一个client*/
void delet_client_information(long id){
    clients[id].client_socket = 0;
    memset(&clients[id].name, '\0', sizeof(clients[id].name));   
}

int give_time(char *buf)
{
    time_t t = 0;
    struct tm *tt = NULL;
    time(&t);
    tt = localtime(&t);
    sprintf(buf, "%02d:%02d:%02d\n",
            tt->tm_hour,
            tt->tm_min, tt->tm_sec);
    buf[strlen(buf)-1]=0;
    return 0;
}

void write_message_in_doc(){
    FILE *file = fopen("messages.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    for (int i = 0; i < NUM_MAX; i++)
    {
        if (clients[i].client_socket != 0) {
            fprintf(file, "Messages from %s:\n", clients[i].name);
            for (int j = 0; j < clients[i].message_count; j++) {
                fprintf(file, "%s\n", clients[i].messages[j]);
            }
            fprintf(file, "\n");
        }
    }
    fclose(file);
}

void sigintHandler(int sig_num)
{
    write_message_in_doc();
    printf("Server is shutting down. \n Closing all client connections. \n You can find the record of this conversation in message.txt\n");
    for (int i = 0; i < NUM_MAX; i++)
    {
        if (clients[i].client_socket != 0)
            close(clients[i].client_socket);
    }
    pthread_mutex_destroy(&waitMemset);
    for(int i = 0; i < NUM_MAX;i++){
        pthread_cancel(threads[i]);
    }    
    unlink("MySock");
    exit(0);
}


