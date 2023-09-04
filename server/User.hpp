#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <thread>
#include <utility>
#include <vector>
#include <cstring>

#define ALLOWED_WRONG_GUESSES 7

#ifndef PROJ2SOCKETCPP_USER_H
#define PROJ2SOCKETCPP_USER_H

using std::string;

class User
{
public:
    User(uint8_t id, string password, struct sockaddr_in address, ushort userSessionFD) : m_id{id}, m_password{std::move(password)}, m_address{address}, m_userSessionFD{userSessionFD} {}
    uint8_t get_id() const { return m_id; };
    uint8_t get_remaining_guesses() const { return m_allowedGuesses; };
    uint8_t decrement_allowed_guesses() { return --m_allowedGuesses; };
    string get_password() const { return m_password; };
    const struct sockaddr_in m_address;
    const ushort m_userSessionFD;
private:
    uint8_t m_allowedGuesses {ALLOWED_WRONG_GUESSES};
    const uint8_t m_id;
    const string m_password;
};

#endif //PROJ2SOCKETCPP_USER_H
