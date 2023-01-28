#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <iostream>

#define buffer_size 2048
#define preamble 0xFEFF

struct newObject {
    long id;
    int x;
    int y;
    int type;
    int color[4];
};
/*Create cache to store each unique id and their correspond parameters.
  Assuming no more than 20 unique ids will be found for each run.
*/
struct newObject idCache[20];

/* Initializing TCP/IP connection to the server SAAB.jar with SEVERPORT = 5463.
   Example command line: ./FILENAME.out 5463 127.0.0.1
 */
int initClient(char * portNumber, char * ipAdress){
    int portno = atoi(portNumber);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server;
    struct sockaddr_in serv_addr;  
    if (sockfd < 0){
        fprintf(stderr,"error loading socket\n");
        exit(0);
    }
    server = gethostbyname(ipAdress);
    if (server == NULL){
        fprintf(stderr,"error loading server\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr,"error connecting to server\n");
        exit(0);
    }
    return sockfd;
}
//Green:  0x1b, 0x5B, 0x32, 0x6D
void codeGreen(int i){
    idCache[i].color[0]=0x1B;
    idCache[i].color[1]=0x5B;
    idCache[i].color[2]=0x32;
    idCache[i].color[3]=0x6D;
}
//Yellow: 0x1B, 0x5B, 0x33, 0x6D
void codeYellow(int i){
    idCache[i].color[0]=0x1B;
    idCache[i].color[1]=0x5B;
    idCache[i].color[2]=0x33;
    idCache[i].color[3]=0x6D;
}
//Red:    0x1B, 0x5B, 0x31, 0x6D
void codeRed(int i){
    idCache[i].color[0]=0x1B;
    idCache[i].color[1]=0x5B;
    idCache[i].color[2]=0x31;
    idCache[i].color[3]=0x6D;
}
/*Check distance from the reference (150,150) point
  Change the objects color array accordingly.*/
void categorizeObjects(){
    int i=0;
    while(idCache[i].id != 0){
        int distance = sqrt(pow((idCache[i].x-150),2)+pow((idCache[i].y-150),2));
        switch(idCache[i].type){
            /*Type 1 objects: Green unless closer than 75 from the designated coordinate
              then yellow and if closer than 50 then red.*/
            case 1:
                if(distance<50){
                    codeRed(i);
                }
                else if(distance<75 && distance > 50){
                    codeYellow(i);
                }
                else{
                    codeGreen(i); 
                }
                break;
            /*Type 2 objects: Green unless closer than 50 from the designated coordinate
              then yellow.*/
            case 2:
                if(distance<50){
                    codeYellow(i);
                }
                else{
                    codeGreen(i);
                }
                break;
            /*Category 2 objects: Yellow unless closer than 100 from the designated
              coordinate then red.*/
            case 3:
                if(distance<100){
                    codeRed(i);
                }
                else{
                    codeYellow(i);
                }
                break;
            default:
                fprintf(stderr, "Object type not in range (1-3)\n");
                idCache[i].color[0]=0;
                idCache[i].color[1]=0;
                idCache[i].color[2]=0;
                idCache[i].color[3]=0;
        }
        i++;
    }
}
long convertStrToLong(char * s){
    if (s == NULL || strlen(s) < 1){
        return 0;
    }
    long num;
    int i;
    char *ptr;
    ptr = strchr(s,'=');
    num = atol(ptr+1);
    return num;
}
int convertStrToInt(char * s){
    if (s == NULL || strlen(s) < 1){
        return 0;
    }
    int num;
    int i;
    char *ptr;
    ptr = strchr(s,'=');
    num = atoi(ptr+1);
    return num;
}

long long int decimalToBinary(int num) {
    long long int binNumber = 0;
    int power = 0;
    
    while (num > 0) {
        int rem = num % 2;
        long long int temp = pow(10, power);
        binNumber += rem * temp;
        power++;
        num /= 2;
    }
    
    return binNumber;
}
char ** splitStringSemicolon(char * s){
    char * tmp= (char*)malloc(strlen(s)*sizeof(char));
    char * token = (char*)malloc(strlen(s)*sizeof(char));
    int i=0;
    tmp = strdup(s);
    if (tmp == NULL){
        fprintf(stderr,"error loading string in splitString\n");
        free(tmp);
        free(token);
        return NULL;
    }
    char ** strArr = (char**)malloc(20*sizeof(char*));
    token = strtok_r(tmp, ";", &tmp);
    while(token != NULL){
            strArr[i] = strdup(token);
            token = strtok_r(tmp, ";", &tmp);
            i++;
    }
    free(tmp);
    free(token);
    strArr[i] = NULL;
    return strArr;
}

char ** splitStringNewline(char * s){
    char * tmp= (char*)malloc(strlen(s)*sizeof(char));
    char * token = (char*)malloc(40*sizeof(char));
    int i=0;
    tmp = strdup(s);
    if (tmp == NULL){
        free(tmp);
        free(token);
        return NULL;
    }
    char ** strArr = (char**)malloc(30*sizeof(char*));
    token = strtok_r(tmp, "\n", &tmp);
    while(token != NULL){
            strArr[i] = strdup(token);
            token = strtok_r(tmp, "\n", &tmp);
            i++;
    }
    free(tmp);
    free(token);
    strArr[i] = NULL;
    return strArr;
}
void identifyObjects(char * s, int * n){
    char ** objectStr = NULL;
    char ** parameterStr = NULL;
    int i=0;
    int j=0;
    objectStr = splitStringNewline(s);
    while(objectStr[j] != NULL){
        int k=0;
        long idCheck;
        int status=0;
        parameterStr = splitStringSemicolon(objectStr[j]);
        idCheck = convertStrToLong(parameterStr[0]);
        for(k=0; k<i; k++){
            if ( idCache[k].id == idCheck ){
                status=1;
                break;
            }
        }
        if (status == 1){
            idCache[k].x = convertStrToInt(parameterStr[1]);
            idCache[k].y = convertStrToInt(parameterStr[2]);
        }else{
            if (i == 20){
                fprintf(stderr,"idCache overflow\n");
                exit(0);
            }
            idCache[i].id = idCheck;
            idCache[i].x = convertStrToInt(parameterStr[0]);
            idCache[i].y = convertStrToInt(parameterStr[2]);
            idCache[i].type = convertStrToInt(parameterStr[3]);
            i++;
        }
        j++;
    }
    *n = i;
    free(objectStr);
    free(parameterStr);
}
int main(int argc, char *argv[]){
    if (argc < 3){
       fprintf(stderr,"Not enough arguments.\n");
       exit(0);
    }
    int sockfd = initClient(argv[1], argv[2]);
    int n;
    char buffer[buffer_size];
    for(int i=0; i<20; i++){
    idCache[i].id = 0;
    }   
    while(1){
        bzero(buffer, buffer_size);
        n = recv(sockfd, buffer, buffer_size, 0);
        int count=0;
        if (n < 0){
            fprintf(stderr,"Error reading server data.\n");
            exit(0);
        }
        identifyObjects(buffer, &count);
        categorizeObjects();
        printf("%x ",preamble);
        printf("%i\n",count);
        int i=0;
        while(idCache[i].id != 0){
            printf("%li %i %i %i ",idCache[i].id,idCache[i].x,idCache[i].y,idCache[i].type);
            for(int j=0; j<4; j++){
                printf("%x ",idCache[i].color[j]);
            }
            printf("\n");
            i++;
        }
        usleep(1500000);
    }
    return 0;
}
