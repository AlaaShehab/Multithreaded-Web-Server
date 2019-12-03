//
// Created by alaa on 11/1/2019.
//

#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "RequestHandler.h"
using namespace std;

RequestHandler::RequestHandler(int client_socket) {
    this->client_socket = client_socket;
}

void RequestHandler::process_request() {

    //read data
    while (true) {
        string requests;
        if (is_closed(client_socket)) {
            break;
        }
        vector<char> server_response(BUFFER_SIZE, 0);

        int read_size = recv(client_socket, &server_response[0], BUFFER_SIZE, 0);
        if (read_size <= 0) {
            break;
        }
        requests += string(server_response.begin(), server_response.begin() + std::min(read_size, BUFFER_SIZE));
        if (requests.find("\r\n\r\n") != std::string::npos) {
            parse_request(requests);
        }
    }
    cout << "connection ended with client\n";
}

void RequestHandler::handle_get(string parsed_request) {
    cout << "Handling get request\n";
    //find file
    vector<string> uri = tokenize(parsed_request, " ");
    string fileName = get_directory() + uri[1];

    string response = "HTTP/1.1 200 OK\r\n";

    ifstream file(fileName.c_str(), ifstream::ate | ifstream::binary);
    if (!file.is_open()) {
        cout << "Cannot open file\n";
        const char *response = "HTTP/1.1 404 Not Found\r\n";
        write(client_socket, response, strlen(response));
        return;
    }
    long len = file.tellg();
    response += "Content-Length:" + to_string(len - 1);
    response += "\r\n\r\n";
    write(client_socket, (char*)response.c_str(), response.size());
    //read file
    char * buff;     //buffer to store file contents
    //retrieve get pointer position
    file.seekg (0, ios::beg);     //position get pointer at the begining of the file
    buff = new char [len];     //initialize the buffer
    memset(buff, '\0', sizeof(buff));

    file.read (buff, len);     //read file to buffer
    file.close();

    while (len  > 0){
        int length_sent = write(client_socket, buff, len);
        if(length_sent <= 0){
            break;
        }
        buff += length_sent;
        len -= length_sent;
    }
}

void RequestHandler::handle_post(vector<string> parsed_request) {
    cout << "Handling post request\n";

    const char *response = "HTTP/1.1 200 OK\r\n\r\n";
    write(client_socket, response, strlen(response));

    vector<string> uri = tokenize(parsed_request[0], " ");
    std::ofstream file(get_directory() + uri[1], ios::out | ios::binary);

    if (!file.is_open()) {
        cout << "Cannot open file\n";
    }

    size_t pos = parsed_request[1].find(":");
    if (pos == std::string::npos) {
        cout << "Cannot parse get request without content length\n";
        return;
    }
    int len = stoi(parsed_request[1].substr(pos + 1, parsed_request[1].length()));

    char * buff;
    buff = new char [len];
    memset(buff, '\0', sizeof(buff));

    while (len > 0) {
        int length_sent = read(client_socket, buff, len-1);
        if (length_sent == len || length_sent == len-1 || length_sent <= 0) {
            break;
        }
        buff += length_sent;
        len -= length_sent;
    }
    cout << buff << "\n";
    file.write(buff, strlen(buff));
    file.close();
    //sleep(10);
}
string RequestHandler::get_directory() {
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    string path = string( result, (count > 0) ? count : 0 );

    vector<string> tokens = tokenize(path, "/");
    string directory;
    for (int i = 0; i < tokens.size() - 2; i++) {
        directory += tokens[i] + "/";
    }
    directory += "Server";
    return directory;
}

vector<string> RequestHandler::tokenize (string str, string del) {
    vector<string> tokens;

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(del)) != std::string::npos) {
        token = str.substr(0, pos);
        tokens.push_back(token);
        str.erase(0, pos + del.length());
    }
    tokens.push_back(str) ;
    return tokens ;
}

RequestHandler::~RequestHandler() {
    close(client_socket);
}

void RequestHandler::parse_request(string request) {

    vector<string> all_requests = tokenize(request, "\r\n\r\n");

    for (int i = 0; i < all_requests.size(); i++) {
        cout << request + "\n";
        if (all_requests[i].empty()) {
            continue;
        }
        cout << "serving request \n";
        vector<string> request_tokenized = tokenize(all_requests[i], "\r\n");
        if (all_requests[i].find("POST") != std::string::npos) {
            handle_post(request_tokenized);
        } else if (all_requests[i].find("GET") != std::string::npos){
            handle_get(request_tokenized[0]);
        }
    }
}

bool RequestHandler::is_closed(int sock) {
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sock, &rfd);
    timeval tv = { 0 };
    select(sock, &rfd, 0, 0, &tv);
    if (!FD_ISSET(sock, &rfd))
        return false;
    int n = 0;
    ioctl(sock, FIONREAD, &n);
    return n == 0;
}

