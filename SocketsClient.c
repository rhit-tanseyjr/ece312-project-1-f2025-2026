/* 
    SocketsClient.c
    ECE 312 Communication Networks - Project 1
   
    Description: TCP-based client-server application that establishes a connection
    with a single client and enables bidirectional real-time messaging.
    The server accepts incoming connections, exchanges usernames, and
    allows both hosts to send and receive messages until 'quit' is entered.

    Features:    - Prompts for username at startup
                 - Displays client IP address upon connection
                 - Multithreaded design for simultaneous send/receive
                 - Messages labeled with sender username
                 - Exit with 'quit' command

    Usage:       ./client <server_ip_address> <port_number>
    Example:     ./client 137.112.197.190 8081
    Compilation: gcc SocketsClient.c -o client

    Author(s):   John Tansey & Elliott Rogers
    Date:        9-19-2025
    Course:      ECE 312 - Communication Networks

    Notes:       - Uses POSIX threads for concurrent message handling
                 - Server binds to INADDR_ANY (all available interfaces)
                 - Requires pthread library for compilation
                 - Supports single client connection at a time
*/

   
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
            printf("\n[Connected with %s]\n<%s> ", server_name, client_name);
            fflush(stdout);
            continue;
        }



        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        printf("\n<%s> %s\n<%s> ", server_name, buffer, client_name);  
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



    printf("Enter your name: ");
    fflush(stdout);
    if (fgets(client_name, sizeof(client_name), stdin)) trim_eol(client_name);
    if (client_name[0] == '\0') strcpy(client_name, "Me");

    char hello[128];
    snprintf(hello, sizeof(hello), "NAME:%s\n", client_name);
    
    if (write(sockfd, hello, strlen(hello)) < 0) {
    perror("write NAME");
    }
    
    printf("Connected to %s on %s(%d)\n", server_name, argv[1], portno);
    printf("Connected to the Server! You can now chat. Type 'quit' to exit.\n");
    
    if (pthread_create(&read_thread, NULL, read_messages, &sockfd) != 0) {
        error("ERROR creating thread");
     }
     
     while(is_running){
        printf("<%s> ", client_name);
        fflush(stdout);
        
        if(fgets(buffer, 256, stdin) == NULL) break;
        
        if(strcmp(buffer, "quit\n") == 0){
            is_running = 0;
            break;
        }

        n = write(sockfd, buffer, strlen(buffer));
        if(n < 0) error("Error Writing to socket");

        }

    shutdown(sockfd, SHUT_RDWR);
     pthread_join(read_thread, NULL);
     close(sockfd);
     printf("Client disconnected\n");


     return 0;
   }




