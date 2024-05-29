

//Built with Direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 
#include "ping_msg.h"

#define PORT 8080 
#define TIMER CLOCK_MONOTONIC
   
int main() 
{ 
    int client_socket_fd;
    struct sockaddr_in local_bind_addr, remote_sendto_addr, rx_from_addr; 
    int rx_from_addr_len, status, num_bytes_received; 
    unsigned short ping_index, ping_count;
    struct timespec requesttime;
    struct timespec responsetime;
    struct timespec servertime_from_local;
    struct timespec servertime_from_server;
    struct timespec servertimeoffset;
    struct timespec rtt,latency;    
    struct ping_message msg;
    fd_set read_set;
    struct timeval timeout;
        
    // Set up the client's local bind-to address
    setup_sockaddr_in(&local_bind_addr, inet_addr("192.168.56.40"), ntohs(PORT));
    setup_sockaddr_in(&remote_sendto_addr, inet_addr("192.168.56.80"), htons(PORT));
    // Create socket
   client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (client_socket_fd < 0) exit_helper("Socket creation failed...",-1,-1);
    // Bind the client's socket to a specific interface and port number
        status = bind(client_socket_fd, (struct sockaddr*)&local_bind_addr, sizeof(local_bind_addr));  
    if (status < 0) exit_helper("Socket bind failed...",client_socket_fd,-1);
    // Handle the ping flow 
    ping_count=10;
    ping_index=0;
    for (;ping_index<ping_count;++ping_index) {         
    
        // Get the timestamp from the MONOTONIC timekeeper
        status=clock_gettime(TIMER,&requesttime);
        if (status < 0) exit_helper("Getting current time failed...",client_socket_fd,-1);

        // Prepare the message buffer
        msg.opcode=UDPPING_OPCODE_REQUEST;
        msg.seqnum=ping_index;
        msg.client_sec=requesttime.tv_sec;
        msg.client_nsec=requesttime.tv_nsec;
        msg.server_sec=0;
        msg.server_nsec=0;
        


        // Send that buffer to the udpping server
        printf("Sending udpping request: %d\n",ping_index+1); 
       
        num_bytes_received = sendto(client_socket_fd, (struct ping_message*)&msg, sizeof(msg), 0, (struct sockaddr*)&remote_sendto_addr, sizeof(remote_sendto_addr));
        if (num_bytes_received < 0) exit_helper("Sending message to remote client failed...",client_socket_fd,-1);
        
        // Wait for a response message
        timeout.tv_sec = 3;
        timeout.tv_usec = 0; //wait max 3 seconds for a reply
        memset(&read_set, 0, sizeof read_set);
        FD_SET(client_socket_fd, &read_set);
        status = select(client_socket_fd + 1, &read_set, NULL, NULL, &timeout);
        if (status < 0) exit_helper("Waiting for response message from udpping server failed...\n", client_socket_fd,-1);
        else if (status == 0) {
            printf("Received no response; Timeout\n");
            continue;
        }

        //Receive the response from the udpping server
    
        rx_from_addr_len = sizeof(rx_from_addr); 

        num_bytes_received = recvfrom(client_socket_fd,(struct ping_message*)&msg , sizeof(msg), 0, (struct sockaddr*)&rx_from_addr, &rx_from_addr_len);
        if (num_bytes_received < 0) exit_helper("Receiving message from remote client failed...", client_socket_fd,-1);

        // Get the timestamp from the MONOTONIC timekeeper
        status=clock_gettime(TIMER,&responsetime);
        if (status < 0) exit_helper("Getting current time failed...",client_socket_fd,-1);

        // Perform message checking
        if (memcmp(&rx_from_addr,&remote_sendto_addr,sizeof(struct sockaddr_in))!=0) {
            printf("Received invalid message: wrong sender\n");
            continue;
        }
        if (num_bytes_received != sizeof(struct ping_message)) {
            printf("Received invalid message: wrong size\n");
            continue;
        }
        if (msg.opcode!= UDPPING_OPCODE_RESPONSE) {
            printf("Received invalid message: wrong opcode\n");
            continue;
        }
        if (msg.seqnum!=ping_index) {
            printf("Received invalid message: wrong seqnum\n");
            continue;
        }

        printf("Received udpping response: \n"); 

        // Calculate RTT
        timespecsub(&responsetime,&requesttime,&rtt);
        printf("   start: %ld s; %9ld ns\n   stop:  %ld s; %9ld ns\n",requesttime.tv_sec,requesttime.tv_nsec,responsetime.tv_sec,responsetime.tv_nsec);
        
        // Get the latency from that
        timespechalf(&rtt,&latency);

        // Apply the latency to get the server's expected RX time of the request
        timespecadd(&requesttime,&latency,&servertime_from_local);

        // Get the server's clock value from the response message
        servertime_from_server.tv_sec=msg.server_sec;
        servertime_from_server.tv_nsec=msg.server_nsec;

        // Get the client-server clock offset
        timespecsub(&servertime_from_server,&servertime_from_local,&servertimeoffset);

        // Print the remote client's message         
        printf("   RTT:               %.3f ms\n",timespec2ms(&rtt));
        printf("   Latency:           %.3f ms\n",timespec2ms(&latency));
        printf("   Clock Offset:      %.3f ms\n",timespec2ms(&servertimeoffset));

        msleep(50);
    } 
}
