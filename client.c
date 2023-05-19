/*************************************************************
* File name : client.c
* Version 6
* Author : Ningxin YE, Yiqing CHEN, Yang YANG
* L3 MIAGE(université paris Dauphine_PSL)
* Date : 2023/5/15
* Compile command : gcc client.c -pthread -o client
* Command to run the program : ./client 
* puts the program into edit mode : ctrl + C
* Terminate the program: quit
**************************************************************/
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<pthread.h>
#include <sys/un.h> 

#define MESSAGE_SIZE 1000 /*The length of the temporarily stored message*/

int const connect_time_max=3; /*Maximum number of attempts to connect to the server*/

/*0 means not in edit-only mode, 1 means in edit-only mode.
Messages sent by other clients are not displayed while the client is in edit_only_mode
ctrl + c -> enter the edit_only_mode, 
sending a message -> exit edit_only_mode*/
int edit_only_mode = 0;  
char pendingMessages[MESSAGE_SIZE][MESSAGE_SIZE];  /*Temporary storage of messages received at edit_only_mode*/
int pendingMessagesCount = 0;  /*Number of messages stored*/


/* Declaration of functions */
int creat_client_socket(long * client_socket);
int  connect_serveur(int client_socket);
void *client_recevie(void *socket);
void handle_sigint(int sig);

int main(int argc, char const *argv[])
{
    long client_socket = -1; 
    signal(SIGINT, handle_sigint); 
    pthread_t thread_client;
    char *client_reponse;

    if(creat_client_socket(&client_socket)<0)
        return -1;
    if(connect_serveur(client_socket) <0)
        return -1;
    
    pthread_create(&thread_client, NULL, client_recevie, (void *)client_socket);
    pthread_detach(thread_client);

    client_reponse = (char*)malloc(sizeof(char)*100);
    while(1){
        fgets(client_reponse,100,stdin);
        //scanf("%[^\n]",client_reponse);
        /*Remove line breaks from strings*/
        client_reponse[strlen(client_reponse) - 1] = '\0';
        send(client_socket,client_reponse,strlen(client_reponse),0);
        if(strncmp(client_reponse, "quit", 4) == 0)
        {   
            printf("logged out！\n");
            pthread_cancel(thread_client);  
            break;
        }
        /*Exit edit_only mode*/
        edit_only_mode = 0;  
        /*Show messages received during edit_only_mode*/
        if(pendingMessagesCount!=0)
        {
             printf("-----Exiting edit-only mode.--------\nThe following are all the messages you received during the edit only mode: \n");
            for (int i = 0; i < pendingMessagesCount; i++) 
            {
                printf("%s\n", pendingMessages[i]);
            }
            pendingMessagesCount = 0;

        }     
    }
    close(client_socket);
    return 0;
}

/**
* @function:        creat_client_socket
* @brief            Create a unix client socket
* @return           >0 a file descriptor pointing to socket 
*                   <0 fail 
*/
int creat_client_socket(long * client_socket){
    *client_socket = socket(AF_UNIX,SOCK_STREAM,0);
    if(client_socket<0){
        perror("Client socket creation error.");
        exit(-1);
        
    }
    printf("client socket successfully created!\n");
    return 0;
}

/**
* @fuction:     accept_client
* @brief        Connect to the server
* @param[in]    client_socket   The client's socket file descriptor
* @return       0:success -1:fail
*/
int  connect_serveur(int client_socket){
    struct sockaddr_un serv_addr={0};
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "./MySock");	/*Chemin vers fichier socket*/
    int connect_time = 3;

    while(connect_time--){
    int test_connect = -1;
    test_connect = connect(client_socket, (struct sockaddr *)(&serv_addr), sizeof(struct sockaddr));
    if(test_connect>=0){
        printf("Connection to the server successful.\n");
        break;
    }
    else{
        printf("Connection error. Retry attempt: %d\n",connect_time_max-connect_time);
        usleep(8000);
        if(connect_time ==0)
        {   perror("Connection timed out.\n");
            exit(-1);
        }
        
    }
    }
    return 0;   
}

/**
 * @function:    client_receive
 * @brief       Handles the reception of messages for a client
 *              If a "SERVER_EXIT" message is received, it exits.
 *              In edit_only_mode, the received messages are stored in a pending messages buffer; 
 *              otherwise, they are printed directly to the console. 
 * @param[in]   socket   The client's socket
 * @return      void
 */
void *client_recevie(void *socket){
    long clientS=(long)socket;
    int client_socket =(int)clientS;
    char serveur_reponse[100] ={0};
    while(1){
        int reponse_size=recv((int)client_socket,serveur_reponse,100,0);
        serveur_reponse[reponse_size] = '\0';  
        /*receive a message "SERVER_EXIT", exit*/
        if (strcmp(serveur_reponse, "SERVER_EXIT") == 0) {
            printf("Server is closing. Exiting client.\n");
            close(client_socket);
            exit(0);
        }
        /*the received messages are stored in a pending messages buffer,otherwise, they are printed directly to the console. */
        if (edit_only_mode) {
            strncpy(pendingMessages[pendingMessagesCount++], serveur_reponse, MESSAGE_SIZE-1);
            pendingMessages[pendingMessagesCount-1][MESSAGE_SIZE-1] = '\0';  // 确保字符串正确地结束
        } else {
            printf("%s\n",serveur_reponse);
            
        }      
        memset(serveur_reponse, 0, sizeof(serveur_reponse));
    }
    pthread_exit(NULL);  
}

/**
 * @function:   sigintHandler·
 * @brief       Handles the SIGINT signal ctrl+c
 *              ctrl +c -> enter edit_only_mode      
 * @param[in]   sig_num   The signal number
 * @return      void
 */
void handle_sigint(int sig) {
   edit_only_mode = !edit_only_mode;  
    if (edit_only_mode) {
        printf("-----Edit only mode (ctrl+c)--------\nenter your message: \n");
    } 
}


