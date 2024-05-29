#ifndef __UDPPING_MSG__
#define __UDPPING_MSG__

struct __attribute__ ((packed)) ping_message {
    unsigned char opcode;   //we will use 0 for request, 1 for response
    unsigned short seqnum;   
    long int client_sec;
    long int client_nsec;
    long int server_sec;
    long int server_nsec;
};
#define UDPPING_OPCODE_REQUEST 0
#define UDPPING_OPCODE_RESPONSE 1

#endif //__UDPPING_MSG__