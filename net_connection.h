#ifndef NET_CONNECTION_
#define NET_CONNECTION_

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include <asio/connect.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp>
#include <bits/c++config.h>
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <memory>

namespace net {
    template<typename T>
    class Connection : public std::enable_shared_from_this<Connection<T>> {
        public:
        enum class Owner {
            server,
            client,
        };

        Connection(Owner parent, asio::io_context& asio_context, asio::ip::tcp::socket socket, TsQueue<OwnedMessage<T>>& q_in)
            : asio_context_(asio_context), socket_(std::move(socket)), message_in_(q_in), owner_type_(parent) {}
        
        virtual ~Connection() {}

        uint32_t GetId() const {
            return id_;
        }

        void ConnectToClient(uint32_t uid = 0) {
            if (owner_type_ == Owner::server) {
                id_ = uid;
                ReadHeader();
            }
        }

        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
            if (owner_type_ == Owner::client) {
                asio::async_connect(socket_, endpoints, 
                [this](std::error_code ec, asio::ip::tcp::endpoint){
                    if (!ec) {
                        ReadHeader();
                    }
                });
            }
        }

        void Disconnect() {
            if (IsConnected())
                asio::post(asio_context_, [this]() {socket_.close(); });
        }

        bool IsConnected() {
            return socket_.is_open();
        }

        void StartListening() {
            
        }

        private:

        void ReadHeader() {
            asio::async_read(socket_, asio::buffer(&msg_tmp_in_.header, sizeof(MessageHeader<T>)),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        if (msg_tmp_in_.header.size > 0) {
                            msg_tmp_in_.body.resize(msg_tmp_in_.header.size);
                            ReadBody();
                        }
                        else {
                            AddToIncomingMessageQueue();
                        }
                    }
                    else {
                        std::cout << "[" << id_ << "] Read Header Fail.\n";
						socket_.close();
                    }
                });
        }

        void ReadBody() {
            asio::async_read(socket_, asio::buffer(msg_tmp_in_.body.data(), msg_tmp_in_.body.size()),
                [this](std::error_code ec, std::size_t length){
                    if (!ec) {
                        AddToIncomingMessageQueue();
                    }
                    else {
                        std::cout << "[" << id_ << "] Read Body Fail.\n";
                        socket_.close();
                    }
                });
        }

        void AddToIncomingMessageQueue() {
            if (owner_type_ == Owner::server) {
                message_in_.PushBack({this->shared_from_this(), msg_tmp_in_});
            }
            else {
                message_in_.PushBack({nullptr, msg_tmp_in_});
            }
            ReadHeader();
        }

        protected:
        asio::ip::tcp::socket socket_;
        asio::io_context& asio_context_;
        TsQueue<Message<T>> message_out_;
        TsQueue<OwnedMessage<T>> message_in_;
        Message<T> msg_tmp_in_;
        Owner owner_type_ = Owner::server;

        std::uint32_t id_ = 0;
    };
}

#endif // NET_CONNECTION_