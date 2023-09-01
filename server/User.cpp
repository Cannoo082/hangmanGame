#include <iostream>
#include <netinet/in.h>


#include "User.hpp"

User::User(uint8_t id, std::string password, struct sockaddr_in address, ushort userSessionFD) : m_id{id}, m_password{password}, m_address{address}, m_userSessionFD{userSessionFD} {}
