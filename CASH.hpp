#include <openssl/aes.h>
#include <openssl/sha.h>
#include <functional>
#include "socklib.hpp"
#include <vector>
#include <stdlib.h>
#include <string>

#define PORT 3021
#define START_C_MSG "\33\255"
#define OK "o"
#define CLOSE "c"
#define END "e"
#define RUN_COMMAND "r"
#define BLOCK_SIZE 50
#define CONTENT_SIZE 16
#define KEY_SIZE 128

namespace utils
{
    std::vector<std::string> nsplit(std::string msg, size_t len)
    {
        std::vector<std::string> res;
        res.push_back("");
        uint index = 0;
        for (size_t ind = 0; ind < msg.length(); ind++)
        {
            if (ind % len == 0 && ind)
            {
                res.push_back("");
                index++;
            }
            res[index] += msg[ind];
        }
        return res;
    }

    struct CommandResult
    {
        std::string output;
        int exitstatus;
    };

    CommandResult execute(const std::string &command)
    {
        int exitcode = 255;
        std::array<char, 1048576> buffer{};
        std::string result;
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS
#endif
        FILE *pipe = popen(command.c_str(), "r");
        if (pipe == nullptr)
        {
            throw std::runtime_error("popen() failed!");
        }
        try
        {
            std::size_t bytesread;
            while ((bytesread = fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0)
            {
                result += std::string(buffer.data(), bytesread);
            }
        }
        catch (...)
        {
            pclose(pipe);
            throw;
        }
        exitcode = WEXITSTATUS(pclose(pipe));
        return CommandResult{result, exitcode};
    }
}

namespace server
{
    class CASH
    {
    public:
        CASH(const char *host, const char s_key[CONTENT_SIZE], const char c_key[CONTENT_SIZE])
        {
            static char _c_key[CONTENT_SIZE] = {};
            static char _s_key[CONTENT_SIZE] = {};
            for (std::uint8_t i = 0; i < CONTENT_SIZE; i++)
            {
                if (i < strlen(c_key))
                {
                    _c_key[i] = c_key[i];
                }
                if (i < strlen(s_key))
                {
                    _s_key[i] = s_key[i];
                }
            }
            static utils::CommandResult c_res;
            TCP::Server(host, PORT, [](TCP::Connection *connection, char *c_addr, uint16_t c_port, TCP::Server *socket)
                        {
                            connection->accepted = true;
                            AES_KEY enc_key, dec_key;
                            unsigned char *res = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char)), *rst = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));
                            AES_set_decrypt_key((unsigned char *)_c_key, KEY_SIZE, &dec_key);
                            AES_set_encrypt_key((unsigned char *)_s_key, KEY_SIZE, &enc_key);

                            char * test = connection->read_socket(BLOCK_SIZE);

                            AES_decrypt((unsigned char *)test, rst, &dec_key);

                            if(strcmp((char *)rst, START_C_MSG) != 0)
                            {
                                connection->write_socket(CLOSE);
                                connection->close_socket();
                                return;
                            }
                            connection->write_socket(OK);

                            const char *rand_int = std::to_string(rand()).c_str();

                            AES_encrypt((unsigned char *)rand_int, rst, &enc_key);

                            connection->write_socket((char *)rst);

                            AES_decrypt((unsigned char *)connection->read_socket(BLOCK_SIZE), rst, &dec_key);

                            if(strcmp((char *)rst, rand_int) != 0)
                            {
                                connection->write_socket(CLOSE);
                                connection->close_socket();
                                return;
                            }
                            connection->write_socket(OK);

