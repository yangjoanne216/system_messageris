/********************************************************
* File name : serveur.c
* Version 6
* Author : Ningxin YE, Yiqing CHEN, Yang YANG
* L3 MIAGE(université paris Dauphine_PSL)
* Date : 2023/5/15
* Compile command : gcc serveur.c -pthread -o serveur
* Command to run the program ：./serveur
* Terminate the program: ctrl + c
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

#define NUM_MAX 10 /*Maximum number of clients*/
#define MAX_MESSAGES 1000 //*Maximum number of messages a client can send*/
#define MAX_MESSAGE_LENGTH 100 /*Maximum length of one client‘s message*/

pthread_t threads[NUM_MAX]; /*One client occupies one thread*/
pthread_mutex_t waitMemset; /*protect access to the buffer (serveur_reponse)*/

/*This structure denotes a client, 
comprising its name, socket, 
and an array of messages sent during the program's runtime,
which are logged to messages.txt upon server exit.*/
typedef struct
{
    char name[100];
    int client_socket;
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    int message_count;
} CLIENT;
CLIENT clients[NUM_MAX] = {0};

/* Declaration of functions */
int creat_socket();
int accept_client(int sev_socket);
void *se_connecter_client(void *arg);
void send_message_to_other(long id, char *serveur_reponse, char *sperotor);
void first_connect(long id, char *client_reponse, char *serveur_reponse);
void delet_client_information(long id);
void give_time(char *buf);
void sigintHandler(int sig_num);
void write_message_in_doc();

int main(int argc, char const *argv[])
{
    signal(SIGINT, sigintHandler);
    int sev_socket = creat_socket();
    int client_socket;
    pthread_mutex_init(&waitMemset,NULL);
    while (1)
    {
        client_socket = accept_client(sev_socket);
        printf("***********************************\n");
    }
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


/**
* @function:        creat_socket
* @brief            Create a unix serveur socket
* @return           >0 a file descriptor pointing to socket 
*                   <0 fail 
*/
int creat_socket()
{
    unlink("Mysock");
    /*Initialisation struct addr serveur*/
    struct sockaddr_un addr ={0}; 
    memset(&addr, 0, sizeof(struct sockaddr_un)); 
    addr.sun_family = AF_UNIX;	
    strcpy(addr.sun_path, "./MySock");	
    /*Creation socket serveur*/
    int sev_socket = -1;
    sev_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sev_socket < 0)
    {
        perror("Socket creation error!\n");
        exit(-1);
    }
    /*Connection de la socket a l'addresse serveur*/
    int test_bind = bind(sev_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (test_bind < 0)
    {
        perror("Binding error!\n");
        exit(-1);
    }
    /* Socket en attente de connexion (a l'ecoute)*/
    int test_listen = listen(sev_socket, NUM_MAX);
    if (test_listen < 0)
    {
        perror("Listening error!");
        exit(-1);
    }
    printf("The server has been successfully created!\n");
    return sev_socket;
}

/**
* @fuction:     accept_client
* @brief        Accepts a connection from a client 
* @param[in]    sev_socket   The server's socket file descriptor
* @return       >0 a file descriptor pointing to socket or -1 fail 
*/
int accept_client(int   sev_socket)
{
    //*Initialisation struct addr client*/
    struct sockaddr_un client_addr = {0};
    socklen_t addrlen = sizeof(client_addr);
    printf("Waiting for client connection...\n");
    int client_socket = accept(sev_socket, (struct sockaddr *)&client_addr, &addrlen);
    if (client_socket < 0)
    {
        perror("Error when accepting client connection");
        exit(-1);
    }
    /*Find an available slot in the clients array*/
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
    }
    else
    {
        printf("Maximum number of clients reached. Connection refused.\n");
        close(client_socket);
    }

    return client_socket;
}

/**
*@function:     se_connecter_client
*@brief         Manages communication with a connected client.
*               it assigns a name to the client
*               handles messages and disconnections. 
*               appends a timestamp and client's name to messages,
*               broadcasts them, 
*s              tores them in the client's array
*@return null
*/
void *se_connecter_client(void *arg)
{
    long id = (long)arg;
    int read_size = 0;
    char sperotor[30] = "----------------\n";
    while (clients[id].client_socket!=0)
    {
        char client_reponse[100] = {0};
        char serveur_reponse[100] = {0};
            /*the client hasn't a name(the fist time of connexion)*/
            if (strlen(clients[id].name) == 0)
            {
                first_connect(id, client_reponse, serveur_reponse);
                memset(serveur_reponse, 0, sizeof(serveur_reponse));
                memset(client_reponse,0,sizeof(client_reponse));
                continue;
            }
            read_size = recv(clients[id].client_socket, client_reponse, 100, 0);
            client_reponse[read_size] = '\0';
            /*this client disconnected or erro when read o*/
            if (read_size <= 0||strncmp(client_reponse, "quit", 4) == 0)
            {
                printf("%s(ID = %ld) disconnected\n", clients[id].name,id);
                sprintf(serveur_reponse,"%s(ID = %ld) disconnected\n",clients[id].name,id);
                send_message_to_other(id,serveur_reponse,sperotor);
                delet_client_information(id);
                pthread_exit(NULL);
                break;
            }
            /*normal situation*/
            else
            {
                char messageTime[100] = {0};
                give_time(messageTime);
                printf("Message received from clien %s(ID=%ld) : %s \n ",clients[id].name,id, client_reponse);
                sprintf(serveur_reponse, "%s %s(ID=%ld) : %s", messageTime, clients[id].name, id,client_reponse);
                send_message_to_other(id,serveur_reponse,sperotor);
                /*store the transmitted messages in the client structure's messages array for later output to a file*/
                if (clients[id].message_count < MAX_MESSAGES) {
                strncpy(clients[id].messages[clients[id].message_count], client_reponse, MAX_MESSAGE_LENGTH);
                clients[id].message_count++;
            }
            }
        }
    
    pthread_exit(NULL);
}

