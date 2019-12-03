//
// Created by alaa on 11/1/2019.
//

#ifndef SERVERCLIENTASSIGNMENT_CREATESERVER_H
#define SERVERCLIENTASSIGNMENT_CREATESERVER_H


#include <bits/stdc++.h>
#include <sys/socket.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <thread_db.h>
#include "RequestHandler.h"

#define BACK_LOG 50
using namespace std;

class WebServer {
public:
    void init_server(int);
private:
    int number_of_connections = 1;
    static void * handle_request(void *);
    void accept_connections(int server_soscket);
};


#endif //SERVERCLIENTASSIGNMENT_CREATESERVER_H
