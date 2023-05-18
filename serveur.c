/********************************************************
File name : serveur.c
Author : Ningxin YE, Yiqing CHEN, Yang YANG
Class : L3 MIAGE(université paris Dauphine_PSL)
Day : 2023/5/15
Compile command line : gcc serveur.c -o serveur
The command line for running the program ： 
For stop the programme : ctrl + c
*********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define NUM_MAX 10

int creat_socket();
int accept_client(int sev_socket);
void *se_connecter_client(void *arg);

pthread_t threads[NUM_MAX];

typedef struct{
    char name[100];
    int client_socket; 
    char ** messages; 
    // Etape 2 : for the gestion of message for a client
}CLIENT;

CLIENT clients[NUM_MAX] = {0};

char ask_name[30] = "what's your name?";

int give_time(char *buf){
    time_t t = 0;
    struct tm *tt = NULL;
    time(&t);
    tt = localtime(&t);
    sprintf(buf, "%02d-%02d %02d:%02d:%02d\n", 
            (tt->tm_mon+1), 
            tt->tm_mday, tt->tm_hour, 
            tt->tm_min, tt->tm_sec
            );
    return 0;

}
int main(int argc, char const *argv[])
{
    /*Create socket*/
    int sev_socket = creat_socket();
    int client_socket;
    /*Accepter les connexions des clients*/
    while(1){
        client_socket = accept_client(sev_socket);
        printf("----------------------------------------\n");
    }   
   // printf("all the message");
     //close the socket
    for(int i=0;i < NUM_MAX;i++){
    if(clients[i].client_socket!=0)
        close(clients[i].client_socket);
    }

    close(sev_socket);
    return 0;
}

