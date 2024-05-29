

//Built with Direction and aid from Dr. Hempel at University of Nebraska-Lincoln


#include "../utils/ecen471871_addins.h" 
#include <netinet/ip_icmp.h>

#define MAX 1024
#define TIMER CLOCK_MONOTONIC


int main() 
{ 
    int client_socket_fd;
    struct sockaddr_in remote_sendto_addr, rx_from_addr; 
    int rx_from_addr_len, status, num_bytes_received; 
    unsigned short ping_index, ping_count;
    struct icmphdr icmp_req_hdr;    
    struct icmphdr icmp_rsp_hdr;
    struct timespec requesttime;
    struct timespec responsetime;
    struct timespec rtt,latency;    
    fd_set read_set;
    struct timeval timeout;
    char buff[MAX];
    int msg_len;
        
    // Set up the client's send-to address
    setup_sockaddr_in(&remote_sendto_addr, inet_addr("192.168.56.80"), 0);
    
    // Create socket
    client_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);   
    if (client_socket_fd < 0) exit_helper("Socket creation failed...",-1,-1);
   
    // Handle the ping flow 
    ping_count=6;
    ping_index=0;
    for (;ping_index<ping_count;++ping_index) {         
       
        // Get the timestamp from the MONOTONIC timekeeper
        status=clock_gettime(TIMER,&requesttime);
        if (status < 0) exit_helper("Getting current time failed...",client_socket_fd,-1);

        // Prepare the message buffer
        memset(&icmp_req_hdr, 0, sizeof(icmp_req_hdr));
        icmp_req_hdr.type = ICMP_ECHO;
        icmp_req_hdr.un.echo.id = 1234;//arbitrary id
        icmp_req_hdr.un.echo.sequence = ping_index;
        memcpy(buff, &icmp_req_hdr, sizeof(icmp_req_hdr));                           
        memcpy(buff + sizeof(icmp_req_hdr), "hello hello is anyone out there?", 32); //icmp payload
        msg_len=sizeof(icmp_req_hdr)+32;
        

        // Send that buffer to the destination
        printf("Sending ICMP ping request: %d\n",ping_index+1); 
        status=sendto(client_socket_fd, buff, msg_len, 0, (struct sockaddr*)&remote_sendto_addr, sizeof(remote_sendto_addr)); 
        if (status < 0) exit_helper("Sending ICMP request failed...",client_socket_fd,-1);

        // Wait for a response message
        timeout.tv_sec = 3;
        timeout.tv_usec = 0; //wait max 3 seconds for a reply
        memset(&read_set, 0, sizeof read_set);
        FD_SET(client_socket_fd, &read_set);
        status = select(client_socket_fd + 1, &read_set, NULL, NULL, &timeout);
        if (status < 0) exit_helper("Waiting for ICMP reply failed...\n", client_socket_fd,-1);
        else if (status == 0) {
            printf("Received no ICMP reply; Timeout\n");
            continue;
        }

        //Receive the response from the destination
        rx_from_addr_len=sizeof(rx_from_addr);
        num_bytes_received=recvfrom(client_socket_fd, &buff, sizeof(buff), 0, (struct sockaddr*)&rx_from_addr, &rx_from_addr_len);
        if (num_bytes_received < 0) exit_helper("Receiving message from ICMP ping handler failed...\n", client_socket_fd,-1);

        // Get the timestamp from the MONOTONIC timekeeper
        status=clock_gettime(TIMER,&responsetime);
        if (status < 0) exit_helper("Getting current time failed...",client_socket_fd,-1);

        // Perform message checking
        if (memcmp(&rx_from_addr,&remote_sendto_addr,sizeof(struct sockaddr_in))!=0) {
            printf("Received invalid message: wrong sender\n");
            continue;
        }
        if (num_bytes_received < sizeof(icmp_rsp_hdr)) {
            printf("Received invalid message: reply too short\n");
            continue;
        }
        memcpy(&icmp_rsp_hdr,buff,sizeof(icmp_rsp_hdr));
        if (icmp_rsp_hdr.type!=ICMP_ECHOREPLY) {
            printf("Received invalid message: wrong message type\n");
            continue;
        }
        if (icmp_rsp_hdr.un.echo.sequence!=ping_index) {
            printf("Received invalid message: wrong seqnum\n");
            continue;
        }

        printf("Received ICMP ping reply: \n"); 

        // Calculate RTT
        timespecsub(&responsetime,&requesttime,&rtt);
        printf("   start: %ld s; %9ld ns\n   stop:  %ld s; %9ld ns\n",requesttime.tv_sec,requesttime.tv_nsec,responsetime.tv_sec,responsetime.tv_nsec);
        
        // Get the latency from that
        timespechalf(&rtt,&latency);

        // Print the remote client's message         
        printf("   RTT:               %.3f ms\n",timespec2ms(&rtt));
        printf("   Latency:           %.3f ms\n",timespec2ms(&latency));        

        msleep(50);
    } 
}
