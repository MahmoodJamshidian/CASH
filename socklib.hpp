#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <thread>
#include <functional>

namespace TCP
{
    class Connection
    {
    private:
        int sid;

    public:
        bool accepted = false;
        Connection(int socket_id)
        {
            this->sid = socket_id;
        }

        char *read_socket(int buffer_size)
        {
            char *buffer = (char *)malloc(sizeof(char) * buffer_size);
            read(this->sid, buffer, buffer_size);
            return buffer;
        }

        void write_socket(const char *data)
        {
            send(this->sid, data, strlen(data), 0);
        }

        void close_socket()
        {
            close(this->sid);
        }
    };

    class Server
    {
    private:
        int sid;
        bool closed = false;
        int accept_socket(char *client_addr[2])
        {
            struct sockaddr_in address;
            char *c_addr = (char *)malloc(sizeof(char) * INET_ADDRSTRLEN);
            int new_socket;
            int addrlen = sizeof(address);
            if ((new_socket = accept(this->sid, (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0)
            {
                throw std::runtime_error("acception error");
            }
            struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&address;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipAddr, c_addr, INET_ADDRSTRLEN);
            int port = ntohs(pV4Addr->sin_port);
            char sport[10];
            sprintf(sport, "%d", port);
            client_addr[0] = c_addr;
            client_addr[1] = sport;
            c_addr = NULL;
            delete c_addr;
            return new_socket;
        }

    public:
        uint16_t port;
        char *host;
        Server(const char *host, uint16_t port, std::function<void(Connection *, char *, uint16_t, Server *)> handler, uint8_t users_handle = 10)
        {
            if ((this->sid = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            {
                throw std::runtime_error("can't create socket");
            }
            this->host = (char *)host;
            this->port = port;
            struct sockaddr_in address;
            int opt = 1;
            int addrlen = sizeof(address);
            if (setsockopt(this->sid, SOL_SOCKET,
                           SO_REUSEADDR | SO_REUSEPORT, &opt,
                           sizeof(opt)))
            {
                throw std::runtime_error("can't set sockopt");
            }
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(host);
            address.sin_port = htons(port);

            if (bind(this->sid, (struct sockaddr *)&address,
                     sizeof(address)) < 0)
            {
                throw std::runtime_error("bind failed");
            }

            if (listen(this->sid, users_handle) < 0)
            {
                throw std::runtime_error("set user listening error");
            }

            char *c_address[2];

            while (true)
            {
                Connection connection(this->accept_socket(c_address));
                new std::thread(handler, &connection, c_address[0], atoi(c_address[1]), this);
                while (!connection.accepted)
                {
                }
                if (this->closed)
                {
                    break;
                }
            }
        }

        void close_socket()
        {
            this->closed = true;
            close(this->sid);
        }
    };

    class Client
    {
    private:
        int sid;

    public:
        Connection *connection;
        Client(const char *host, uint16_t port, std::function<void(Connection *)> handler)
        {
            int sid;
            if ((sid = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                throw std::runtime_error("can't create socket");
            }
            struct sockaddr_in serv_addr;

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)
            {
                throw std::runtime_error("invalid address");
            }

            if (connect(sid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                throw std::runtime_error("connection failed");
            }
            this->connection = new TCP::Connection(sid);
            if (handler != NULL)
            {
                handler(this->connection);
                this->connection->close_socket();
            }
        }
    };
}