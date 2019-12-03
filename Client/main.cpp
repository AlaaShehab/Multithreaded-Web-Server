//
// Created by alaa on 11/15/2019.
//

#include <netinet/in.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <string>
#include <vector>
#include <limits.h>
#include <unistd.h>
#include <fstream>

#define MAX_LEN 4800
using namespace std;

int client_socket;

vector<string> tokenize(string str, string del) {
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

string get_directory() {
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    string path = string( result, (count > 0) ? count : 0 );

    vector<string> tokens = tokenize(path, "/");
    string directory;
    for (int i = 0; i < tokens.size() - 2; i++) {
        directory += tokens[i] + "/";
    }
    directory += "Client";
    return directory;
}

void send_get_request(string request) {
    //receive OK or NOT FOUND from server
    string response;
    while (true) {
        cout << "Client: reading 200/404 response\n";
        vector<char> server_response(4800, 0);
        int read_size = recv(client_socket, &server_response[0], 4800, 0);
        if (read_size <= 0) {
            break;
        }
        response += string(server_response.begin(), server_response.begin() + std::min(read_size, 4800));
        if (response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    vector<string> response_tokenized = tokenize(response, "\r\n");

    if (response_tokenized[0].find("404 Not Found") != std::string::npos) {
        return;
    }

    //get content length size
    size_t pos = response_tokenized[1].find(":");
    if (pos == std::string::npos) {
        cout << "Cannot parse get request without content length\n";
        return;
    }
    int len = stoi(response_tokenized[1].substr(pos + 1, response_tokenized[1].length()));

    //receive data from server
    cout << "Recieving data from server\n";
    char * buff;
    buff = new char [len];
    memset(buff, '\0', sizeof(buff));

    while (len > 0) {
        int length_sent = read(client_socket, buff, len-1);
        if (length_sent <=0 || length_sent == len || length_sent == len-1) {
            break;
        }
        buff += length_sent;
        len -= length_sent;
    }

    cout << buff << "\n";
    std::ofstream file(get_directory() + "image.jpg", ios::out | ios::binary);

    file.write(buff, strlen(buff));
    file.close();
}

void send_post_request(string request) {
    cout << "Client: Sending post request\n";
    int n;
    char sendline[MAX_LEN];
    vector<string> token_request = tokenize(request, " ");

    //read data from file
    string fileName = get_directory() + token_request[1];

    ifstream file(fileName.c_str(), ifstream::ate | ifstream::binary);
    if (!file.is_open()) {
        cout << "File not found! cannot send data to server\n";
        return;
    }
    long len = file.tellg();

    //send request
    request += "\r\nContent-Length:" + to_string(len);
    request += "\r\n\r\n";
    if ((write(client_socket , (char* )request.c_str() , request.size())) < 0) {
        perror("failed to write to socket client->server");
    }

    string response;
    while (true) {
        vector<char> server_response(4800, 0);
        int read_size = recv(client_socket, &server_response[0], 4800, 0);
        if (read_size <= 0) {
            break;
        }
        response += string(server_response.begin(), server_response.begin() + std::min(read_size, 4800));
        if (response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    cout << response << "\n";
    //read file
    char * buff;     //buffer to store file contents
    //retrieve get pointer position
    file.seekg (0, ios::beg);     //position get pointer at the begining of the file
    buff = new char [len];     //initialize the buffer
    memset(buff, '\0', sizeof(buff));
    file.read (buff, len);     //read file to buffer
    file.close();

    //send file to server
    while (len  > 0){
        int length_sent = write(client_socket, buff, len);
        if(length_sent == -1){
            return;
        }
        if(length_sent == 0){
            break;
        }
        buff += length_sent;
        len -= length_sent;
    }

    cout << "Request send successfully\n";
}

void send_get_only (vector<string> get_requests) {
    for (int i = 0; i < get_requests.size(); i++) {
        cout << "Client: Sending get request\n";
        int n;
        char recvline[MAX_LEN];
        //send request
        get_requests[i] += "\r\n\r\n";
        if ((write(client_socket , (char* )get_requests[i].c_str() , get_requests[i].size())) < 0) {
            perror("failed to write to socket client->server");
        }
    }
    for (int i = 0; i < get_requests.size(); i++) {
        send_get_request(get_requests[i]);
    }
}
void parse_requests(string file_path) {
    cout << "Client looking for requests at: " << file_path + "\n";

    //Open file
    ifstream infile(file_path);
    if (!infile.is_open()) {
        perror("file not found! cannot parse requests in client side");
        return;
    }
    string line;
    vector<string> get_requests;
    while (getline(infile, line)) {
        vector<string> request = tokenize(line, " ");
        if (request[0] == "GET") {
            get_requests.push_back(line);
        }

        if (request[0] == "POST") {
            //loop on get request to pipeline
            if (!get_requests.empty()) {
                send_get_only(get_requests);
                get_requests.clear();
            }
            send_post_request(line);
        }
    }
    //loop on get request to pipeline
    if (!get_requests.empty()) {
        send_get_only(get_requests);
        get_requests.clear();
    }
    cout << "closing client socket\n";
    close(client_socket);
}

void handle_request(string filename) {
    parse_requests(get_directory() + filename);
}

void init_client(int port, string filename) {
    struct sockaddr_in server_addr;

    memset(&client_socket, '0', sizeof(client_socket));

    //create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("failed to create socket");
        exit(EXIT_FAILURE);
    }
    cout << "client socket created successfully\n";

    //clear the address
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("failed to connect to server");
        exit(EXIT_FAILURE);
    }
    cout << "client connected to server successfully\n";

    //filename in which the requests are stored
    handle_request(filename);
}

int main (int argc, char* argv[]) {
    init_client(atoi(argv[1]), "/requests");
}