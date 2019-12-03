#include "WebServer.h"

using namespace std;

int main(int argc, char* argv[]) {
    cout << "starting server\n";
    WebServer webServer;

    int port = atoi(argv[1]);
    webServer.init_server(port);
    return 0;
}