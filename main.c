#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <arpa/inet.h>
#include <zconf.h>

#define MAXDATASIZE 8096 //Max amount of data we can recieve at once
#define sizeSendFE 8
#define httpCodeSize 3
#define httpVersionSize 8
#define sizeFES 32
#define sizeFSS 16 //may need to truncate size down, doesnt seem
#define timeBufferSize 32

//Added to see what git does

char Port[7];
char DocRoot[256];

void *get_in_addr(struct sockaddr *sa) {// IPV4 or IPV6;
    if (sa->sa_family == AF_INET){
        return & (((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);

}
int main(int argc, char *argv[]) {

    char addressChar[INET6_ADDRSTRLEN];

    ///////////////
    // Store Arguments
    printf("Retrieving command line arguments.\n");
    if(argc==1){//no aruguments
        printf("No arguments passed.\n");
    }
    if(argc >= 2){
        printf("Arguments are as follows.\n");
        for (short i = 1;i < argc; ++i ){//increments through each command
            printf("\nargv %d: %s", i, argv[i]);
            if (i == 1){//This should be DocRoot, all together this is a bit lazy and assumes the person will enter commands in the right order
                strncpy(DocRoot, argv[i], sizeof(DocRoot));
            }
            if (i == 2){//This should be port, all together this is a bit lazy and assumes the user will enter commands in the right order
                int checkArg = atoi(argv[2]);
                if (checkArg > 65535){//Make sure we have a valid port number
                    printf("\nError, port number %d to large.\n", checkArg);
                }
                else if (checkArg < 0){
                    printf("\nError, invalid port number %d.\n", checkArg);
                }
                else if (checkArg < 1024){
                    printf("\nWarning, port number %d may be reserved.\n", checkArg);
                }
                strncpy(Port, argv[i], sizeof(Port));
            }
        }
        printf("\n");
    }

    printf("DocRoot is %s\n", DocRoot);//Lazy Check
    printf("Port is %s\n", Port);//Lazy Check

    ///////////////
    //Open Port
    struct addrinfo hints;
    struct addrinfo * address_resource;

    struct sockaddr_storage income_addr;//connecters address

    memset(&hints, 0, sizeof(hints)); // make sure the struct is clear
    hints.ai_family = AF_UNSPEC; // Any protocol the system supports
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;// Listening

    /////
    //Info for Default Ip (string), Info for port number (string), hints(struct addrinfo), status(struct *addrinfo)
    int ainfo_desc = getaddrinfo(NULL, Port, &hints, &address_resource);//Port Used to be constant, might not like being changed

    /////
    //Allocate a socket for our program
    int socket_desc = socket(address_resource->ai_family, address_resource->ai_socktype, address_resource->ai_protocol);

    /////
    //socket descriptor, magic...
    short enable = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {//returns negative value if unsucsessful
        printf("Error: setsockopt failed\n");
    }
    if(bind(socket_desc, address_resource->ai_addr, address_resource->ai_addrlen) < 0){//returns negative value if unsucsessful
        printf("Error: socket bind encounter issue\n");
    }
    if(listen(socket_desc, 5) < 0){// Number describes how many incomming connections we will queue
        printf("Error: listen\n");
    }

    socklen_t income_size;
    int new_connect;

    while(1){
        income_size = sizeof income_addr;
        new_connect = accept(socket_desc, (struct sockaddr *)&income_addr, &income_size);
        if (new_connect == -1){
            continue;
        }

        inet_ntop(income_addr.ss_family,get_in_addr((struct sockaddr *) &income_addr), addressChar, sizeof addressChar);
        printf("Server: Connection recieved from %s.\n", addressChar);
        if(!fork()){//child
            close(socket_desc);
            if(/*send(new_connect, "Connection Established\n",23,0)*/ send(new_connect, "", 0, 0) == -1){
                printf("Error: Sending message\n");
                close(new_connect);// Stuff we do at the end of the connection
                exit(0);// Stuff we do at the end of the connection
            }

            else{//Connection is sucsesfully established
                char recievedText[MAXDATASIZE];//Max amound of bytes we can get at once
                char inText;
                int loop = 1;
                int numBytes = 0;
                
                if((numBytes = recv(new_connect, recievedText, MAXDATASIZE-1, 0))==-1){//This guy is working
                    printf("Server: Error recieving text");
                }
                recievedText[numBytes] = '\0';
                printf("Recieved:\n%s\n", recievedText);
                //////////
                // Parse
                // We can assume first line is a command, we must verify it
                // We should probably store the remaining lines
                // We should check the remaining lines for errors
                char parsedRecieved [MAXDATASIZE/16][numBytes];//Create a 2d char array to store our strings, should make this more scalable and less huge
                for (int i = 0; i < MAXDATASIZE/16; ++i){
                    for (int j = 0; j < numBytes; ++j){
                        parsedRecieved[i][j] = '\0';
                    }
                }
                char * tempString;
                tempString = strtok(recievedText, " \r\n");
                short i = 0;
                while (tempString != NULL){
                    strcpy(parsedRecieved[i], tempString);
                    tempString = strtok(NULL, " \r\n");
                    ++i;
                }
                //Command will be parsedRecieved[0], File will be parsedRecieved[1]
                char filePath[sizeof(DocRoot) + sizeof(parsedRecieved[1])];
                strcpy(filePath,DocRoot);
                strcat(filePath,parsedRecieved[1]);
                //////////
                //Processing Recieved Commands
                //Change current errors to changing an ascii value that is the error code
                short sendContents = 0;
                short isHead = 0;
                //short httpCodeSize = 3;
                char httpCode[httpCodeSize];
                //short httpVersionSize = 8;
                char httpVersion[httpVersionSize];

                strcpy(httpCode, "200");//Carefull with this string
                strcpy(httpVersion, "HTTP/1.1");

                if (strcmp(parsedRecieved[2], "HTTP/1.1") != 0){
                    //printf("Error: %s is requesting an unsported http version\n", addressChar);
                    strcpy(httpCode,"400");
                }
                else if(strcmp(parsedRecieved[0], "GET") == 0){//Client has issued get command
                   // printf("Client %s issued GET command\n", addressChar);
                    sendContents = 1;
                }
                else if(strcmp(parsedRecieved[0], "HEAD") == 0){
                   // printf("Client %s issued HEAD command\n", addressChar);
                   isHead = 1;
                }
                else{
                    strcpy(httpCode, "405");
                    //printf("Client %s has issued invalid command %s\n", addressChar, parsedRecieved[0]);
                }
                FILE * tempFile;
                tempFile = fopen(filePath, "r");//should be a+ for debugging, r for actual
                struct stat st;
                stat(filePath, &st);
                if(tempFile == NULL){
                    strcpy(httpCode, "404");
                    //printf("%s does not exist\n", filePath);
                }

                char * fileExtention;
                //short sizeSendFE = 8;
                char sendFE[sizeSendFE];
                //short sizeFES = 32;
                char fileExtentionString[sizeFES];
                for (short i = 0; i < sizeFES; ++i){
                    fileExtentionString[i] = ' ';
                }
                for (short i = 0; i < sizeSendFE; ++i){
                    sendFE[i] = ' ';
                } 
                fileExtention = strtok(filePath, ".");
                fileExtention = strtok(NULL, ".");
                strcpy(sendFE, fileExtention);

                if (strcmp(httpCode, "200") != 0){
                    strcpy(fileExtentionString,"text/html");
                }
                else if (strncmp(sendFE, "txt", 3) == 0){
                    strcpy(fileExtentionString, "text/plain");
                }
                else if (strncmp(sendFE, "html", 4) == 0){
                    strcpy(fileExtentionString,"text/html"); 
                }
                else if (strncmp(sendFE, "css", 3) == 0){
                    strcpy(fileExtentionString, "text/css");
                }
                else if (strncmp(sendFE, "jpeg", 4) == 0){
                    strcpy(fileExtentionString,"image/jpeg");
                }
                else if (strncmp(sendFE, "png", 3) == 0){
                    strcpy(fileExtentionString, "image/png");
                }
                else if (strncmp(sendFE, "gif", 3) == 0 ){
                    strcpy(fileExtentionString,"image/gif");
                }
                else if (strncmp(sendFE, "pdf", 3) == 0){
                    strcpy(fileExtentionString, "application/pdf"); 
                }
                else if (strncmp(sendFE, "js", 2) == 0){
                    strcpy(fileExtentionString,"application/javascript");
                }
                else {
                    strcpy(httpCode, "415");
                    strcpy(fileExtentionString, "text/html");
                    sendContents = 0;
                }
                

                //////////
                // Respond to request
                /////
                //Status Line
                send(new_connect, httpVersion, httpVersionSize, 0);
                send(new_connect, " ", 1, 0);
                send(new_connect, httpCode, httpCodeSize, 0);
                send(new_connect, " ", 1, 0);
                if (strcmp(httpCode, "200") == 0){
                    send(new_connect, "OK", 2, 0);
                } 
                else if (strcmp(httpCode, "400") == 0){
                    send(new_connect, "Bad Request", 11, 0); 
                }
                else if (strcmp(httpCode, "404") == 0){
                    send(new_connect, "Not Found", 9, 0); 
                }
                else if (strcmp(httpCode, "405") == 0){
                    send(new_connect, "Method Not Allowed", 18, 0);
                }
                else if (strcmp(httpCode, "415") == 0){
                    send(new_connect, "Unsupported Media Type", 22, 0);
                }
                send(new_connect, "\r\n", 2, 0);
                /////
                //Header Lines
                //Include content type, content length, date
                //Content Type, need to figure out how to do this better

                send(new_connect, "Content-Type: ", 14, 0);
                send(new_connect, fileExtentionString, sizeFES, 0);
                send(new_connect, "\r\n", 2, 0); 

                //Content Length
                //short sizeFSS = 16;//may need to truncate size down, doesnt seem to make a difference
                char fileSizeString[sizeFSS];
                for(short i = 0; i < sizeFSS; ++i){
                   fileSizeString[i] = ' '; //See how this responds
                }
                sprintf(fileSizeString, "%ld",st.st_size); 
                if (strcmp(httpCode, "200") != 0){
                   for(short i = 0; i < sizeFSS; ++i){
                       fileSizeString[i] = ' ';
                   }
                   strcpy(fileSizeString, "83");
                }
                send(new_connect, "Content-Length: ", 16, 0);
                send(new_connect, fileSizeString, sizeFSS, 0);
                send(new_connect, "\r\n", 2, 0);
                /////
                //Date
                time_t rawtime;
                time(&rawtime);
                struct tm * tm = localtime(&rawtime);
                //short timeBufferSize = 31;
                char timeBuffer[timeBufferSize];
                strftime(timeBuffer, timeBufferSize," %a, %d %b %Y %T %Z", tm);
                send(new_connect, "Date: ", 5, 0);
                send(new_connect, timeBuffer, sizeof(timeBuffer), 0);
                send(new_connect, "\r\n", 2, 0);
                send(new_connect, "\r\n", 2, 0);
                /////
                // Body
                   
                if ((sendContents == 1) && (strcmp(httpCode,"404") != 0)){
                printf("Client %s has requested %s\nFile contents are as follows\n\n", addressChar, filePath);
                    char c;
                    c = fgetc(tempFile);
                    for (long i = 0; i < st.st_size; ++i){                    
                        printf("%c",c);
                        send(new_connect, &c, 1, 0);
                        c = fgetc(tempFile);
                    }
                    fclose(tempFile);
                }
                else if(isHead !=0){//If head request
                    
                }
                else {
                    send(new_connect,
                    "<html>\n<head>\n<title>Error</title>\n</head>\n<body>\n<p>Error ",59, 0);
                     send(new_connect, httpCode, sizeof(httpCode), 0);
                     send(new_connect, "</p>\n</body>\n</html>\n", 21, 0);
                }
                printf("\nServer: Closing Connection to %s.\n", addressChar);
                close(new_connect);
            }
        }
        close(new_connect);
    }

    getchar();//Pause program until user input recieved. If this happens something when wrong, mainly for debuging

    return 0;
}
