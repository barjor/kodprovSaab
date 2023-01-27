#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define buffer_size 512

struct newObject {
    long id;
    int x;
    int y;
    int type;
    int color[4];
};

int initClient(char * portNumber, char * ipAdress){
    int portno = atoi(portNumber);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server;
    struct sockaddr_in serv_addr;  
    if (sockfd < 0){
        printf("error loading socket\n");
        exit(0);
    }
    server = gethostbyname(ipAdress);
    if (server == NULL){
        printf("error loading server\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("error connecting to server\n");
    }
    return sockfd;
}

// input one object and check distance from the reference point
// change the objects color array accordingly.
//Red:    0x1B, 0x5B, 0x31, 0x6D
//Yellow: 0x1B, 0x5B, 0x33, 0x6D
//Green:  0x1b, 0x5B, 0x32, 0x6D
/*Category 2 objects: Yellow unless closer than 100 from the designated
coordinate then red.
• Type 1 objects: Green unless closer than 75 from the designated coordinate
then yellow and if closer than 50 then red.
• Type 2 objects: Green unless closer than 50 from the designated coordinate
then yellow.*/
void categorizeTarget(struct newObject * obj){
    int distance;
    int array[4];
    distance = sqrt(pow((obj->x-150),2)+pow((obj->y-150),2));
    switch(obj->type){
        case 1:
            if(distance<50){
                //red
                obj->color[0]=0x1B;
                obj->color[1]=0x5B;
                obj->color[2]=0x31;
                obj->color[3]=0x6D;
            }
            else if(distance<75 && distance > 50){
                //yellow
                obj->color[0]=0x1B;
                obj->color[1]=0x5B;
                obj->color[2]=0x33;
                obj->color[3]=0x6D;
            }
            else{
                //green 
                obj->color[0]=0x1b;
                obj->color[1]=0x5B;
                obj->color[2]=0x32;
                obj->color[3]=0x6D;   
            }
            break;
        case 2:
            if(distance<50){
                //yellow
                obj->color[0]=0x1B;
                obj->color[1]=0x5B;
                obj->color[2]=0x33;
                obj->color[3]=0x6D;
            }
            else{
                //green 
                obj->color[0]=0x1b;
                obj->color[1]=0x5B;
                obj->color[2]=0x32;
                obj->color[3]=0x6D;  
            }
            break;
        case 3:
            if(distance<100){
                //red
                obj->color[0]=0x1B;
                obj->color[1]=0x5B;
                obj->color[2]=0x31;
                obj->color[3]=0x6D;
            }
            else{
                //yellow
                obj->color[0]=0x1B;
                obj->color[1]=0x5B;
                obj->color[2]=0x33;
                obj->color[3]=0x6D;
            }
            break;
        default:
            printf("Incorrect type\n");
    }
}

char ** splitStringNewline(char * s){
    char * tmp= (char*)malloc(strlen(s)*sizeof(char));
    char * token = (char*)malloc(strlen(s)*sizeof(char));
    int i=0;
    tmp = strdup(s);
    if (tmp == NULL){
        printf("error loading string in splitString\n");
        free(tmp);
        free(token);
        return 0;
    }
    char ** strArr = (char**)malloc(20*sizeof(char*));
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

char ** splitStringSemicolon(char * s){
    char * tmp= (char*)malloc(strlen(s)*sizeof(char));
    char * token = (char*)malloc(strlen(s)*sizeof(char));
    int i=0;
    tmp = strdup(s);
    if (tmp == NULL){
        printf("error loading string in splitString\n");
        free(tmp);
        free(token);
        return 0;
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

long convertStrToLong(char * s){
    if (s == NULL || strlen(s) < 1){
        printf("Wrong size on input\n");
        return 0;
    }
    long num;
    int i;
    char *ptr;
    ptr = strchr(s,'=');
    ptr++;
    num = atol(ptr);
    return num;
}
int convertStrToInt(char * s){
    if (s == NULL || strlen(s) < 1){
        printf("Wrong size on input\n");
        return 0;
    }
    int num;
    int i;
    char *ptr;
    ptr = strchr(s,'=');
    ptr++;
    num = atoi(ptr);
    return num;
}

struct newObject * createObject(char ** s){
    int size=0;
    while(s[size] != NULL){
        size++;
    }
    if( size != 4){
        printf("Error creating struct, wrong dimensions of input\n");
        return NULL;
    }
    struct newObject *object;
    object = (struct newObject*)malloc(sizeof(struct newObject));
    object->id = convertStrToLong(s[0]);
    object->x = convertStrToInt(s[1]);
    object->y = convertStrToInt(s[2]);
    object->type = convertStrToInt(s[3]);
    return object;
}

int countObjects(char *s){
    if (s == NULL || strlen(s) < 1){
        return 0;
    }
    int count=0;
    int i=0;
    while(s[i] != '\0'){
        if(s[i] == '\n'){
            count++;
        }
        i++;
    }
    return count;
}


struct newObject ** identifyObjects(char * s, int * count){
    int nrObjects = 0;
    nrObjects = countObjects(s);
    *count = nrObjects;

    if (nrObjects == 0){
        printf("No objects detected\n");
        return NULL;
    }

    struct newObject **objectStruct;
    objectStruct = (struct newObject **)malloc((nrObjects+1)*sizeof(struct newObject *));
    char ** objectStr = NULL;
    char ** parameterStr = NULL;
    int i=0;

    objectStr = splitStringNewline(s);//Fill objectStr with substrings divided by newline character.

    while(objectStr[i] != NULL){
        parameterStr = splitStringSemicolon(objectStr[i]);//Divide by semicolon
        objectStruct[i] = createObject(parameterStr);
        i++;
    }
    free(parameterStr);
    free(objectStr);
    return objectStruct;
}

int main(int argc, char *argv[]){
    if (argc < 3){
       printf("not enough arguments\n");
       exit(0);
    }
    int sockfd = initClient(argv[1], argv[2]);
    int n;
    char buffer[buffer_size];
    
    while(1){
        bzero(buffer, buffer_size);
        n = recv(sockfd, buffer, buffer_size, 0);
        int count=0;
        if (n < 0){
            printf("Error reading server data\n");
            exit(0);
        }
        struct newObject **objectStruct;
        objectStruct = identifyObjects(buffer, &count);

        if (objectStruct[0] != NULL){
            for(int i=0; i<count; i++){
                categorizeTarget(objectStruct[i]);
            }
        }
        

        free(objectStruct);
    }
    
    return 0;
}
