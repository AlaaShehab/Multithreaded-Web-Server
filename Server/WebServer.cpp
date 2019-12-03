//
// Created by alaa on 11/1/2019.
//

#include <netinet/in.h>
#include "WebServer.h"
#include <pthread.h>
#include <asm/ioctls.h>

void WebServer::init_server(int port) {
    //create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //handle error in socket creation
    if (server_socket == 0) {
        perror("failed to create socket");
        exit(EXIT_FAILURE);
        close(server_socket);
    }
    cout << "Server socket created successfully\n";

    //bind socket with addr
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("failed to bind socket with addr and port specified");
        exit(EXIT_FAILURE);
        close(server_socket);
    }
    cout << "Server socket binded with port successfully\n";

    //listen to connections
    if (listen(server_socket, BACK_LOG) < 0) {
        perror("failed to listen to socket");
        exit(EXIT_FAILURE);
    }
    cout << "Server socket listening to connections successfully\n";

    //accept connection
    accept_connections(server_socket);
}

void * WebServer::handle_request(void * new_socket) {
    int client_socket = *((int*) new_socket);
    free(new_socket);

    cout << "Server: client connected\n";
    auto *requestHandler = new RequestHandler(client_socket);
    requestHandler->process_request();

    return NULL;
}

void WebServer::accept_connections(int server_socket) {
    struct timeval tv;

    while(true) {
        tv.tv_sec = 7 * number_of_connections;
        tv.tv_usec = 0;
        if ((setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv)) < 0) {
            cout << "error setting socket\n";
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        cout << "Server socket started to accept connections\n";

        //accept connections
        int client_socket = accept(server_socket,
                                   (struct sockaddr *) &client_addr, &addrlen);
        cout << "Server socket accepted connection successfully\n";

        if (client_socket < 0) {
            perror("accept failed");
            break;
        }

        pthread_t thread;
        int *pclient = (int*)malloc(sizeof(int));
        *pclient = client_socket;
        number_of_connections++;

        pthread_create(&thread, NULL, handle_request, pclient);
    }
    close(server_socket);
}
