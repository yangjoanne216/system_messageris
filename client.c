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
#include<pthread.h>
#include <sys/un.h> 

int const connect_time_max=3;
int creat_client_socket(long * client_socket);
int  connect_serveur(int client_socket);
void *client_recevie(void *socket);
int main(int argc, char const *argv[])
{
    long client_socket = -1;
    pthread_t thread_client;
    //const char *ip_adresse = argv[1];
    char *client_reponse;
    //create socket error
    if(creat_client_socket(&client_socket)<0)
        return -1;
     //connect error
    if(connect_serveur(client_socket) <0)
        return -1;
    
    pthread_create(&thread_client, NULL, client_recevie, (void *)client_socket);
    pthread_detach(thread_client);

    client_reponse = (char*)malloc(sizeof(char)*100);
    while(1){
        fgets(client_reponse,100,stdin);
        //scanf("%[^\n]",client_reponse);
        //Remove line breaks from strings
        client_reponse[strlen(client_reponse) - 1] = '\0';
       
        send(client_socket,client_reponse,strlen(client_reponse),0);
        if(strncmp(client_reponse, "quit", 4) == 0)
        {   
            printf("log out！\n");
            pthread_cancel(thread_client);  // 关闭接收线程
            break;
        }
    }
    close(client_socket);
    return 0;
}

int creat_client_socket(long * client_socket){
    *client_socket = socket(AF_UNIX,SOCK_STREAM,0);
    if(client_socket<0){
        perror("error when creat client's socket");
        exit(-1);
        
    }
    printf("client socket successfully created!\n");
    return 0;
}

int  connect_serveur(int client_socket){
    struct sockaddr_un serv_addr={0};
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "./MySock");	// Chemin vers fichier socket
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
        serveur_reponse[reponse_size] = '\0';  // 确保字符串正确地结束
        // 如果收到服务器关闭的特殊消息，就退出
        if (strcmp(serveur_reponse, "SERVER_EXIT") == 0) {
            printf("Server is closing. Exiting client.\n");
            close(client_socket);
            exit(0);
        }

        printf("%s\n",serveur_reponse);
        memset(serveur_reponse, 0, sizeof(serveur_reponse));
    }
    pthread_exit(NULL);
    
}


