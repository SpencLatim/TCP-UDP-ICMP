

//Built with Direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 

#define MAX 1024
#define PORT 8080 
   
int main() 
{ 
  
    int server_listener_fd, client_connection_fd;    
    struct sockaddr_in server_bind_addr, remote_client_addr; 
    int remote_client_addr_len;
    int status; 
    char buff[MAX]; 
    int buff_msg_len;
       
    // Set up the server's local bind-to address
    setup_sockaddr_in(&server_bind_addr, inet_addr("192.168.56.80"), ntohs(PORT));
    
    // Create socket
    server_listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_listener_fd < 0) exit_helper("Socket creation failed...",-1,-1);
   
    // Bind server's listening socket to a specific interface and port number
    status = bind(server_listener_fd, (struct sockaddr*)&server_bind_addr, sizeof(server_bind_addr));
    if (status < 0) exit_helper("Socket bind failed...",server_listener_fd,-1);
   
    // Listen on the server socket
    status = listen(server_listener_fd, 5); 
    if (status < 0) exit_helper("Server listen failed...",server_listener_fd,-1);
    else printf("Server listening for client connections...\n");
       
    // Accept the client connection
    remote_client_addr_len = sizeof(remote_client_addr); 
    client_connection_fd = accept(server_listener_fd, (struct sockaddr*)&remote_client_addr, &remote_client_addr_len); 
    if (client_connection_fd < 0) exit_helper("Client connection accept failed...",server_listener_fd,-1);
   
    // Handle the chat flow 
    for (;;) { 
       
        // Get ready for receiving the message from the client
        printf("Waiting for client message...\n"); 
        bzero(buff, MAX); 
        
        // Receive the message from the client 
        status = read(client_connection_fd, buff, sizeof(buff)); 
        if (status == 0) exit_helper("Client closed connection...",server_listener_fd,client_connection_fd);
        else if (status < 0) exit_helper("Receiving message from client failed...", server_listener_fd,client_connection_fd);
        
        // Print the client message 
        printf("Received client message: %s\n", buff); 
        
        // If msg contains "Exit" then end chat and close server listening socket 
        if (strncmp("exit", buff, 4) == 0) exit_helper("Server Exit...",server_listener_fd,client_connection_fd); 

        // Get the server text message
        buff_msg_len = string_entry_prompt("Enter server message (enter 'exit' to close connection):", buff, MAX);
        if (buff_msg_len==0) exit_helper("Server Exit...",server_listener_fd,client_connection_fd); 
        
        // Print the message to send
        printf("Sending message to server: %s\n", buff); 
                
        // Send that buffer to the client 
        status = write(client_connection_fd, buff, buff_msg_len);
        if (status < 0) exit_helper("Sending message to client failed...",server_listener_fd,client_connection_fd);
        
        // If msg contains "Exit" then end chat and close server listening socket 
        if (strncmp("exit", buff, 4) == 0) exit_helper("Server Exit...",server_listener_fd,client_connection_fd); 
    } 
}
