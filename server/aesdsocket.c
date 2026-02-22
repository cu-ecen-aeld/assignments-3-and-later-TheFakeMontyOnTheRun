#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

struct thread_data
{
    pthread_t id;
    int sockfd;
    char ipstr[INET_ADDRSTRLEN];

};

struct sigaction handler;

/* general flag for stopping and cleaning up */
int running = 1;

/* are we runnning as daemon? */
int runAsDaemon = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void handleSignals()
{
    if (running)
    {
        syslog(LOG_DEBUG, "Caught signal, exiting");
        running = 0;
    }
}

/**
 * lifted from a game I wrote a couple of years ago. It kind of sucked tho.
 * I had to modify it in a few spots and might reflect those improvements back into the game :)
 * @return new socket, ready for accepting connections
 */
int createAcceptingSocket()
{
    int sockfd, portno;

    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        syslog(LOG_ERR, "Could not create listening socket");
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
        syslog(LOG_ERR, "Could not bind listening socket");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void saveIncomingData(int newsockfd)
{
    FILE* output;
    ssize_t bytesRead = 0;
    pthread_mutex_lock(&mutex);
    output = fopen("/var/tmp/aesdsocketdata", "a+");
    do
    {
        char buffer[256];
        memset(buffer, 0, 256);
        bytesRead = recv(newsockfd, buffer, 256, MSG_DONTWAIT);
        if (bytesRead > 0)
        {
            /* append the received data */
            fwrite(&buffer, 1, bytesRead, output);
        }
    }
    while (bytesRead > 0);
    fclose(output);
    pthread_mutex_unlock(&mutex);
}

char* fullyReadExistingData(size_t *size)
{
    pthread_mutex_lock(&mutex);
    FILE* data = fopen("/var/tmp/aesdsocketdata", "r");
    fseek(data, 0, SEEK_END);
    *size = ftell(data);
    char* currentBuffer = (char*)malloc(*size);
    rewind(data);
    fread(currentBuffer, 1, *size, data);
    fclose(data);
    pthread_mutex_unlock(&mutex);
    return currentBuffer;
}

void* connectionHandler(void *thread_data)
{
    struct thread_data data = *((struct thread_data*)thread_data);

    saveIncomingData(data.sockfd);

    /* fully read the existing data */
    size_t size;
    char* currentBuffer = fullyReadExistingData(&size);

    /* send what we have back */
    ssize_t offset = 0;
    while (offset < size)
    {
        offset += send(data.sockfd, currentBuffer + offset, size - offset, 0);
    }

    /* clean up for this peer */
    free(currentBuffer);
    close(data.sockfd);
    syslog(LOG_DEBUG, "Closed connection from %s", data.ipstr);
    free(thread_data);
    return NULL;
}

int main(int argc, char** argv)
{
    /* Setup signal handler */
    handler.sa_handler = handleSignals;
    sigaction(SIGINT, &handler, NULL);
    sigaction(SIGTERM, &handler, NULL);

    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        runAsDaemon = 1;
    }

    /* Accept incoming connections */
    int sock = createAcceptingSocket();

    if (sock == -1)
    {
        syslog(LOG_ERR, "Could not create accepting socket");
        return -1;
    }

    if (listen(sock, 5) < 0)
    {
        syslog(LOG_ERR, "Could not listen on listening socket");
        close(sock);
        running = 0;
        return -1;
    }

    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    if (runAsDaemon) {
        if (fork()) {
            /* am I the root process? (that is, I got a PID from the fork call) */
            return 0;
        } else {
            /* I'm the child process - let's roll. */
            setsid();
        }
    }

    while (running)
    {
        int newsockfd = accept(sock, (struct sockaddr*)&cli_addr, &clilen);

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ipstr, sizeof ipstr);

        syslog(LOG_DEBUG, "Accepted connection from %s", ipstr);

        /* very ugly hack */
        struct thread_data *data = calloc(1, sizeof(struct thread_data));
        data->sockfd = newsockfd;
        memcpy(&data->ipstr, ipstr, sizeof(ipstr));

        pthread_create(&data->id, NULL, connectionHandler, data);
    }


    /* clean up server */
    unlink("/var/tmp/aesdsocketdata");
    close(sock);

    return 0;
}
