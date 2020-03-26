//
//  GNU_UDP_client.hpp
//  SoapyClient
//
//  Created by Khramtsov Viktor on 07.05.19.
//

#ifndef GNU_UDP_client_hpp
#define GNU_UDP_client_hpp

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

class udp_client_server_runtime_error : public std::runtime_error
{
public:
    udp_client_server_runtime_error(const char *w) : std::runtime_error(w) {}
};


class udp_client
{
public:
    udp_client(const std::string& addr, int port);
    ~udp_client();
    
    int                 get_socket() const;
    int                 get_port() const;
    std::string         get_addr() const;
    
    int                 send(const char *msg, size_t size);
    
private:
    int                 f_socket;
    int                 f_port;
    std::string         f_addr;
    struct addrinfo *   f_addrinfo;
};


#endif /* GNU_UDP_client_hpp */
