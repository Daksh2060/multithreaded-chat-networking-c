#define _DEFAULT_SOURCE
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "list.h"

// Buffer to store messages, adjust size to alter message character limit
#define MAX_BUFFER_SIZE 256

// Global variables
int sock_fd;
struct sockaddr_in peer_addr;

// Lists to store messages to be sent and received
List *ToBeSent;
List *Received;

// Mutexes and condition variables for thread synchronization
pthread_mutex_t ToBeSentMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ReceivedMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t toBeSentCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t receivedCond = PTHREAD_COND_INITIALIZER;

// Boolean to control the main loop
bool cont = true;

/**
 * @brief Thread function to handle keyboard input.
 *
 * This function reads keyboard input, processes it, and adds it to the ToBeSent list.
 *
 * @return NULL
 */
void *keyInputThread()
{
    // Set non-blocking input for keyboard
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    char input[MAX_BUFFER_SIZE];
    int size;

    while (cont)
    {
        // Read keyboard input
        size = read(0, input, sizeof(input));

        if (size > 0)
        {
            input[size] = '\0';

            // Check for the termination command
            if (strcmp(input, "!\n") == 0 || strcmp(input, "!\0") == 0)
            {
                cont = false;
                char *send = strdup(input);

                // Send termination command
                sendto(sock_fd, input, size, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
                free(send);
                break;
            }

            // Lock the mutex, add message to the list, and signal the condition variable
            pthread_mutex_lock(&ToBeSentMutex);
            {
                List_append(ToBeSent, input);
            }
            pthread_mutex_unlock(&ToBeSentMutex);

            pthread_cond_signal(&toBeSentCond);
        }
    }

    return NULL;
}

/**
 * @brief Thread function to handle sending messages over UDP.
 *
 * This function continuously checks the ToBeSent list and sends messages over UDP.
 *
 * @return NULL
 */
void *UDPOutputThread()
{
    while (cont)
    {
        pthread_mutex_lock(&ToBeSentMutex);

        // Wait for a message to be available in the list
        while (List_count(ToBeSent) == 0)
        {
            pthread_cond_wait(&toBeSentCond, &ToBeSentMutex);
        }

        if (List_first(ToBeSent))
        {
            // Get the message from the list, send it, and free the memory
            char *item = List_remove(ToBeSent);
            char *send = strdup(item);
            sendto(sock_fd, send, strlen(send), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
            free(send);
        }

        pthread_mutex_unlock(&ToBeSentMutex);
    }

    return NULL;
}

/**
 * @brief Thread function to handle receiving messages over UDP.
 *
 * This function continuously receives messages over UDP and adds them to the Received list.
 *
 * @return NULL
 */
void *UDPInputThread()
{
    char input[MAX_BUFFER_SIZE];
    int size;

    while (cont)
    {
        // Receive a message
        size = recvfrom(sock_fd, input, sizeof(input), 0, NULL, NULL);
        if (size < 0)
        {
            perror("socket recvfrom error");
            break;
        }
        if (size > 0)
        {
            input[size] = '\0';

            pthread_mutex_lock(&ReceivedMutex);
            char *receivedData;
            receivedData = strdup(input);

            // Check for the termination command
            if (strcmp(receivedData, "!\n") == 0 || strcmp(receivedData, "!\0") == 0)
            {
                cont = false;
                List_append(Received, receivedData);

                pthread_cond_signal(&receivedCond);
                pthread_mutex_unlock(&ReceivedMutex);
                break;
            }
            else
            {
                // Add the received message to the list and signal the condition variable
                List_append(Received, receivedData);
                pthread_cond_signal(&receivedCond);
                pthread_mutex_unlock(&ReceivedMutex);
            }
        }
    }

    return NULL;
}

/**
 * @brief Thread function to handle displaying received messages on the screen.
 *
 * This function continuously checks the Received list and prints messages to the screen.
 *
 * @return NULL
 */
void *screenOutputThread()
{
    while (cont)
    {
        pthread_mutex_lock(&ReceivedMutex);

        // Wait for a message to be available in the list
        while (List_count(Received) == 0)
        {
            pthread_cond_wait(&receivedCond, &ReceivedMutex);
        }

        List_first(Received);
        char *print = List_remove(Received);

        if (strcmp(print, "!\n") == 0 || strcmp(print, "!\0") == 0)
        {
            free(print);
            pthread_mutex_unlock(&ReceivedMutex);
            break;
        }
        else
        {
            // Display the received message on the screen
            FILE *file = stdout;
            printf("Received > ");
            fputs(print, file);
            free(print);
            pthread_mutex_unlock(&ReceivedMutex);
        }
    }

    return NULL;
}

/**
 * @brief Function to free memory for list items.
 *
 * This function is a callback used by the List_free function.
 *
 * @param pItem Pointer to the item to be freed.
 */
void pItemFreeFn(void *pItem)
{
    if (pItem)
    {
        pItem = NULL;
    }
}

/**
 * @brief Main function.
 *
 * This function initializes the program, sets up sockets, creates threads, and manages their lifecycle.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[])
{
    // Initialize lists
    ToBeSent = List_create();
    Received = List_create();

    // Check command-line arguments
    if (argc < 4)
    {
        printf("Please Enter: <local port> <remote machine name> <remote port>\n");
        return 1;
    }

    // Parse local and remote ports from command-line arguments
    unsigned long local_port = strtoul(argv[1], NULL, 0);
    if (local_port < 1024 || local_port > 65535)
    {
        printf("Please enter a valid local port (1024 - 65535)\n");
        return 1;
    }

    unsigned long remote_port = strtoul(argv[3], NULL, 0);
    if (remote_port < 1024 || remote_port > 65535)
    {
        printf("Please enter a valid remote port (1024 - 65535)\n");
        return 1;
    }

    // Set up address information for the remote machine
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(argv[2], NULL, &hints, &result);
    if (ret != 0)
    {
        printf("Error - invalid remote address\n");
        return (1);
    }

    // Set up the peer address
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(remote_port);
    struct sockaddr_in *remote_addr = NULL;
    struct addrinfo *curr = result;

    // Find the appropriate address from the result
    while (curr)
    {
        if (curr->ai_family == AF_INET && curr->ai_addr != NULL)
        {
            remote_addr = (struct sockaddr_in *)curr->ai_addr;
            break;
        }
        curr = curr->ai_next;
    }

    // Check for a valid address
    if (inet_aton(inet_ntoa(remote_addr->sin_addr), &peer_addr.sin_addr) == 0)
    {
        printf("Error - invalid remote address\n");
        return 1;
    }

    // Create a socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        printf("Failed to open socket\n");
        return 1;
    }

    // Set up the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(local_port);

    // Bind the socket
    if (bind(sock_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr)) < 0)
    {
        printf("Failed to bind socket\n");
        return 1;
    }

    // Create threads for different tasks
    pthread_t k_input_thread, UDP_out, UDP_in, screen_out;

    // Create and check the creation of threads
    if (pthread_create(&k_input_thread, NULL, keyInputThread, NULL) != 0)
    {
        printf("pthread_create for keyInputThread failed");
        return 1;
    }

    if (pthread_create(&UDP_out, NULL, UDPOutputThread, NULL) != 0)
    {
        printf("pthread_create for UDP_out failed");
        return 1;
    }

    if (pthread_create(&UDP_in, NULL, UDPInputThread, NULL) != 0)
    {
        printf("pthread_create for UDP_in failed");
        return 1;
    }

    if (pthread_create(&screen_out, NULL, screenOutputThread, NULL) != 0)
    {
        printf("screen_out for UDP_out failed");
        return 1;
    }

    // Detach threads for independent termination
    pthread_detach(UDP_out);
    pthread_detach(screen_out);
    pthread_detach(UDP_in);

    // Wait for the termination of the key input thread and initiate termination of other threads
    if (pthread_join(k_input_thread, NULL) == 0)
    {
        pthread_cancel(UDP_out);
        usleep(10000);
        pthread_cancel(screen_out);
        usleep(10000);
        pthread_cancel(UDP_in);
        usleep(10000);
    }

    // Wait for the termination of the remaining threads
    pthread_join(UDP_out, NULL);
    pthread_join(screen_out, NULL);
    pthread_join(UDP_in, NULL);

    // Free address information and close the socket
    freeaddrinfo(result);
    close(sock_fd);

    // Free memory used by lists
    List_free(Received, pItemFreeFn);
    List_free(ToBeSent, pItemFreeFn);

    printf("\n< Chat Has Been Ended >\n");

    return 0;
}
