  #include <stdio.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <stdlib.h>
  #include <pthread.h>
  #include <string.h>
  #include <unistd.h>

  int is_running = 1;

    char client_name[64] = "Client";
    char server_name[64] = "Server";


    static inline void trim_eol(char *s) { 
        s[strcspn(s, "\r\n")] = '\0'; 
    }


  void error(char *msg)
  {
      perror(msg);
      exit(1);
  }


  void* read_messages(void* socket_ptr)
{
    int newsockfd = *((int*)socket_ptr);
    char buffer[256];
    int n;
    
    while(is_running){
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 256);
        
        if (n <= 0){
            printf("Client Disconnected. \n");
            is_running = 0;
            break;
        }
        if (strcmp(buffer, "quit\n") ==0){
            printf("Client is disconnecting");
            is_running = 0;
            break; 
        }
        
        if (strncmp(buffer, "NAME:", 5) == 0) {
        const char *p = buffer + 5;
        if (*p) {
            strncpy(client_name, p, sizeof(client_name)-1);
            client_name[sizeof(client_name)-1] = '\0';
        }
        printf("\n[Connected with %s]\nYou> ", client_name);
        fflush(stdout);
        continue;
    }
        
        
        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        printf("\n<%s> %s\n<You> ", client_name, buffer);  
        fflush(stdout);
    }
    return NULL;
}
    

   int main(int argc, char *argv[])
   {
        int sockfd, newsockfd, portno, clilen;
        char buffer[256];
        struct sockaddr_in serv_addr, cli_addr;
        int n;
        pthread_t read_thread;

        if (argc < 2) {
            fprintf(stderr,"ERROR, no port provided\n");
            exit(1);
        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0) 
           error("ERROR opening socket");

        bzero((char *) &serv_addr, sizeof(serv_addr));

        portno = atoi(argv[1]);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr)) < 0) 
                 error("ERROR on binding");

        listen(sockfd,5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) 
             error("ERROR on accept");
        
        printf("Client has connected successfully! You can now chat.\n");
        
        if(pthread_create(&read_thread, NULL, read_messages, &newsockfd) != 0) error("Error Creating Thread");

        


        while(is_running){

            printf("<You> ");

            fflush(stdout);

            if (fgets(buffer, 256, stdin) == NULL) break;

            if (strcmp(buffer, "quit\n") == 0) {
                is_running = 0;
                break;
            }

//        bzero(buffer,256);
//        n = read(newsockfd,buffer,255);

 //       if (n < 0) error("ERROR reading from socket");

  //      printf("Here is the message: %s\n",buffer);

        n = write(newsockfd, buffer, strlen(buffer));

        if (n < 0) error("ERROR writing to socket");

        }
        
        pthread_join(read_thread, NULL);
        close(newsockfd);
        close(sockfd);

        return 0;
   }


