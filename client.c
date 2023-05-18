/*************************************************************
File name : client.c
Author : Ningxin YE, Yiqing CHEN, Yang YANG
Class : L3 MIAGE(université paris Dauphine_PSL)
Day : 2023/5/15
Compile command line : gcc client.c -pthread -o client
The command line for running the program ： ./client 172.20.10.5
For stop the programme : ctrl + c
**************************************************************/
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
int const connect_time_max=3;
int creat_client_socket(long * client_socket);
int  connect_serveur(int client_socket,char* argv);
void *client_recevie(void *socket);


int main(int argc, char const *argv[])
{
    if(argc<2){
        printf("use : ./client ip_address");
        exit(-1);
    }

    long client_socket = -1;
    pthread_t thread_client;
    //const char *ip_adresse = argv[1];
    char *client_reponse;
    //create socket error
    if(creat_client_socket(&client_socket)<0)
        return -1;
    //connect error
    if(connect_serveur(client_socket,argv[1])<0)
        return -1;
    
    pthread_create(&thread_client, NULL, client_recevie, (void *)client_socket);
    pthread_detach(thread_client);

    client_reponse = (char*)malloc(sizeof(char)*100);
    while(1){
        //fgets(client_reponse,100,stdin);
        scanf("%[^\n]",client_reponse);
        //Remove line breaks from strings
        client_reponse[strlen(client_reponse) - 1] = '\0';
        
        //scanf("%s",client_reponse);
        //printf("%s",client_reponse);
        send(client_socket,client_reponse,strlen(client_reponse),0);
        if(strncmp(client_reponse, "quit", 4) == 0)
        {
            printf("log out！\n");
            break;
        }
    }
    close(client_socket);
    return 0;
}

int creat_client_socket(long * client_socket){
    *client_socket = socket(AF_INET,SOCK_STREAM,0);
    if(client_socket<0){
        perror("error when creat client's socket");
        exit(-1);
        
    }
    printf("client socket successfully created!\n");
    return 0;
}

int  connect_serveur(int client_socket,char* argv){
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = 8080;
    serv_addr.sin_addr.s_addr = inet_addr(argv);
    int connect_time = 3;

    while(connect_time--){
    int test_connect = -1;
    test_connect = connect(client_socket, (struct sockaddr *)(&serv_addr), sizeof(struct sockaddr));
    if(test_connect>=0){
        printf("Connection to the server successful.\n");
        break;
    }
    else{
        printf("error when we try to connect serveur. try the %d time\n",connect_time_max-connect_time);
        usleep(8000);
        if(connect_time ==0)
        {   perror("connet out of time\n");
            exit(-1);
        }
        
    }
    }
    return 0;
    
}
void *client_recevie(void *socket){
    long clientS=(long)socket;
    int client_socket =(int)clientS;
    char serveur_reponse[100] ={0};
    while(1){
        int reponse_size=recv((int)client_socket,serveur_reponse,100,0);
        printf("%s\n",serveur_reponse);
    }
    pthread_exit(NULL);
    
}


