#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include "player.pb.h"
#include "cl.pb.h"

using boost::asio::ip::tcp;

class Message
{
public:
    char *Convert2Bytes()
    {
        char *buf = new char(Message::package_head_size + length_);
        char c1 = length_ >> 24;
        char c2 = length_ >> 16;
        char c3 = length_ >> 8;
        char c4 = length_;
        buf[0] = c1;
        buf[1] = c2;
        buf[2] = c3;
        buf[3] = c4;

        c1 = msg_type_ >> 24;
        c2 = msg_type_ >> 16;
        c3 = msg_type_ >> 8;
        c4 = msg_type_;
        buf[4] = c1;
        buf[5] = c2;
        buf[6] = c3;
        buf[7] = c4;

        c1 = sequence_ >> 24;
        c2 = sequence_ >> 16;
        c3 = sequence_ >> 8;
        c4 = sequence_;
        buf[8] = c1;
        buf[9] = c2;
        buf[10] = c3;
        buf[11] = c4;

        c1 = flags_ >> 24;
        c2 = flags_ >> 16;
        c3 = flags_ >> 8;
        c4 = flags_;
        buf[12] = c1;
        buf[13] = c2;
        buf[14] = c3;
        buf[15] = c4;

        c1 = reserved_ >> 24;
        c2 = reserved_ >> 16;
        c3 = reserved_ >> 8;
        c4 = reserved_;
        buf[16] = c1;
        buf[17] = c2;
        buf[18] = c3;
        buf[19] = c4;

        for (int i = 0; i < body_.size(); ++i)
        {
            buf[20+i] = body_[i];
        }
        return buf;
    }

    char* BodyToBytes()
    {
        char* data = new char(body_.size());
        for (int i = 0; i < body_.size(); ++i)
        {
            data[i] = body_[i];
        }
        return data;
    }

public:
    static const uint32_t package_head_size = 20;

    uint32_t length_;
    uint32_t msg_type_;
    uint32_t sequence_;
    uint32_t flags_;
    uint32_t reserved_;

    std::vector<char> body_;
};

class Server;
void tempfunc(Server *server, char* data, int length);


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

    void response(uint32_t msgType, char*data, int length)
    {
        Message rspMsg;
        rspMsg.length_ = length + Message::package_head_size;
        rspMsg.msg_type_ = msgType;
        for (int i = 0; i < length; ++i)
        {
            rspMsg.body_.push_back(data[i]);
        }
        char* bytes = rspMsg.Convert2Bytes();

        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(bytes, rspMsg.length_),
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

    void broadcastNotify(uint32_t msgType, char*data, int length)
    {
         Message rspMsg;
        rspMsg.length_ = length + Message::package_head_size;
        rspMsg.msg_type_ = msgType;
        for (int i = 0; i < length; ++i)
        {
            rspMsg.body_.push_back(data[i]);
        }
        char* bytes = rspMsg.Convert2Bytes();
        
        tempfunc(server_, data, length);
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
                    uint32_t length = c4 << 24 | c3 << 16 | c2 << 8 | c1;

                    c1 = bytes_buffer_[4];
                    c2 = bytes_buffer_[5];
                    c3 = bytes_buffer_[6];
                    c4 = bytes_buffer_[7];
                    uint32_t msg_type = c4 << 24 | c3 << 16 | c2 << 8 | c1;

                    c1 = bytes_buffer_[8];
                    c2 = bytes_buffer_[9];
                    c3 = bytes_buffer_[10];
                    c4 = bytes_buffer_[11];
                    uint32_t sequence = c4 << 24 | c3 << 16 | c2 << 8 | c1;

                    c1 = bytes_buffer_[12];
                    c2 = bytes_buffer_[13];
                    c3 = bytes_buffer_[14];
                    c4 = bytes_buffer_[15];
                    uint32_t flags = c4 << 24 | c3 << 16 | c2 << 8 | c1;

                    c1 = bytes_buffer_[16];
                    c2 = bytes_buffer_[17];
                    c3 = bytes_buffer_[18];
                    c4 = bytes_buffer_[19];
                    uint32_t reserved = c4 << 24 | c3 << 16 | c2 << 8 | c1;

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
        switch (message_.msg_type_)
        {
            case ID_C2L_EnterWorld:
                C2L_EnterWorld req;
                req.ParseFromArray(message_.BodyToBytes(), message_.length_);
                L2C_EnterWorld rsp;
                rsp.set_ret(1);
                rsp.set_uid(req.uid());
                char * rspbytes = new char(rsp.ByteSize());
                rsp.SerializeToArray(rspbytes, rsp.ByteSize());
                response(ID_L2C_EnterWorld, rspbytes, rsp.ByteSize());

                L2C_NotifyEnterWorld broadcast;
                broadcast.set_ret(1);
                broadcast.set_uid(req.uid());
                char* broadcastBytes = new char(broadcast.ByteSize());
                broadcast.SerializeToArray(broadcastBytes, broadcast.ByteSize());
                broadcastNotify(ID_L2C_NotifyEnterWorld, broadcastBytes, broadcast.ByteSize());
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
    int status_;
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

void tempfunc(Server *server, char* data, int length)
{
    server->broadcastNotify(data, length);
}

int main(int argc, char **argv)
{
    boost::asio::io_context io_context;
    Server server(io_context, 12345);
    io_context.run();

    return 0;
}