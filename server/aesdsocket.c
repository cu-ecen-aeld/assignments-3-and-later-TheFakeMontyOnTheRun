#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 * lifted from a game I wrote a couple of years ago. It kind of sucked tho.
 * I had to modify it in a few spots and might reflect those improvements back into the game :)
 * @return new socket, ready for accepting connections
 */
int createAcceptingSocket()
{
    int sockfd, portno;

    struct sockaddr_in serv_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = 9000;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    int ret = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (ret < 0)
    {
        close(sockfd);
        return -1;
    }

    ret = listen(sockfd, 5);

    if (ret < 0)
    {
        close(sockfd);
        return -1;
    }

    return sockfd;
}


int main(int argc, char** argv)
{
    /* prepare the data repository */
    FILE *output;
    output = fopen("/var/tmp/aesdsocketdata", "a+");

    /* Accept incoming connections */
    int sock = createAcceptingSocket();
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    int newsockfd = accept(sock, (struct sockaddr*)&cli_addr, &clilen);
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli_addr.sin_addr, ipstr, sizeof ipstr);
    printf("Accepted connection from %s\n", ipstr);


    ssize_t bytesRead = 0;
    do {
        char buffer[257];
        memset(buffer, 0, 257);
        bytesRead = recv(newsockfd, buffer, 256, MSG_DONTWAIT);
        if (bytesRead > 0) {
            /* append the received data */
            fwrite(&buffer, bytesRead, 1, output);
            buffer[0] = '\n';
            fwrite(&buffer, 1, 1, output);

            /* fully read the existing data */
            char *currentBuffer = NULL;
            FILE* data = fopen("/var/tmp/aesdsocketdata", "r");
            fseek(data, 0, SEEK_END);
            size_t size = ftell(data) + 1;
            currentBuffer = (char*)malloc(size);
            rewind(data);
            fread(currentBuffer, 1, size, data);
            currentBuffer[size - 1] = 0;

            /* send what we have back */
            ssize_t offset = 0;
            while (offset < size) {
                offset += send(newsockfd, currentBuffer + offset, size - offset, 0);
            }

            /* clean up */
            free(currentBuffer);
            fclose(data);
        }
    } while (bytesRead > 0);



    /* clean up */
    fclose(output);
    close(newsockfd);
    close(sock);

    return 0;
}
