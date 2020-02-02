/**
 * This is a small sample of a simple chat server
 * 
 * Each message is sent as lines
 * 
 */

#include <cstdio>
#include <string>

#include <p0443_v2/asio/accept.hpp>

int main(int argc, char **argv) {
    std::uint16_t port = 0;

    if(argc != 2) {
        printf("Usage: telnet_lite_server <port>\n");
        return 1;    
    }

    port = std::stoi(argv[1]);
}