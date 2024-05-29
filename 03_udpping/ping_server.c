

//Built with Direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 
#include "ping_msg.h"

#define PORT 8080 
#define TIMER CLOCK_MONOTONIC
   
int main() 
{ 
    
    int server_socket_fd;
    struct sockaddr_in local_bind_addr, rx_from_addr; 
    int rx_from_addr_len, status, num_bytes_received; 
    struct timespec currtime;
    struct ping_message msg;

    // Set up the server's local bind-to address
    
    setup_sockaddr_in(&local_bind_addr, inet_addr("192.168.56.80"), ntohs(PORT));
    // Create socket
    
    server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket_fd < 0) exit_helper("Socket creation failed...",-1,-1);
    // Bind the client's socket to a specific interface and port number
    
    status = bind(server_socket_fd, (struct sockaddr*)&local_bind_addr, sizeof(local_bind_addr));  
    if (status < 0) exit_helper("Socket bind failed...",server_socket_fd,-1);
    // Indicate that the server is ready
    printf("UDPPing Server is ready for requests...\n");

    // Handle the ping flow 
    for (;;) {
      

        // Receive the request from a udpping client
     
             rx_from_addr_len = sizeof(rx_from_addr);

          num_bytes_received = recvfrom(server_socket_fd, (struct ping_message*)&msg, sizeof(msg), 0, (struct sockaddr*)&rx_from_addr, &rx_from_addr_len );
        if (num_bytes_received < 0) exit_helper("Receiving message from remote client failed...", server_socket_fd,-1);
        // Get the timestamp from the MONOTONIC timekeeper
        status=clock_gettime(TIMER,&currtime);
        if (status < 0) exit_helper("Getting current time failed...",server_socket_fd,-1);


        // Perform message checking
        if (num_bytes_received != sizeof(struct ping_message)) {
            printf("Received invalid message: wrong size\n");
            continue;
        }
        if (msg.opcode!=UDPPING_OPCODE_REQUEST) {
            printf("Received invalid message: wrong opcode\n");
            continue;
        }

        
        // Prepare the response message
        
        msg.opcode=  UDPPING_OPCODE_RESPONSE;
        msg.server_sec=currtime.tv_sec;
        msg.server_nsec=currtime.tv_nsec;
        // Send the response message
        printf("Received udpping request; Sending udpping response\n"); 
        
          num_bytes_received = sendto(server_socket_fd, (struct ping_message*)&msg, sizeof(msg), 0, (struct sockaddr*)&rx_from_addr, sizeof(rx_from_addr));
          if (num_bytes_received < 0) exit_helper("Sending message to remote client failed...",server_socket_fd,-1);

    }
}
    