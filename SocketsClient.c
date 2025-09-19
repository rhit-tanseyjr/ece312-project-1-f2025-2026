   #include <stdio.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netdb.h> 
   #include <stdlib.h>
   #include <unistd.h>
   #include <string.h>
   #include <pthread.h>
   #include <arpa/inet.h>
    #include <netdb.h>




   int is_running = 1;

    char client_name[64] = "Client";
    char server_name[64] = "Server";


    static inline void trim_eol(char *s) { 
        s[strcspn(s, "\r\n")] = '\0'; 
    }

   void error(char *msg)
   {
       perror(msg);
       exit(0);
   }

   void* read_messages(void* socket_ptr)
{   int sockfd = *((int*)socket_ptr);
    char buffer[256];
    int n;

    while(is_running){
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        
        if(n <= 0){
            printf("Server Disconnected.\n");
            is_running = 0;
            break;
        }

        if(strcmp(buffer, "quit\n") == 0){
            printf("Server is disconnecting...\n");
            is_running = 0;
            break;
        }

        if (strncmp(buffer, "NAME:", 5) == 0) {
            const char *p = buffer + 5;
            if (*p) {
                strncpy(server_name, p, sizeof(server_name)-1);
                server_name[sizeof(server_name)-1] = '\0';
            }
            printf("\n[Connected with %s]\nYou> ", server_name);
            fflush(stdout);
            continue;
        }



        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        printf("\n<%s> %s\n<You> ", server_name, buffer);  
        fflush(stdout);
}
    return NULL;
}

   int main(int argc, char *argv[])
   {
     int sockfd, portno, n;
     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[256];
     pthread_t read_thread;

     if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
     }

     portno = atoi(argv[2]);
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) 
         error("ERROR opening socket");
     server = gethostbyname(argv[1]);

     if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         exit(0);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;

     bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

     serv_addr.sin_port = htons(portno);

     if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
         error("ERROR connecting");

    // printf("Enter your name: ");
    // fflush(stdout);
    // if (fgets(client_name, sizeof(client_name), stdin)) trim_eol(client_name);
    // if (client_name[0] == '\0') strcpy(client_name, "Me");

    // char hello[128];
    // snprintf(hello, sizeof(hello), "NAME:%s\n", client_name);
    // if (write(sockfd, hello, strlen(hello)) < 0) {
    //     perror("write NAME");
    //     // (optional) exit or continue
    // }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, server->h_addr, ip, sizeof(ip));

    
    printf("Connected to %s (%s):%d\n", argv[1], ip, portno);
    printf("Connected to the Server! You can now chat. Type 'quit' to exit.\n");


     if (pthread_create(&read_thread, NULL, read_messages, &sockfd) != 0) {
        error("ERROR creating thread");
     }
     
     while(is_running){
        printf("<You> ");
        fflush(stdout);
        
        if(fgets(buffer, 256, stdin) == NULL) break;
        
        if(strcmp(buffer, "quit\n") == 0){
            is_running = 0;
            break;
        }

        n = write(sockfd, buffer, strlen(buffer));
        if(n < 0) error("Error Writing to socket");
        }
     //bzero(buffer,256);
     //n = write(sockfd,buffer,strlen(buffer));
     //if (n < 0) 
      //    error("ERROR writing to socket");
     //bzero(buffer,256);
     //n = read(sockfd,buffer,255);
     //if (n < 0) 
      //    error("ERROR reading from socket");
     //printf("%s\n",buffer);

     pthread_join(read_thread, NULL);
     close(sockfd);
     printf("Client disconnected\n");
     return 0;
   }





//    #include <stdio.h>
//    #include <sys/types.h>
//    #include <sys/socket.h>
//    #include <netinet/in.h>
//    #include <netdb.h> 
//    #include <stdlib.h>
//    #include <unistd.h>
//    #include <string.h>
//    #include <pthread.h>



//    int is_running = 1;

//    void error(char *msg)
//    {
//        perror(msg);
//        exit(0);
//    }

//    void* read_messages(void* socket_ptr)
// {   int sockfd = *((int*)socket_ptr);
//     char buffer[256];
//     int n;

//     while(is_running){
//         bzero(buffer, 256);
//         n = read(sockfd, buffer, 255);
        
//         if(n <= 0){
//             printf("Server Disconnected.\n");
//             is_running = 0;
//             break;
//         }

//         if(strcmp(buffer, "quit\n") == 0){
//             printf("Server is disconnecting...\n");
//             is_running = 0;
//             break;
//         }
//         printf("Server: %s", buffer);
// }
//     return NULL;
// }

//    int main(int argc, char *argv[])
//    {
//      int sockfd, portno, n;
//      struct sockaddr_in serv_addr;
//      struct hostent *server;
//      char buffer[256];
//      pthread_t read_thread;

//      if (argc < 3) {
//         fprintf(stderr,"usage %s hostname port\n", argv[0]);
//         exit(0);
//      }

//      portno = atoi(argv[2]);
//      sockfd = socket(AF_INET, SOCK_STREAM, 0);

//      if (sockfd < 0) 
//          error("ERROR opening socket");
//      server = gethostbyname(argv[1]);

//      if (server == NULL) {
//          fprintf(stderr,"ERROR, no such host\n");
//          exit(0);
//      }

//      bzero((char *) &serv_addr, sizeof(serv_addr));
//      serv_addr.sin_family = AF_INET;

//      bcopy((char *)server->h_addr, 
//           (char *)&serv_addr.sin_addr.s_addr,
//           server->h_length);

//      serv_addr.sin_port = htons(portno);

//      if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
//          error("ERROR connecting");

//      printf("Connected to the Server! You can now chat. Type 'quit' to exit.\n");
     
//      if (pthread_create(&read_thread, NULL, read_messages, &sockfd) != 0) {
//         error("ERROR creating thread");
//      }
     
//      while(is_running){
//         printf("Client: ");
//         fflush(stdout);
        
//         if(fgets(buffer, 256, stdin) == NULL) break;
        
//         if(strcmp(buffer, "quit\n") == 0){
//             is_running = 0;
//             break;
//         }

//         n = write(sockfd, buffer, strlen(buffer));
//         if(n < 0) error("Error Writing to socket");
//         }
//      //bzero(buffer,256);
//      //n = write(sockfd,buffer,strlen(buffer));
//      //if (n < 0) 
//       //    error("ERROR writing to socket");
//      //bzero(buffer,256);
//      //n = read(sockfd,buffer,255);
//      //if (n < 0) 
//       //    error("ERROR reading from socket");
//      //printf("%s\n",buffer);

//      pthread_join(read_thread, NULL);
//      close(sockfd);
//      printf("Client disconnected\n");
//      return 0;
//    }
