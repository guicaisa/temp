#include <boost/asio.hpp>
#include <string>
#include <vector>
#include "message.h"
#include "cl.pb.h"
#include <iostream>

using boost::asio::ip::tcp;

class Client
{
public:
	Client(boost::asio::io_context& io_context, const std::string& ip, const std::string& port)
		: socket_(io_context)
	{
		tcp::resolver resolver(io_context);
		auto end_points = resolver.resolve(ip, port);
		boost::asio::connect(socket_, end_points);
        status_ = 0;
	}

	void asyncRead()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length), [this](boost::system::error_code ec, std::size_t length)
        {
			if (!ec)
			{
                cacheReadDatas(length);
                asyncRead();
			}
		});
	}

    void write(char* data, int length)
    {
        boost::asio::async_write(socket_, boost::asio::buffer(data, length),
            [this](boost::system::error_code ec, std::size_t /*length*/)
            {

            });
    }

    void sendMsg(uint32_t msgType, const std::string& str)
    {
        Message reqMsg;
        reqMsg.length_ = str.size() + Message::package_head_size;
        reqMsg.msg_type_ = msgType;
        for (int i = 0; i < str.size(); ++i)
        {
            reqMsg.body_.push_back(str[i]);
        }
        char* bytes = reqMsg.Convert2Bytes();
        write(bytes, reqMsg.length_);
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
                }
                else
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
        case ID_L2C_EnterWorld:
            printf("ID_L2C_EnterWorld\n");
            break;
        case ID_L2C_NotifyEnterWorld:
            printf("ID_L2C_NotifyEnterWorld\n");
            break;
        }
    }
	
private:
	enum
	{
		max_length = 1024
	};
	char data_[max_length];
	std::vector<char> bytes_buffer_;
	int status_;
    Message message_;

	tcp::socket socket_;
};

void clientTest()
{
	boost::asio::io_context io_context;
	std::string ip = "127.0.0.1";
	std::string port = "12345";

	Client client(io_context, ip, port);

	std::thread t([&](){
		client.asyncRead();
		io_context.run();
	});

    while (1)
    {
        int num = 0;
        std::cin >> num;
        if (num == 0)
        {
            break;
        }
        io_context.post([&client = client, num = num]()
            {
                C2L_EnterWorld req;
                req.set_uid(123);
                std::string serialized_data;
                req.SerializeToString(&serialized_data);
                client.sendMsg(ID_C2L_EnterWorld, serialized_data);
            });
    }

    t.join();
}