                            char *c_data;
                            std::string cmd;
                            utils::CommandResult exec_res;
                            std::vector<std::string> exec_res_str;
                            char exit_status;
                            std::string t_str;
                            while (strcmp((c_data = connection->read_socket(BLOCK_SIZE)), "") != 0)
                            {
                                if(strcmp(c_data, RUN_COMMAND) == 0)
                                {
                                    cmd = "";
                                    while(strcmp((c_data = connection->read_socket(BLOCK_SIZE)), END) != 0)
                                    {
                                        if(strcmp(c_data, "") == 0)
                                        {
                                            connection->close_socket();
                                            return;
                                        }
                                        connection->write_socket(OK);
                                        AES_decrypt((unsigned char *)c_data, rst, &dec_key);
                                        cmd += (char *)rst;
                                    }
                                    exec_res = utils::execute(cmd);
                                    t_str = (char)exec_res.exitstatus;
                                    AES_encrypt((unsigned char *)t_str.c_str(), rst, &enc_key);
                                    connection->write_socket((char *)rst);
                                    connection->read_socket(BLOCK_SIZE);
                                    
                                    char *res_char = (char *)malloc(sizeof(char) * exec_res.output.size());
                                    for(size_t i = 0; i < exec_res.output.size(); i++){
                                        res_char[i] = exec_res.output[i];
                                    }
                                    std::string res_char_s = res_char;
                                    
                                    exec_res_str = utils::nsplit(res_char_s, CONTENT_SIZE);
                                    for(int i = 0; i < exec_res_str.size(); i++)
                                    {
                                        AES_encrypt((unsigned char *)exec_res_str[i].c_str(), rst, &enc_key);
                                        connection->write_socket((char *)rst);
                                        connection->read_socket(BLOCK_SIZE);
                                    }
                                    connection->write_socket(END);
                                    continue;
                                }
                            }
                            
                            connection->close_socket(); });
        }
    };
}

namespace client
{
    class CASH
    {
        TCP::Connection *g_con;
        AES_KEY enc_key, dec_key;

    public:
        CASH(const char *host, const char s_key[CONTENT_SIZE], const char c_key[CONTENT_SIZE], std::function<void(client::CASH *)> handler = NULL)
        {
            char _c_key[CONTENT_SIZE] = {};
            char _s_key[CONTENT_SIZE] = {};
            for (std::uint8_t i = 0; i < CONTENT_SIZE; i++)
            {
                if (i < strlen(c_key))
                {
                    _c_key[i] = c_key[i];
                }
                if (i < strlen(s_key))
                {
                    _s_key[i] = s_key[i];
                }
            }
            AES_set_encrypt_key((unsigned char *)_c_key, KEY_SIZE, &enc_key);
            AES_set_decrypt_key((unsigned char *)_s_key, KEY_SIZE, &dec_key);
            TCP::Connection *connection = TCP::Client(host, PORT, NULL).connection;
            unsigned char *res = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char)), *rst = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));

            AES_encrypt((unsigned char *)START_C_MSG, rst, &enc_key);

            connection->write_socket((char *)rst);

            if (strcmp(connection->read_socket(1), OK) != 0)
            {
                throw std::runtime_error("connection faild");
            }

            AES_decrypt((unsigned char *)connection->read_socket(1024), rst, &dec_key);
            AES_encrypt(rst, res, &enc_key);

            connection->write_socket((char *)res);

            if (strcmp(connection->read_socket(1), OK) != 0)
            {
                throw std::runtime_error("connection faild");
            }

            g_con = connection;

            if (handler != NULL)
            {
                handler(this);
                connection->close_socket();
            }
        }

        void close_socket()
        {
            g_con->close_socket();
        }

        utils::CommandResult run(const char *cmd)
        {
            utils::CommandResult _res;
            if (strcmp(cmd, "") == 0)
            {
                _res = {"", 0};
                return _res;
            }
            std::vector<std::string> res = utils::nsplit(cmd, CONTENT_SIZE);
            unsigned char *rst = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));
            unsigned char *enc_res = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));
            g_con->write_socket(RUN_COMMAND);
            for (size_t i = 0; i < res.size(); i++)
            {
                rst = (unsigned char *)res[i].c_str();
                AES_encrypt(rst, enc_res, &enc_key);
                g_con->write_socket((char *)enc_res);
                g_con->read_socket(BLOCK_SIZE);
            }
            g_con->write_socket(END);
            char *resp;
            std::string d_resp;
            if (strcmp((resp = g_con->read_socket(BLOCK_SIZE)), "") != 0)
            {
                AES_decrypt((unsigned char *)resp, rst, &dec_key);
                _res.exitstatus = (int)rst[0];
                g_con->write_socket(OK);
            }
            else
            {
                throw std::runtime_error("connection closed");
            }
            while (strcmp((resp = g_con->read_socket(BLOCK_SIZE)), END) != 0)
            {
                if (strcmp(resp, "") == 0)
                {
                    throw std::runtime_error("connection closed");
                }
                g_con->write_socket(OK);
                AES_decrypt((unsigned char *)resp, rst, &dec_key);
                d_resp += (char *)rst;
            }
            _res.output = d_resp;
            return _res;
        }
    };
}