int creat_socket(){
    int sev_socket = -1;
    sev_socket = socket(AF_INET,SOCK_STREAM,0);
    if(sev_socket<0){
        perror("error when we creat socket!!!\n");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = 8080;
    addr.sin_addr.s_addr=0;

    int test_bind = bind(sev_socket, (struct sockaddr *)&addr,sizeof(struct sockaddr));

    if(test_bind <0){
        perror("error when bind!\n");
        exit(-1);
    }

    int test_listen = listen(sev_socket,NUM_MAX);
    if(test_listen <0){
        perror("error whe listen");
        exit(-1);
    }
    printf("The server has been successfully created!\n");
    return sev_socket;
}

/*int accept_client(int sev_socket){
    struct sockaddr_in client_addr;
    int addrlen= sizeof(client_addr);
    memset(&client_addr,0,sizeof(struct sockaddr_in));
    printf("wait for the connexion of client\n");
    int client_socket = accept(sev_socket,(struct sockaddr *)&client_addr,&addrlen);
    if(client_socket < 0){
        perror("erro when we connext the client");
        exit(-1);
    }
    //int id = 0;
    for(int i = 0; i<NUM_MAX; i++){
        if(clients[i].client_socket==0){
            clients[i].client_socket = client_socket;
            pthread_create(threads+i,NULL,se_connecter_client,(void *)(long)i);
            pthread_detach(threads[i]);
            break;
        }
    }
    printf("connext client adresse: %s\n",inet_ntoa(client_addr.sin_addr));
    return client_socket;
}*/

int accept_client(int sev_socket) {
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    printf("Waiting for client connection...\n");
    int client_socket = accept(sev_socket, (struct sockaddr*)&client_addr, &addrlen);
    if (client_socket < 0) {
        perror("Error when accepting client connection");
        exit(-1);
    }

    // Find an available slot in the clients array
    int client_index = -1;
    for (int i = 0; i < NUM_MAX; i++) {
        if (clients[i].client_socket == 0) {
            client_index = i;
            break;
        }
    }

    if (client_index != -1) {
        clients[client_index].client_socket = client_socket;
        pthread_create(&threads[client_index], NULL, se_connecter_client, (void*)(intptr_t)client_index);
        pthread_detach(threads[client_index]);
        printf("Connected with client at address: %s\n", inet_ntoa(client_addr.sin_addr));
    } else {
        printf("Maximum number of clients reached. Connection refused.\n");
        close(client_socket);
    }

    return client_socket;
}


void *se_connecter_client(void *arg){
    long id = (long)arg;
    int read_size=0;
    char sperotor[30]="----------------\n";
    while (1)
    {  
        char client_reponse [100]={0};
        char serveur_reponse[100]={0};
        //if a client have connected
        if(clients[id].client_socket)
        {
            // if the client hasn't a name(the fist time of connexion)
            if(strlen(clients[id].name)==0)
            {
                send(clients[id].client_socket, ask_name,strlen(ask_name),0);
                read_size=recv(clients[id].client_socket,client_reponse,100,0);
                client_reponse[read_size] = '\0';
                //enter the information of this clients
                sprintf(clients[id].name,"%s",client_reponse);      
                sprintf(serveur_reponse,"$%s(id=%ld) is comming!",clients[id].name,id);
                serveur_reponse[strlen(serveur_reponse)] = '\0';
                printf("%s\n",serveur_reponse);
                for(int i = 0; i<NUM_MAX;i++){
                    if(i != id&&clients[i].client_socket != 0){
                        send(clients[i].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                    }
                }
                memset(serveur_reponse,0,sizeof(serveur_reponse));
            continue;
            }
        
        read_size= recv(clients[id].client_socket,client_reponse,100,0);
        client_reponse[read_size] = '\0';
        //this client is out of connexion or erro when read o
        if(read_size<0){
            printf("%s is out of connexion\n", clients[id].name);
            //delet this clients
            //TODO : 将clients的信息发送到一个文件里
            clients[id].client_socket = 0;
            memset(&clients[id].name,'\0',sizeof(clients[id].name));
            pthread_exit(NULL);
            break;
        } 
        // the read_size is not 0
        else
        {
            char messageTime[100] = {0};
            give_time(messageTime);
            printf("receive of the message of client %s : %s \n ",clients[id].name, client_reponse);  
            sprintf(serveur_reponse,"%s%s : %s",messageTime,clients[id].name, client_reponse);
            serveur_reponse[strlen(serveur_reponse)]='\0';
            // send the messages to all the other clients
            for(int i = 0; i<NUM_MAX; i++)
            {
                if(i!=id&&clients[i].client_socket != 0)
                {
                    //Send Separator to all the other clients
                    send(clients[i].client_socket,sperotor,strlen(sperotor),0);
                    // send the messages to all the other clients( There is always a problem with this place and the information received is not accurate. )
                    send(clients[i].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                    //sleep(10);
                }
            }
        }    

        // when the client want to quit
        if(strncmp(client_reponse,"quit",4)==0)
        {
            printf("id=%ld, %s log out\n",id,clients[id].name);
            sprintf(serveur_reponse,"message from serveur:id=%ld, %s log out\n",id,clients[id].name);
            serveur_reponse[strlen(serveur_reponse)]='\0';
            //send the log other information to all the others clients
            for(int i = 0; i<NUM_MAX; i++){
                if(i!=id&&clients[i].client_socket != 0){
                    //Send Separator to all the other clients
                    send(clients[id].client_socket,sperotor,strlen(sperotor),0);
                    // send the messages to all the other clients
                    send(clients[id].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                }
            }
            //delect the information of clients
            clients[id].client_socket = 0;
            memset(&clients[id].name,'\0',sizeof(clients[id].name));
            break;
        }
           
        }
    }
    pthread_exit(NULL);
}

/*void *se_connecter_client(void *arg) {
    long id = (long)arg;
    char sperotor[30] = "----------------\n";    
    while (1) {
        char client_reponse[100];
        char serveur_reponse[100];
        int read_size = 0;
        memset(serveur_reponse, 0, sizeof(serveur_reponse));
        memset(client_reponse, 0, sizeof(client_reponse));
        read_size = recv(clients[id].client_socket, client_reponse, sizeof(client_reponse), 0);
        client_reponse[read_size] = '\0';
        if (read_size <= 0) {
            printf("%s is disconnected\n", clients[id].name);        
           // Notify other clients about the disconnection
            for (int i = 0; i < NUM_MAX; i++) {
                if (i != id && clients[i].client_socket != 0) {
                    send(clients[i].client_socket, sperotor, strlen(sperotor), 0);
                    send(clients[i].client_socket, serveur_reponse, strlen(serveur_reponse)+1, 0);
                }
            }
                    // Clear client information
            close(clients[id].client_socket);
            clients[id].client_socket = 0;
            memset(clients[id].name, '\0', sizeof(clients[id].name));
            break;
        }

        // Handle client's message
        char messageTime[100] = {0};
        give_time(messageTime);
        printf("Received message from client %s: %s\n", clients[id].name, client_reponse);
        sprintf(serveur_reponse, "%s%s : %s", messageTime, clients[id].name, client_reponse);
        strncat(serveur_reponse, "\n", sizeof(serveur_reponse) - strlen(serveur_reponse) - 1);

        for (int i = 0; i < NUM_MAX; i++) {
            if (i != id && clients[i].client_socket != 0) {
                
                send(clients[i].client_socket, sperotor, strlen(sperotor), 0);
                send(clients[i].client_socket, serveur_reponse, strlen(serveur_reponse)+1, 0);
            } d
        }
    }

    pthread_exit(NULL);
}*/



