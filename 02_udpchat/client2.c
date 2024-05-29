

//Built with Direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 

#define MAX 1024
#define PORT 8080 
   
int main() 
{ 

    int client_socket_fd;
    struct sockaddr_in local_bind_addr, remote_sendto_addr, rx_from_addr; 
    int rx_from_addr_len, status; 
    char buff[MAX], remote_ep_label[100]; 
    int buff_msg_len; 
   
    // Set up the client's local bind-to address
       setup_sockaddr_in(&local_bind_addr, inet_addr("192.168.56.80"), ntohs(PORT));
    setup_sockaddr_in(&remote_sendto_addr, inet_addr("192.168.56.40"), htons(PORT));

    // Create socket
     client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);  
    if (client_socket_fd < 0) exit_helper("Socket creation failed...",-1,-1);
   
    // Bind the client's socket to a specific interface and port number 
     status = bind(client_socket_fd, (struct sockaddr*)&local_bind_addr, sizeof(local_bind_addr));
    if (status < 0) exit_helper("Socket bind failed...",client_socket_fd,-1);
    

    for (;;) { 
        // Get ready for receiving the message from the remote client
        printf("Waiting for message...\n"); 
        bzero(buff, MAX);

        // Receive the message from the remote side and copy it into the buffer 
        rx_from_addr_len=sizeof(rx_from_addr);
         
             status = recvfrom(client_socket_fd, buff, sizeof(buff), 0, (struct sockaddr*)&rx_from_addr, &rx_from_addr_len );
        if (status < 0) exit_helper("Receiving message from remote client failed...", client_socket_fd,-1);
        
        // Print the remote client's message 
        sockaddr_to_string(&rx_from_addr,remote_ep_label,sizeof(remote_ep_label));
        printf("Received message: %s from remote client at %s\n", buff, remote_ep_label); 
        
        // If msg contains "exit" then end chat 
        if (strncmp("exit", buff, 4) == 0) exit_helper("Client Exit...",client_socket_fd,-1); 

        // Get a text message from the user to send to the other client
        buff_msg_len = string_entry_prompt("Enter message for remote client (enter 'exit' to close connection):", buff, MAX);
        if (buff_msg_len==0) exit_helper("Client Exit...",client_socket_fd,-1); 
        
        // Print the message to send
        printf("Sending message: %s\n", buff); 
        
             status = sendto(client_socket_fd, buff, buff_msg_len, 0, (struct sockaddr*)&remote_sendto_addr, sizeof(remote_sendto_addr));
        if (status < 0) exit_helper("Sending message to remote client failed...",client_socket_fd,-1);
        
        // If msg contains "exit" then end chat
        if (strncmp("exit", buff, 4) == 0) exit_helper("Client Exit...",client_socket_fd,-1); 
    } 
}
