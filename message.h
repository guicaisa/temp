#include <stdint.h>
#include <vector>

class Message
{
public:
    ~Message()
    {
        //body_.clear();
    }

    std::string  Convert2Str()
    {
        std::string str(length_, 0);
        char c1 = length_ >> 24;
        char c2 = length_ >> 16;
        char c3 = length_ >> 8;
        char c4 = length_;
        str[0] = c1;
        str[1] = c2;
        str[2] = c3;
        str[3] = c4;

        c1 = msg_type_ >> 24;
        c2 = msg_type_ >> 16;
        c3 = msg_type_ >> 8;
        c4 = msg_type_;
        str[4] = c1;
        str[5] = c2;
        str[6] = c3;
        str[7] = c4;

        c1 = sequence_ >> 24;
        c2 = sequence_ >> 16;
        c3 = sequence_ >> 8;
        c4 = sequence_;
        str[8] = c1;
        str[9] = c2;
        str[10] = c3;
        str[11] = c4;

        c1 = flags_ >> 24;
        c2 = flags_ >> 16;
        c3 = flags_ >> 8;
        c4 = flags_;
        str[12] = c1;
        str[13] = c2;
        str[14] = c3;
        str[15] = c4;

        c1 = reserved_ >> 24;
        c2 = reserved_ >> 16;
        c3 = reserved_ >> 8;
        c4 = reserved_;
        str[16] = c1;
        str[17] = c2;
        str[18] = c3;
        str[19] = c4;

        for (int i = 0; i < body_.size(); ++i)
        {
            str[20 + i] = body_[i];
        }
        return str;
    }

    char* Convert2Bytes()
    {
        char* buf = new char(length_);
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
            buf[20 + i] = body_[i];
        }
        return buf;
    }
   
    void Package(std::shared_ptr<std::vector<char>> pack)
    {
        char c1 = length_ >> 24;
        char c2 = length_ >> 16;
        char c3 = length_ >> 8;
        char c4 = length_;
        pack->push_back(c1);
        pack->push_back(c1);
        pack->push_back(c2);
        pack->push_back(c3);
        pack->push_back(c4);

        c1 = msg_type_ >> 24;
        c2 = msg_type_ >> 16;
        c3 = msg_type_ >> 8;
        c4 = msg_type_;
        pack->push_back(c1);
        pack->push_back(c2);
        pack->push_back(c3);
        pack->push_back(c4);

        c1 = sequence_ >> 24;
        c2 = sequence_ >> 16;
        c3 = sequence_ >> 8;
        c4 = sequence_;
        pack->push_back(c1);
        pack->push_back(c2);
        pack->push_back(c3);
        pack->push_back(c4);

        c1 = flags_ >> 24;
        c2 = flags_ >> 16;
        c3 = flags_ >> 8;
        c4 = flags_;
        pack->push_back(c1);
        pack->push_back(c2);
        pack->push_back(c3);
        pack->push_back(c4);

        c1 = reserved_ >> 24;
        c2 = reserved_ >> 16;
        c3 = reserved_ >> 8;
        c4 = reserved_;
        pack->push_back(c1);
        pack->push_back(c2);
        pack->push_back(c3);
        pack->push_back(c4);

        for (int i = 0; i < body_.size(); ++i)
        {
            pack->push_back(body_[i]);
        }
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

    std::string BodyToString()
    {
        std::string str;
        for (int i = 0; i < body_.size(); ++i)
        {
            str += body_[i];
        }
        return str;
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