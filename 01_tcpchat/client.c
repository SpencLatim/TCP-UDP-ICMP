

//Built with direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 

#define MAX 1024
#define PORT 8080 

int main()
{
    
    int server_connection_fd;   
    struct sockaddr_in client_bind_addr, remote_server_addr;
    int status;
    char buff[MAX];
    int buff_msg_len;
 
    //Set up the remote server's address
    setup_sockaddr_in(&remote_server_addr, inet_addr("192.168.56.80"), htons(PORT));
    
    // Set up the client's local bind-to address
    setup_sockaddr_in(&client_bind_addr, inet_addr("192.168.56.40"), 0);
    
    // Create socket
    server_connection_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_connection_fd < 0) exit_helper("Socket creation failed...",-1,-1);
    
    // Bind client socket to a specific interface
    status = bind(server_connection_fd, (struct sockaddr*)&client_bind_addr, sizeof(client_bind_addr));
    if (status < 0) exit_helper("Socket bind failed...",server_connection_fd,-1);

    // Connect the client socket to the server's listening socket
    status = connect(server_connection_fd, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr));
    if (status < 0) exit_helper("Connecting to the server failed...",server_connection_fd,-1);
 
    // Handle the chat flow
    for (;;) { 
        // Get the client text message
        buff_msg_len=string_entry_prompt("Enter client message (enter 'exit' to close connection):", buff, MAX);
        if (buff_msg_len == 0) exit_helper("Client Exit...",server_connection_fd,-1); 
   
        // Print the message to send
        printf("Sending message to client: %s\n", buff); 
                       
        // Send that buffer to the server 
        status = write(server_connection_fd, buff, buff_msg_len);
        if (status < 0) exit_helper("Sending message to server failed...",server_connection_fd,-1);

        // If msg contains "exit" then close client connection and end chat. 
        if (strncmp("exit", buff, 4) == 0) exit_helper("Client Exit...",server_connection_fd,-1); 

        // Get ready for receiving the message from the server
        printf("Waiting for server message...\n"); 
        bzero(buff, sizeof(buff));
        
        // Receive the message from the server 
        status = read(server_connection_fd, buff, buff_msg_len);
        if (status == 0) exit_helper("Server closed connection...",server_connection_fd,-1);
        else if (status < 0) exit_helper("Receiving message from server failed...",server_connection_fd,-1);

        // Print the server message 
        printf("Received server message: %s\n", buff); 
        
        // If msg contains "exit" then close client connection and end chat. 
        if (strncmp("exit", buff, 4) == 0) exit_helper("Client Exit...",server_connection_fd,-1); 
    } 
}
