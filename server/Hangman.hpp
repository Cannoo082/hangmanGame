#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <thread>
#include <utility>
#include <vector>
#include <cstring>

#include "User.hpp"

#ifndef PROJ2SOCKETCPP_HANGMAN_H
#define PROJ2SOCKETCPP_HANGMAN_H

using std::string;
using std::vector;

class Hangman
{
public:
    Hangman(string word, std::vector<User*>& players);
    string get_already_guessed() const;
    string get_guessed_part() const { return m_guessedPart; };
    uint8_t increment_round() { return ++m_round; };
    bool process_guessed_letter(char letter, int socket_fd);
    void update_guessed_letters(char letter);
    vector<User*>& get_players() { return m_players; };
private:
    vector<User*>& m_players;
    uint8_t m_round {};
    string m_guessedPart;
    const string m_word;
    char m_alreadyGuessed[26];
    const size_t m_letterCount;
    // functions
    void update_guessed_part(char letter);
};


#endif //PROJ2SOCKETCPP_HANGMAN_H