/**
* @function:    send_message_to_other
* @brief        Accepts a connection from a client 
* @param[in]    id               The client's id who sent the message
* @param[in]    serveur_reponse  The server's response message to be broadcasted
* @param[in]    sperotor         A separator string, can be NULL
* @return       null
*/
void send_message_to_other(long id, char *serveur_reponse, char *sperotor)
{
    pthread_mutex_lock(&waitMemset);
    serveur_reponse[strlen(serveur_reponse)]='\0';
    for (int i = 0; i < NUM_MAX; i++)
    {
        if (i != id && clients[i].client_socket != 0)
        {
             char message_to_send[1000];  
            strcpy(message_to_send, serveur_reponse); 

            if(sperotor!=NULL){ send(clients[i].client_socket, sperotor, strlen(sperotor), 0);}
            send(clients[i].client_socket, message_to_send, strlen(message_to_send), 0);
        }
    }
    memset(serveur_reponse,0,strlen(serveur_reponse));
    pthread_mutex_unlock(&waitMemset);
    
}

/**
 * @function:    first_connect
 * @brief       Assigns a name to a client during their first connection
 *              It asks the client for their name
 * @param[in]   id                The client's id
 * @param[out]  client_reponse    Buffer to store the client's response
 * @param[out]  serveur_reponse   Buffer to store the server's response
 * @return      null
 */
void first_connect(long id, char *client_reponse, char *serveur_reponse)
{
    char ask_name[30] = "what's your name?";
    send(clients[id].client_socket, ask_name, strlen(ask_name), 0);
    int read_size = recv(clients[id].client_socket, client_reponse, 100, 0);
    client_reponse[read_size] = '\0';
    // enter the information of this clients
    sprintf(clients[id].name, "%s", client_reponse);
    pthread_mutex_lock(&waitMemset);
    sprintf(serveur_reponse, "Client $%s(ID=%ld) has entered the session.", clients[id].name, id);
    serveur_reponse[strlen(serveur_reponse)] = '\0';
    printf("%s\n", serveur_reponse);
     pthread_mutex_unlock(&waitMemset);
    send_message_to_other(id, serveur_reponse,NULL);
}

/**
 * @function:   delet_client_information
 * @brief       Delete a client       
 * @param[in]   id               The client's id
 * @return      null
 */
void delet_client_information(long id){
    clients[id].client_socket = 0;
    memset(&clients[id].name, '\0', sizeof(clients[id].name));   
}

/**
 * @function:   give_time
 * @brief       Generate current time  
 * @param[out]  buf             Buffer to store the current time
 * @return      null
 */
void give_time(char *buf)
{
    time_t t = 0;
    struct tm *tt = NULL;
    time(&t);
    tt = localtime(&t);
    sprintf(buf, "%02d:%02d:%02d\n",
            tt->tm_hour,
            tt->tm_min, tt->tm_sec);
    buf[strlen(buf)-1]=0;
   
}

/**
 * @function:   write_message_in_doc
 * @brief       Writes all client messages into a document
 *              This function opens a file called "messages.txt" for writing. 
 *              It iterates over all connected clients and writes each client's name followed by all their messages into the file. 
 * @return      void
 */
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

/**
 * @function:   sigintHandler·
 * @brief       Handles the SIGINT signal ctrl+c)
 *              After the server has exited (ctrl+c), 
 *              send a message to get all connected clients to exit.
 *              The messageris stored in the client are written to messages.txt
 * @param[in]   sig_num   The signal number
 * @return      void
 */
void sigintHandler(int sig_num)
{
    write_message_in_doc();
    for (int i = 0; i < NUM_MAX; i++) {
        if (clients[i].client_socket != 0) {
            write(clients[i].client_socket, "SERVER_EXIT" , strlen("SERVER_EXIT"));
        }
    }
    printf("Server is shutting down. \n Closing all client connections. \n You can find the record of this conversation in message.txt\n");
    pthread_mutex_destroy(&waitMemset);
    for(int i = 0; i < NUM_MAX;i++){
        pthread_cancel(threads[i]);
    }    
    unlink("MySock");
    exit(sig_num);
}


