#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include "player.pb.h"
#include "cl.pb.h"
#include "message.h"
#ifdef WIN32
#include "mysql.h"
#else
#include <mysql/mysql.h>
#endif

using boost::asio::ip::tcp;

class Server;

void tempfunc(Server *server, uint32_t msg_type, const std::string& str);
int tempCreateAccount(Server* server, const std::string& account, const std::string& password, uint64_t* uid);
int tempLogin(Server* server, const std::string& account, const std::string& password, uint64_t* uid);
std::vector<PlayerState> tempGetPlayerStates(Server* server);


class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket) : socket_(std::move(socket)) 
    {
        uid_ = 0;
        speed_ = 0.0f;
        is_move_ = false;
        is_in_world_ = false;
        pos_.set_x(0);
        pos_.set_y(0);
        pos_.set_z(0);
        direciton_.set_x(0);
        direciton_.set_y(0);
        direciton_.set_z(0);
    }

    void start()
    {
        do_read();
    }

    void update()
    {
        if (!is_in_world_ || !is_move_)
        {
            return;
        }
    }

    bool isInWorld()
    {
        return is_in_world_;
    }

    uint64_t uid() { return uid_; }
    const Vector3& pos() { return pos_; }
    const Vector3& direction() { return direciton_; }
    float speed() { return speed_; }
    bool isMove() { return is_move_; }

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
        rspMsg.body_ = std::vector<char>(str.size(), 0);
        for (int i = 0; i < str.size(); ++i)
        {
            rspMsg.body_[i] = str[i];
        }
        std::string sendStr = rspMsg.Convert2Str();

        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(sendStr.c_str(), sendStr.size()),
                                 [this, self, msgType = msgType](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                    if (!ec)
                                    {
                                        printf("write %d success\n", msgType);
                                    }
                                    else 
                                    {
                                        printf("write %d error\n", msgType);
                                    }
                                 });
    }

    void write(uint32_t msg_type, const std::string str)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(str.c_str(), str.size()),
                                 [this, self, msgType = msg_type](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                    if (!ec)
                                    {
                                        printf("write %d success\n", msgType);
                                    }
                                    else
                                    {
                                        printf("write %d error\n", msgType);
                                    }
                                 });
    }

    void broadcastNotify(uint32_t msgType, const std::string& str)
    {
        Message rspMsg;
        rspMsg.length_ = str.size() + Message::package_head_size;
        rspMsg.msg_type_ = msgType;
        rspMsg.body_ = std::vector<char>(str.size(), 0);
        for (int i = 0; i < str.size(); ++i)
        {
            rspMsg.body_[i] = str[i];
        }
        std::string sendStr = rspMsg.Convert2Str();

        tempfunc(server_, msgType, sendStr);
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
                else
                {
                    break_loop = true;
                }
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
            {
                C2L_EnterWorld req;
                req.ParseFromString(message_.BodyToString());
                printf("C2L_EnterWorld: uid: %d, pos: x: %f, y: %f, z: %f\n", req.uid(), req.pos().x(), req.pos().y(), req.pos().z());

                L2C_EnterWorld rsp;
                rsp.set_ret(1);
                rsp.set_uid(req.uid());
                rsp.mutable_pos()->CopyFrom(req.pos());
                std::string serialized_data_rsp;
                rsp.SerializeToString(&serialized_data_rsp);
                response(ID_L2C_EnterWorld, serialized_data_rsp);

                L2C_NotifyEnterWorld broadcast;
                broadcast.set_ret(1);
                broadcast.set_uid(req.uid());
                broadcast.mutable_pos()->CopyFrom(req.pos());
                std::string serialized_broadcast;
                broadcast.SerializeToString(&serialized_broadcast);
                broadcastNotify(ID_L2C_NotifyEnterWorld, serialized_broadcast);

                pos_.set_x(req.pos().x());
                pos_.set_y(req.pos().y());
                pos_.set_z(req.pos().z());
                is_in_world_ = true;

                std::vector<PlayerState> states = tempGetPlayerStates(server_);
                if (states.size() != 0)
                {
                    L2C_NotifyPlayeStates n_player_state;
                    for (int i = 0; i < states.size(); ++i)
                    {
                        PlayerState* temp = n_player_state.add_player_states();
                        *temp = states[i];
                    }
                    std::string serialized_notify;
                    n_player_state.SerializeToString(&serialized_notify);
                    response(ID_L2C_NotifyPlayeStates, serialized_notify);
                }
                break;
            }
            
            case ID_C2L_Move:
            {
                C2L_Move cMove;
                cMove.ParseFromString(message_.BodyToString());
                printf("C2L_Move: uid: %d, speed: %f, pos: x: %f, y: %f, z: %f\n", cMove.uid(), cMove.speed(), cMove.direction().x(), cMove.direction().y(), cMove.direction().z());


                L2C_Move sMove;
                sMove.set_ret(1);
                sMove.set_uid(cMove.uid());
                sMove.set_speed(cMove.speed());
                sMove.mutable_direction()->set_x(cMove.direction().x());
                sMove.mutable_direction()->set_y(cMove.direction().y());
                sMove.mutable_direction()->set_z(cMove.direction().z());
                sMove.mutable_pos()->set_x(cMove.pos().x());
                sMove.mutable_pos()->set_y(cMove.pos().y());
                sMove.mutable_pos()->set_z(cMove.pos().z());
                std::string serialized_data_move;
                sMove.SerializeToString(&serialized_data_move);
                response(ID_L2C_Move, serialized_data_move);

                L2C_NotifyMove broadcast_move;
                broadcast_move.set_ret(1);
                broadcast_move.set_uid(cMove.uid());
                broadcast_move.set_speed(cMove.speed());
                broadcast_move.mutable_direction()->set_x(cMove.direction().x());
                broadcast_move.mutable_direction()->set_y(cMove.direction().y());
                broadcast_move.mutable_direction()->set_z(cMove.direction().z());
                broadcast_move.mutable_pos()->set_x(cMove.pos().x());
                broadcast_move.mutable_pos()->set_y(cMove.pos().y());
                broadcast_move.mutable_pos()->set_z(cMove.pos().z());
                std::string serialized_broadcast_move;
                broadcast_move.SerializeToString(&serialized_broadcast_move);
                broadcastNotify(ID_L2C_NotifyMove, serialized_broadcast_move);

                is_move_ = true;
                speed_ = cMove.speed();
                direciton_.set_x(cMove.direction().x());
                direciton_.set_y(cMove.direction().y());
                direciton_.set_z(cMove.direction().z());
                pos_.set_x(cMove.pos().x());
                pos_.set_y(cMove.pos().y());
                pos_.set_z(cMove.pos().z());
                break;
            }

            case ID_C2L_StopMove:
            {
                C2L_StopMove req;
                req.ParseFromString(message_.BodyToString());
                printf("C2L_StopMove: uid: %d\n", req.uid());

                L2C_StopMove rsp;
                rsp.set_ret(1);
                rsp.set_uid(req.uid());
                std::string serialized_data;
                rsp.SerializeToString(&serialized_data);
                response(ID_L2C_StopMove, serialized_data);

                L2C_NotifyStopMove broadcast;
                broadcast.set_ret(1);
                broadcast.set_uid(req.uid());
                std::string serialized_broadcast;
                broadcast.SerializeToString(&serialized_broadcast);
                broadcastNotify(ID_L2C_NotifyStopMove, serialized_broadcast);

                is_move_ = false;
                break;
            }

            case ID_C2L_CreateAccount:
            {
                C2L_CreateAccount req;
                req.ParseFromString(message_.BodyToString());
                printf("C2L_CreateAccount: account: %s, password: %s\n", req.account().c_str(), req.password().c_str());

                uint64_t uid = 0;
                int ret = tempCreateAccount(server_, req.account(), req.password(), &uid);
                L2C_CreateAccount rsp;
                rsp.set_ret(ret);
                rsp.set_account(req.account());
                rsp.set_password(req.password());
                rsp.set_uid(uid);
                std::string serialized_data;
                rsp.SerializeToString(&serialized_data);
                response(ID_L2C_CreateAccount, serialized_data);
                break;
            }

            case ID_C2L_Login:
            {
                C2L_Login req;
                req.ParseFromString(message_.BodyToString());
                printf("C2L_Login: account: %s, password: %s\n", req.account().c_str(), req.password().c_str());

                L2C_Login rsp;
                rsp.set_account(req.account());
                rsp.set_password(req.password());
                if (uid_ != 0)
                {
                    rsp.set_ret(1);
                    rsp.set_uid(uid_);
                }
                else
                {
                    uint64_t uid = 0;
                    int ret = tempLogin(server_, req.account(), req.password(), &uid);
                    rsp.set_ret(ret);
                    rsp.set_uid(uid);
                }
                std::string serialized_data;
                rsp.SerializeToString(&serialized_data);
                response(ID_L2C_Login, serialized_data);
                break;
            }

            case ID_C2L_LeaveWorld:
            {
                C2L_LeaveWorld req;
                req.ParseFromString(message_.BodyToString());
                printf("C2L_LeaveWorld: uid: %ld\n", req.uid());

                L2C_LeaveWorld rsp;
                rsp.set_ret(1);
                rsp.set_uid(req.uid());
                std::string serialized_data;
                rsp.SerializeToString(&serialized_data);
                response(ID_L2C_LeaveWorld, serialized_data);

                L2C_NotifyLeaveWorld broadcast;
                broadcast.set_ret(1);
                broadcast.set_uid(req.uid());
                std::string serialized_broadcast;
                broadcast.SerializeToString(&serialized_broadcast);
                broadcastNotify(ID_L2C_NotifyLeaveWorld, serialized_broadcast);

                is_move_ = false;
                is_in_world_ = false;
                break;
            }
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

    uint64_t uid_;
    Vector3 pos_;
    Vector3 direciton_;
    float speed_;
    bool is_move_;
    bool is_in_world_;
};

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), timer_(io_context, boost::asio::chrono::seconds(1))
    {
        count_ = 0;
        conn_ = mysql_init(NULL);
        if (conn_ == NULL) {
            fprintf(stderr, "mysql_init() failed\n");
        }

        // 连接到 MySQL 数据库
        if (mysql_real_connect(conn_, "127.0.0.1", "ciao", "123456", "testdb", 3306, NULL, 0) == NULL) {
            fprintf(stderr, "mysql_real_connect() failed, %s\n", mysql_errno(conn_));
            mysql_close(conn_);
        }

        do_accept();
        do_timer();
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

    void do_timer()
    {
        timer_.async_wait([&](const boost::system::error_code& ec) {
            if (!ec)
            {
                update();
                timer_.expires_after(boost::asio::chrono::seconds(1));  
                do_timer();
            }
            });
    }

    void update()
    {
        //printf("server update %d\n", count_);
        ++count_;
        if (sessions_.size() == 0)
        {
            return;
        }
        for (int i = 0; i < sessions_.size(); ++i)
        {
            sessions_[i]->update();
        }
        std::vector<PlayerState> states = getPlayerStates();
        if (states.size() == 0)
        {
            return;
        }
        L2C_NotifyPlayeStates n_player_state;
        for (int i = 0; i < states.size(); ++i)
        {
            PlayerState* temp = n_player_state.add_player_states();
            *temp = states[i];
        }
        std::string serialized_notify;
        n_player_state.SerializeToString(&serialized_notify);

        Message rspMsg;
        rspMsg.length_ = serialized_notify.size() + Message::package_head_size;
        rspMsg.msg_type_ = ID_L2C_NotifyPlayeStates;
        rspMsg.body_ = std::vector<char>(serialized_notify.size(), 0);
        for (int i = 0; i < serialized_notify.size(); ++i)
        {
            rspMsg.body_[i] = serialized_notify[i];
        }
        std::string sendStr = rspMsg.Convert2Str();
        broadcastNotify(rspMsg.msg_type_, sendStr);
    }

    void broadcastNotify(uint32_t msg_type, const std::string str) 
    {
        for (int i = 0; i < sessions_.size(); ++i)
        {
            sessions_[i]->write(msg_type, str);
        }
    }

    int createAccount(const std::string& account, const std::string& password, uint64_t* uid)
    {
        std::string query = "INSERT INTO account (account, password) VALUES ('" + account + "', '" + password + "')";
        if (mysql_query(conn_, query.c_str())) {
            fprintf(stderr, "INSERT INTO account failed. Error: %s\n", mysql_error(conn_));
            mysql_close(conn_);
            return 123;
        }

       *uid = mysql_insert_id(conn_);

        return 1;
    }

    int login(const std::string& account, const std::string& password, uint64_t* uid)
    {
        std::string query= "select id from account where account = '" + account + "' and password = '" + password + "'";
        if (mysql_query(conn_, query.c_str())) {
            fprintf(stderr, "select from account failed. Error: %s\n", mysql_error(conn_));
            mysql_close(conn_);
            return 123;
        }

        MYSQL_RES* res = mysql_store_result(conn_);
        if (res == NULL) 
        {
            fprintf(stderr, "mysql_store_result() failed. Error: %s\n", mysql_error(conn_));
            mysql_close(conn_);
            return 123;
        }

        MYSQL_ROW row;
        int num_fields = mysql_num_fields(res);
        while ((row = mysql_fetch_row(res))) 
        {
            for (int i = 0; i < num_fields; i++) 
            {
                *uid = atoi(row[i]);
            }
        }

        mysql_free_result(res);
        return 1;
    }

    std::vector<PlayerState> getPlayerStates()
    {
        std::vector<PlayerState> player_states;
        for (int i = 0; i < sessions_.size(); ++i)
        {
            std::shared_ptr<Session> ses = sessions_[i];
            if (ses->isInWorld())
            {
                PlayerState state;
                state.set_uid(ses->uid());
                state.set_speed(ses->speed());
                state.set_is_move(ses->isMove());
                state.mutable_pos()->set_x(ses->pos().x());
                state.mutable_pos()->set_y(ses->pos().y());
                state.mutable_pos()->set_z(ses->pos().z());
                state.mutable_direction()->set_x(ses->direction().x());
                state.mutable_direction()->set_y(ses->direction().y());
                state.mutable_direction()->set_z(ses->direction().z());
                player_states.push_back(state);
            }
        }
        return player_states;
    }

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
    MYSQL* conn_;
    boost::asio::steady_timer timer_;
    int count_;
};

void tempfunc(Server* server, uint32_t msg_type, const std::string& str)
{
    server->broadcastNotify(msg_type, str);
}

int tempCreateAccount(Server* server, const std::string& account, const std::string& password, uint64_t* uid)
{
    return server->createAccount(account, password, uid);
}

int tempLogin(Server* server, const std::string& account, const std::string& password, uint64_t* uid)
{
    return server->login(account, password, uid);
}

std::vector<PlayerState> tempGetPlayerStates(Server* server)
{
    return server->getPlayerStates();
}

void clientTest(int uid);


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
        int uid = atoi(argv[1]);
        clientTest(uid);
    }
    return 0;
}

