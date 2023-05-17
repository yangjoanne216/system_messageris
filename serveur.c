/********************************************************
File name : serveur.c
Author : Ningxin YE, Yiqing CHEN, Yang YANG
Class : L3 MIAGE(université paris Dauphine_PSL)
Day : 2023/5/15
Compile command line : gcc serveur.c -pthread -o serveur
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

    char name[10];
    int client_socket; // TODO : for the gestion of message for a client
}CLIENT;

CLIENT clients[NUM_MAX] = {0};

char ask_name[30] = "what's your name?\n";

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
        printf("--------------------------\n");

    }
     //close the socket
    for(int i=0;i < NUM_MAX;i++){
    if(clients[i].client_socket!=0)
        close(clients[i].client_socket);
    }

    close(sev_socket);
    return 0;
}

int creat_socket(){
    int sev_socket = socket(AF_INET,SOCK_STREAM,0);
    if(sev_socket<0){
        perror("error when we creat socket!!!\n");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = 8080;
    addr.sin_addr.s_addr=0;

    //这段代码不确定
    /*int reuse = 1;
    int ret = -1;
    ret = setsockopt(sev_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));*/

    int test_bind = bind(sev_socket, (struct sockaddr *)&addr,sizeof(struct sockaddr));

    if(test_bind == -1){
        perror("error when bind!\n");
        exit(-1);
    }

    int test_listen = listen(sev_socket,NUM_MAX);
    if(test_listen == -1){
        perror("error whe listen");
        exit(-1);
    }
    printf("The server has been successfully created!\n");
    return sev_socket;
}

int accept_client(int sev_socket){
    struct sockaddr_in client_addr;
    int addrlen= sizeof(client_addr);
    memset(&client_addr,0,sizeof(struct sockaddr_in));
    printf("wait for the connexion of client\n");
    int client_socket = accept(sev_socket,(struct sockaddr *)&client_addr,&addrlen);
    if(client_socket < 0){
        perror("erro when we connext the client");
        exit(-1);
    }
    int id = 0;
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
}

void *se_connecter_client(void *arg){
    long id = (long)arg;
    int read_size=0;
    while (1)
    {
        char *client_reponse;
        char *serveur_reponse;
        //char *client_reponse_with_time;
        char sperotor[30]="----------------\n";

        //Dynamically allocated memory
        client_reponse = (char*)malloc(sizeof(char)*100);
        serveur_reponse= (char*)malloc(sizeof(char)*100);
        //client_reponse_with_time = (char*)malloc(sizeof(char)*100);

        //if a client have connected
        if(clients[id].client_socket){
            // if the client hasn't a name(the fist time of connexion)
            if(strlen(clients[id].name)==0){
                send(clients[id].client_socket, ask_name,sizeof(ask_name),0);
                read_size=recv(clients[id].client_socket,client_reponse,100,0);
                client_reponse[read_size] = '\0';
                //enter the information of this clients
                sprintf(clients[id].name,"%s",client_reponse);
                sprintf(serveur_reponse,"id= %ld, %s is　comming",id,clients[id].name);
                printf("%s\n",serveur_reponse);
                //Notify other clients of the client's arrival
                for(int i = 0; i<NUM_MAX;i++){
                    if(i != id){
                        send(clients[i].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                    }
                }
            //Jump out of this (first) loop to execute the next one
            continue;
            }
        
        read_size= recv(clients[id].client_socket,client_reponse,100,0);
        client_reponse[read_size] = '\0';

        //this client is out of connexion or erro when read o
        if(read_size<0){
            printf("%s is out of connexion", clients[id].name);
            //delet this clients
            //TODO : 将clients的信息发送到一个文件里
            clients[id].client_socket = 0;
            memset(&clients[id].name,'\0',sizeof(clients[id].name));
            pthread_exit(NULL);
            break;
        } 
        // the read_size is not 0
        else{
            char messageTime[100] = {0};
            give_time(messageTime);
            printf("receive of the message of client %s : %s \n ",clients[id].name, client_reponse);
            sprintf(serveur_reponse,"%s%s : %s",messageTime,clients[id].name, client_reponse);

            // send the messages to all the other clients
            for(int i = 0; i<NUM_MAX; i++){
                if(i!=id){
                    //Send Separator to all the other clients
                    send(clients[i].client_socket,sperotor,strlen(sperotor),0);
                    // send the messages to all the other clients
                    send(clients[i].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                }
            }
        }    

        // when the client want to quit
        if(strncmp(client_reponse,"quit",4)==0)
        {
            printf("id=%ld, %s log out\n",id,clients[id].name);
            sprintf(serveur_reponse,"message from serveur:id=%ld, %s log out\n",id,clients[id].name);
            //send the log other information to all the others clients
            for(int i = 0; i<NUM_MAX; i++){
                if(i!=id){
                    //Send Separator to all the other clients
                    send(clients[id].client_socket,sperotor,strlen(sperotor),0);
                    // send the messages to all the other clients
                    send(clients[id].client_socket,serveur_reponse,strlen(serveur_reponse),0);
                }
            }
            //delect the information of clients
            clients[id].client_socket = 0;
            memset(&clients[id].name,'\0',sizeof(clients[id].name));
            pthread_exit(NULL);
            break;
        }
           
    }
    }
    pthread_exit(NULL);
}





