#ifndef __ECEN_471871_ADDINS__
#define __ECEN_471871_ADDINS__

//NOTE: This helps Intellisense in VSCode find the symbols in netdb.h
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif // _DEFAULT_SOURCE

#include <arpa/inet.h> // inet_addr()
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include <errno.h>
#include <time.h>  //for timestamps and time measurements
#include <ifaddrs.h>
#include <net/if.h>


int string_entry_prompt(char * prompt, char* buff, int nmax) {
    int n=0;
    printf("%s",prompt);
    bzero(buff, nmax); 
    while ((buff[n++] = getchar()) != '\n' && n<(nmax-1));
    if (n>1 && buff[n-1]=='\n') buff[n-1]=0;
    return n-1;
}
void exit_helper(char * msg, int sock1, int sock2) {
    printf("%s",msg);
    if (errno!=0) printf("most recent error: %s\n",strerror(errno));else printf("\n");
    if (sock1<0) close(sock1);
    if (sock2<0) close(sock2);
    exit(-1);
}

void setup_sockaddr_in(struct sockaddr_in* sa, in_addr_t addr, in_port_t port) {
    bzero(sa, sizeof(struct sockaddr_in)); 
    sa->sin_family = AF_INET; 
    sa->sin_addr.s_addr = addr;
    sa->sin_port = port;
}
void sockaddr_to_string(const struct sockaddr_in* ep, char * buff, int nmax) {
    char name[100];
    char port[10];
    getnameinfo((const struct sockaddr*)ep, sizeof(struct sockaddr_in), name, sizeof(name), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
    if (strlen(name)+1+strlen(port)>=nmax) return;
    sprintf(buff, "%s:%s", name, port);
}

void adapter_from_ip(const char * ip, char* ifname, int nmax) {
    struct ifaddrs *addrs, *iap;
    struct sockaddr_in *sa;
    char buf[32];

    getifaddrs(&addrs);
    for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
        if (iap->ifa_addr && (iap->ifa_flags & IFF_UP) && iap->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *)(iap->ifa_addr);
            inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), buf, sizeof(buf));
            if (!strcmp(ip, buf)) {
                strncpy(ifname,iap->ifa_name,nmax);
                freeifaddrs(addrs);
                return;
            }
        }
    }
    if (nmax>0) ifname[0]=0;
    freeifaddrs(addrs);
}

void timespecnorm(struct timespec* time1) {
    time1->tv_sec=time1->tv_sec;
    time1->tv_nsec=time1->tv_nsec;

    while (time1->tv_nsec >=1000000000L) {
        time1->tv_sec++;
        time1->tv_nsec -= 1000000000L;
    }
    while (time1->tv_nsec <=-1000000000L) {
        time1->tv_sec--;
        time1->tv_nsec += 1000000000L;
    }

    //resolve the case of opposite signs (except for 0 values)        
    while (time1->tv_sec>0 && time1->tv_nsec <0) {
        time1->tv_sec--;
        time1->tv_nsec += 1000000000L;
    }
    while (time1->tv_sec<0 && time1->tv_nsec >0) {
        time1->tv_sec++;
        time1->tv_nsec -= 1000000000L;
    }
}
char timespeccmp(const struct timespec* time1, const struct timespec* time2) {
    if (time1->tv_sec>time2->tv_sec) return 1;
    else if (time1->tv_sec<time2->tv_sec) return -1;        
    else {
        if (time1->tv_nsec>time2->tv_nsec) return 1;
        else if (time1->tv_nsec==time2->tv_nsec) return 0;
        else return -1;
    }
}
void timespecsub(const struct timespec* time1, const struct timespec* time2, struct timespec* tsres) {
    (tsres)->tv_sec = (time1)->tv_sec - (time2)->tv_sec;
    (tsres)->tv_nsec = (time1)->tv_nsec - (time2)->tv_nsec;
    timespecnorm(tsres);    
}
void timespeccopy(const struct timespec* time1, struct timespec* tsres) {
    (tsres)->tv_sec = (time1)->tv_sec;
    (tsres)->tv_nsec = (time1)->tv_nsec;
    timespecnorm(tsres);    
}

void timespecadd(const struct timespec* time1, const struct timespec* time2, struct timespec* tsres) {
    (tsres)->tv_sec = (time1)->tv_sec + (time2)->tv_sec;
    (tsres)->tv_nsec = (time1)->tv_nsec + (time2)->tv_nsec;
    timespecnorm(tsres);    
}

void timespechalf(const struct timespec* time1, struct timespec* tsres) {
    (tsres)->tv_sec = (time1)->tv_sec/2;
    (tsres)->tv_nsec = (time1)->tv_nsec/2;
    timespecnorm(tsres);    
}
float timespec2ms(const struct timespec* time1) {
    struct timespec timetmp;
    timetmp.tv_sec=time1->tv_sec;
    timetmp.tv_nsec=time1->tv_nsec;
    timespecnorm(&timetmp);
    if (timetmp.tv_sec>=0 && timetmp.tv_nsec>=0) return (float)(timetmp.tv_sec*1000)+((float)(timetmp.tv_nsec)/1000000.0f);
    else return -(float)(-timetmp.tv_sec*1000)+((float)(-timetmp.tv_nsec)/1000000.0f);
}
float timespec2s(const struct timespec* time1) {
    struct timespec timetmp;
    timetmp.tv_sec=time1->tv_sec;
    timetmp.tv_nsec=time1->tv_nsec;
    timespecnorm(&timetmp);
    if (timetmp.tv_sec>=0 && timetmp.tv_nsec>=0) return (float)(timetmp.tv_sec)+((float)(timetmp.tv_nsec)/1000000000.0f);
    else return -(float)(-timetmp.tv_sec)+((float)(-timetmp.tv_nsec)/1000000000.0f);
}

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}























#endif //__ECEN_471871_ADDINS__