//
// Created by taganyer on 25-5-24.
//
#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif

#include <tdcf/base/Marcos.hpp>
#include <tdcf/detail/Command.hpp>

namespace tdcf {

    enum class CommandMarkTypes : unsigned {
        Null,
        Broadcast,
        Scatter,
        Reduce,
        AllGather,
        AllReduce,
        ReduceScatter,
        AllToAll,
        Supplement,
    };

    class CommandMark : public Command {
    public:
        static StatusFlag create(const void *data, unsigned data_size, SerializablePtr& buf) {
            if (data_size < 4) return StatusFlag::FurtherWaiting;
            auto ptr = static_cast<const unsigned *>(data);
            buf = std::make_shared<CommandMark>(ntohl(ptr[0]),
                                                static_cast<CommandMarkTypes>(ntohl(ptr[1])),
                                                ntohl(ptr[2]), ntohl(ptr[3]));
            return StatusFlag::Success;
        }

        CommandMark() = default;

        CommandMark(const CommandMark&) = default;

        explicit CommandMark(unsigned version) : _version(version) {};

        explicit CommandMark(unsigned version, CommandMarkTypes type,
                             unsigned command_count, unsigned data_count) :
            _version(version), type(type), followed_commands(command_count), followed_datas(data_count) {};

        CommandMark& operator=(const CommandMark&) = default;

        [[nodiscard]] unsigned serialize_size() const override {
            return sizeof(unsigned) * 4;
        };

        void serialize(void *buffer) const override {
            auto ptr = static_cast<unsigned *>(buffer);
            ptr[0] = htonl(_version);
            ptr[1] = htonl(static_cast<unsigned>(type));
            ptr[2] = htonl(followed_commands);
            ptr[3] = htonl(followed_datas);
        };

        [[nodiscard]] SerializableType derived_type() const override {
            return static_cast<SerializableType>(CommandTypes::CommandMark);
        };

        [[nodiscard]] unsigned data() const { return _version; };

        CommandMark& operator++() {
            if (likely(_version != UINT32_MAX)) ++_version;
            else _version = 0;
            return *this;
        };

        CommandMark operator++(int) {
            CommandMark temp = *this;
            if (likely(_version != UINT32_MAX)) ++_version;
            else _version = 0;
            return temp;
        };

        CommandMark& operator+=(unsigned _step) {
            unsigned res = UINT32_MAX - _version;
            if (likely(_step < res)) _version += _step;
            else _version = _step - res - 1;
            return *this;
        };

        friend bool operator==(const CommandMark& left, const CommandMark& right) {
            return left._version == right._version;
        };

        friend bool operator!=(const CommandMark& left, const CommandMark& right) {
            return left._version != right._version;
        };

        friend bool operator<(const CommandMark& left, const CommandMark& right) {
            if (left._version < right._version)
                return right._version - left._version <= UINT32_MAX / 2;
            return left._version - right._version > UINT32_MAX / 2;
        };

        friend bool operator>(const CommandMark& left, const CommandMark& right) {
            if (left._version > right._version)
                return left._version - right._version <= UINT32_MAX / 2;
            return right._version - left._version > UINT32_MAX / 2;
        };

        friend bool operator<=(const CommandMark& left, const CommandMark& right) {
            return left == right || left < right;
        };

        friend bool operator>=(const CommandMark& left, const CommandMark& right) {
            return left == right || left > right;
        };

    private:
        unsigned _version = 0;

    public:
        CommandMarkTypes type = CommandMarkTypes::Null;

        unsigned followed_commands = 0;

        unsigned followed_datas = 0;

    };

}
