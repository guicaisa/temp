#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include "player.pb.h"
#include "cl.pb.h"
#include "message.h"

using boost::asio::ip::tcp;

class Server;
void tempfunc(Server *server, const std::string& str);


class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start()
    {
        do_read();
    }

public:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        printf("read: %d\n", length);
                                        //do_write(length);
                                        cacheReadDatas(length);
                                        do_read();
                                    }
                                });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                     }
                                 });
    }

    void response(uint32_t msgType, const std::string& str)
    {
        Message rspMsg;
        rspMsg.length_ = str.size() + Message::package_head_size;
        rspMsg.msg_type_ = msgType;
        for (int i = 0; i < str.size(); ++i)
        {
            rspMsg.body_.push_back(str[i]);
        }
        std::string sendStr = rspMsg.Convert2Str();

        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(sendStr.c_str(), sendStr.size()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                   
                                 });
    }

    void write(char*data, int length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                   
                                 });
    }

    void broadcastNotify(uint32_t msgType, const std::string& str)
    {
        Message rspMsg;
        rspMsg.length_ = str.size() + Message::package_head_size;
        rspMsg.msg_type_ = msgType;
        for (int i = 0; i < str.size(); ++i)
        {
            rspMsg.body_.push_back(str[i]);
        }
        std::string sendStr = rspMsg.Convert2Str();

        tempfunc(server_, sendStr);
    }

    void cacheReadDatas(const std::size_t length)
    {
        for (int i = 0; i < length; ++i)
        {
            bytes_buffer_.emplace_back(data_[i]);
        }
        bool break_loop = false;
        while (1)
        {
            switch (status_)
            {
            case 0:
                if (bytes_buffer_.size() >= Message::package_head_size)
                {
                    char c1 = bytes_buffer_[0];
                    char c2 = bytes_buffer_[1];
                    char c3 = bytes_buffer_[2];
                    char c4 = bytes_buffer_[3];
                    uint32_t length = c1 << 24 | c2 << 16 | c3 << 8 | c4;

                    c1 = bytes_buffer_[4];
                    c2 = bytes_buffer_[5];
                    c3 = bytes_buffer_[6];
                    c4 = bytes_buffer_[7];
                    uint32_t msg_type = c1 << 24 | c2 << 16 | c3 << 8 | c4;

                    c1 = bytes_buffer_[8];
                    c2 = bytes_buffer_[9];
                    c3 = bytes_buffer_[10];
                    c4 = bytes_buffer_[11];
                    uint32_t sequence = c1 << 24 | c2 << 16 | c3 << 8 | c4;

                    c1 = bytes_buffer_[12];
                    c2 = bytes_buffer_[13];
                    c3 = bytes_buffer_[14];
                    c4 = bytes_buffer_[15];
                    uint32_t flags = c1 << 24 | c2 << 16 | c3 << 8 | c4;

                    c1 = bytes_buffer_[16];
                    c2 = bytes_buffer_[17];
                    c3 = bytes_buffer_[18];
                    c4 = bytes_buffer_[19];
                    uint32_t reserved = c1 << 24 | c2 << 16 | c3 << 8 | c4;

                    message_.length_ = length - Message::package_head_size;
                    message_.msg_type_ = msg_type;
                    message_.sequence_ = sequence;
                    message_.flags_ = flags;
                    message_.reserved_ = reserved;

                    status_ = 1;
                    bytes_buffer_.erase(bytes_buffer_.begin(), bytes_buffer_.begin() + Message::package_head_size);
                } else 
                {
                    break_loop = true;
                }
                break;
            case 1:
                if (bytes_buffer_.size() >= message_.length_)
                {
                    message_.body_ = std::vector<char>(bytes_buffer_.begin(), bytes_buffer_.begin() + message_.length_);
                    bytes_buffer_.erase(bytes_buffer_.begin(), bytes_buffer_.begin() + message_.length_);
                    status_ = 0;

                    processMessage();

                    // Player player;
                    // player.ParseFromArray(message_.BodyToBytes(), message_.length_);
                    // printf("player: id: %d, name: %s, score: %d\n", player.id(), player.name().c_str(), player.score());
                    // int a = player.id();
                }
                break_loop = true;
                break;
            }
            if (break_loop)
            {
                break;
            }
        }
    }

    void processMessage() 
    {
        printf("processMessage: %d\n", message_.msg_type_);
        switch (message_.msg_type_)
        {
            case ID_C2L_EnterWorld:
                C2L_EnterWorld req;
                req.ParseFromArray(message_.BodyToBytes(), message_.length_);

                L2C_EnterWorld rsp;
                rsp.set_ret(1);
                rsp.set_uid(req.uid());
                std::string serialized_data_rsp;
                rsp.SerializeToString(&serialized_data_rsp);
                response(ID_L2C_EnterWorld, serialized_data_rsp);

                L2C_NotifyEnterWorld broadcast;
                broadcast.set_ret(1);
                broadcast.set_uid(req.uid());
                std::string serialized_broadcast;
                broadcast.SerializeToString(&serialized_broadcast);
                broadcastNotify(ID_L2C_NotifyEnterWorld, serialized_broadcast);
            break;
        }
    }

    

    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];

    std::vector<char> bytes_buffer_;
    int status_ = 0;
    Message message_;
    Server *server_;
};

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

public:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    printf("connect accepted\n");
                    std::shared_ptr<Session> ses = std::make_shared<Session>(std::move(socket));
                    ses->server_ = this;
                    ses->start();
                    sessions_.push_back(ses);
                }
                do_accept();
            });
    }

    void broadcastNotify(char* data, int length) 
    {
        for (int i = 0; i < sessions_.size(); ++i)
        {
            sessions_[i]->write(data, length);
        }
    }

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
};

void tempfunc(Server* server, const std::string& str)
{
    server->broadcastNotify(const_cast<char*>(str.c_str()), str.size());
}

void clientTest();

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        boost::asio::io_context io_context;
        Server server(io_context, 12345);
        io_context.run();
    }
    else
    {
        clientTest();
    }
    return 0;
}

