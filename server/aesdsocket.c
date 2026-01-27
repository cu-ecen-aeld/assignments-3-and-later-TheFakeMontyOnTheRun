//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netinet/tcp.h>

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
    FILE *output;
    int sock = createAcceptingSocket();
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    int newsockfd = accept(sock, (struct sockaddr*)&cli_addr, &clilen);
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli_addr.sin_addr, ipstr, sizeof ipstr);
    printf("Accepted connection from %s\n", ipstr);
    output = fopen("/var/tmp/aesdsocketdata", "w");
    size_t bytesRead = 0;
    do
    {
        char buffer[257];
        memset(buffer, 0, 257);
        bytesRead = recv(newsockfd, buffer, 256, 0);
        puts(buffer);

        fwrite(&buffer, bytesRead, 1, output);

    } while (bytesRead > 0);

    fclose(output);
    close(newsockfd);
    close(sock);

    return 0;
}
