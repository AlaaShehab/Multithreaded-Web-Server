//
// Created by alaa on 11/1/2019.
//

#ifndef SERVERCLIENTASSIGNMENT_HANDLEREQUEST_H
#define SERVERCLIENTASSIGNMENT_HANDLEREQUEST_H

#include <string>
#include <vector>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 4800
using namespace std;

class RequestHandler {
public:
    RequestHandler(int);
    ~RequestHandler();
    void process_request();
private:
    int client_socket;
    void handle_get(string);
    void handle_post(vector<string>);
    string get_directory();
    vector<string> tokenize (string s, string del);
    bool is_closed(int);

    void parse_request(string basicString);
};


#endif //SERVERCLIENTASSIGNMENT_HANDLEREQUEST_H
