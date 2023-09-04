#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <thread>
#include <vector>
#include <cstring>

#include "User.hpp"
#include "Hangman.hpp"


#define LOCALHOST "127.0.0.1"
#define MAX_BACKLOG 10
#define LISTENING_PORT 9191
#define BUFFER_SIZE 1024
// Server Connection Messages:
#define BIND_SUCCESSFUL "Bind Successful"
#define BIND_FAILED "Bind Failed, Error: "
#define LISTENING_SUCCESSFUL "Listening Successful"
#define LISTENING_FAILED "Listening Failed, Error: "
#define CLIENT_CONNECTION_FAILED "Client Connection Failed, Error: "
#define LISTEN "Listening on port: "

// Client Connection Messages:
#define WELCOME_MSG "Welcome to the Hangman game! "
#define WAITING_FOR_OTHER_PLAYERS "Waiting for other players to connect..."
#define ASK_PASSW "Please input your password: "

// ASK FOR WORD
#define ASK_FOR_WORD " Please enter a word to play hangman! "
// START GAME
#define START_GAME "Starting game with word: "

// INPUT VALIDATION
#define ONLY_ONE_LETTER "Please enter only one letter"
#define NOT_A_LETTER "Please enter a valid letter"
#define ALREADY_GUESSED "This letter is already guessed"

// PLAYER COUNT
#define INVALID_PLAYER_COUNT "Invalid number of players. "
#define ASK_PLAYER_COUNT "Please input the number of players: "

// GAME MESSAGES
#define YOUR_TURN "It's your turn to guess a letter!"
#define CORRECT_GUESS "Correct guess!"
#define INCORRECT_GUESS "Incorrect guess!"
#define ALREADY_GUESSED_LETTERS "Already guessed letters: "
#define REMAINING_GUESS_COUNT "Remaining wrong guess count: "
#define ASK_LETTER "Please enter a letter: "
#define WAITING_FOR_OTHERS "Waiting for other players to guess"
#define REACHED_PLAYER_COUNT "Reached player count"
#define GAME_FULL "Game is full"
#define PLEASE_WAIT "Please wait..."
#define GAME_OVER "Game Over!"
#define ELIMINATED "You have been eliminated!"



using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::to_string;


std::string askForPassword(int clientSocket, char* buffer)
{
    send(clientSocket, ASK_PASSW, sizeof(ASK_PASSW), 0);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    return std::string{buffer};
}

void getPlayerCount(int clientSocket, char* buffer, int& player_count)
{
    send(clientSocket, WELCOME_MSG ASK_PLAYER_COUNT, sizeof(WELCOME_MSG ASK_PLAYER_COUNT), 0);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    // check if valid number
    player_count = atoi(buffer);
    while(player_count < 2)
    {
        send (clientSocket, INVALID_PLAYER_COUNT ASK_PLAYER_COUNT, sizeof(INVALID_PLAYER_COUNT ASK_PLAYER_COUNT), 0);
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        player_count = atoi(buffer);
    }
}

void printRegisteredUsers(std::vector<User*>& users)
{
    cout << "Registered Users: " << endl;
    for (auto& user : users)
    {
        cout << "User " << user->get_id() << " " << inet_ntoa(user->m_address.sin_addr) << ":" << ntohs(user->m_address.sin_port) << " " << user->m_userSessionFD << " " << user->get_password() << endl;
    }
}

void acceptIncoming(std::vector<User*>& users, char* buffer, int& player_count, int& current_user_count, struct sockaddr_in clientAddress, int clientSocket)
{
    if(player_count > 0 && current_user_count == player_count)
    {
        send(clientSocket, GAME_FULL, sizeof(GAME_FULL), 0);
        close(clientSocket);
        return;
    }

    cout << "Client " << ++current_user_count <<" Connected" << endl;

    if (current_user_count == 1)
    {
        //ask for password and # of players
        getPlayerCount(clientSocket, buffer, player_count);
        User* newUser = new User(0, askForPassword(clientSocket, buffer), clientAddress, clientSocket);
        users.push_back(newUser);
    }
    else
    {
        User* newUser2 = new User((current_user_count - 1), askForPassword(clientSocket, buffer), clientAddress, clientSocket);
        users.push_back(newUser2);
        send(clientSocket, PLEASE_WAIT, sizeof(PLEASE_WAIT), 0);
    }

    if (player_count != current_user_count)
    {
        send(clientSocket, WAITING_FOR_OTHER_PLAYERS, sizeof(WAITING_FOR_OTHER_PLAYERS), 0);
    }

    if (player_count == current_user_count)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // otherwise the message is not sent
        send(clientSocket,  REACHED_PLAYER_COUNT, sizeof(REACHED_PLAYER_COUNT), 0);
        cout << REACHED_PLAYER_COUNT << current_user_count << "/" << player_count << endl;
    }
}

void sendEveryoneRoundInfo(char* buffer, Hangman& hangman)
{
    string roundInfo = "Round Info: \n";
    roundInfo += "Word: " + hangman.get_guessed_part() + "\n";
    roundInfo += "Round: " + std::to_string(hangman.increment_round()) + "\n";

    buffer = const_cast<char*>(roundInfo.c_str());

    for (auto& user : hangman.get_players())
    {
        send(user->m_userSessionFD, buffer, BUFFER_SIZE, 0);
    }
}

void sendPlayerInfo(User* user, char* buffer, Hangman& hangman)
{
    string messageToSend = string{YOUR_TURN} + "\n" ;
    messageToSend += ALREADY_GUESSED_LETTERS + hangman.get_already_guessed() + "\n";
    messageToSend += REMAINING_GUESS_COUNT + std::to_string(user->get_remaining_guesses()) + "\n";
    send(user->m_userSessionFD, messageToSend.c_str(), messageToSend.size(), 0);

    while (true)
    {
        send(user->m_userSessionFD, ASK_LETTER, sizeof(ASK_LETTER), 0);
        recv(user->m_userSessionFD, buffer, BUFFER_SIZE, 0);
        if (strlen(buffer) > 1)
        {
            send(user->m_userSessionFD, ONLY_ONE_LETTER, sizeof(ONLY_ONE_LETTER), 0);
            continue;
        }
        else if(!isalpha(buffer[0]))
        {
            send(user->m_userSessionFD, NOT_A_LETTER, sizeof(NOT_A_LETTER), 0);
            continue;
        }
        else if (hangman.get_already_guessed().find(buffer[0]) != std::string::npos)
        {
            send(user->m_userSessionFD, ALREADY_GUESSED, sizeof(ALREADY_GUESSED), 0);
            continue;
        }
        else
        {
            if(hangman.process_guessed_letter(buffer[0], user->m_userSessionFD))
            {
                send(user->m_userSessionFD, CORRECT_GUESS, sizeof(CORRECT_GUESS), 0);
                if (hangman.get_guessed_part().find('_') == std::string::npos)
                {
                    cout << GAME_OVER << endl;
                    break;
                }
            }
            else
            {
                send(user->m_userSessionFD, INCORRECT_GUESS, sizeof(INCORRECT_GUESS), 0);
            }
            break;
        }
    }
}

void sendWaiterInfo(User* user, std::vector<User*>& users)
{
    for (auto& localUser : users)
    {
        if (localUser->m_userSessionFD != user->m_userSessionFD)
        {
            send(localUser->m_userSessionFD, WAITING_FOR_OTHERS, sizeof(WAITING_FOR_OTHERS), 0);
        }
    }
}



void startGame(std::vector<User*>& users, char* buffer, Hangman& hangman)
{
    //sendEveryoneRoundInfo(users, buffer, hangman);
    while(true)
    {
        for (auto& user : users)
        {
            if (user == nullptr) continue;
            if(user->get_remaining_guesses() == 0)
            {
                send(user->m_userSessionFD, ELIMINATED, sizeof(ELIMINATED), 0);
                users.erase(users.begin() + user->get_id());
                delete user;
                break;
            }
            if (users.empty()) break;
            sendEveryoneRoundInfo(buffer, hangman);
            std::thread sendPlayerInfoThread(sendPlayerInfo, user, buffer, std::ref(hangman));
            std::thread sendWaiterInfoThread(sendWaiterInfo, user, std::ref(users));
            sendPlayerInfoThread.join();
            sendWaiterInfoThread.join();
        }
        if (users.empty()) break;
    }
}



int main() {
    signal(SIGPIPE, SIG_IGN); // ignore sigpipe
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTENING_PORT);
    inet_pton(AF_INET, LOCALHOST, &addr.sin_addr);

    int bindResult = bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr));

    if (bindResult == 0) cout << BIND_SUCCESSFUL << endl;
    else
    {
        cerr << BIND_FAILED << errno << endl;
        return -1;
    }

    int isListening = listen(socket_fd, MAX_BACKLOG);

    if (isListening == 0) cout << LISTENING_SUCCESSFUL << endl;
    else
    {
        cerr << LISTENING_FAILED << errno << endl;
        return -1;
    }

    string hangmanWord;

    cout << LISTEN << LISTENING_PORT << endl;
    cout << WELCOME_MSG << ASK_FOR_WORD << endl;
    cin >> hangmanWord;
    cout << START_GAME << hangmanWord << endl;


    int current_user_count {};
    int player_count {};
    std::vector<User*> users;
    std::vector<std::thread> threads;

    char buffer[BUFFER_SIZE] {};

    do
    {
        while(current_user_count > 0)
        {
            if(player_count != 0) break;
            else continue;
        } // wait for player number to be given from the first player
        // accept the incoming connection
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(struct sockaddr_in);
        int clientSocket = accept(socket_fd,
                                  (struct sockaddr *) &clientAddress,
                                  &clientAddressSize);
        if(clientSocket == -1)
        {
            cerr << CLIENT_CONNECTION_FAILED << errno << endl;
            continue;
        }

        std::thread acceptIncomingThread(acceptIncoming, std::ref(users), buffer, std::ref(player_count), std::ref(current_user_count), clientAddress, clientSocket);
        acceptIncomingThread.join();
    } while(player_count <= 0 || (current_user_count)!= player_count);

    auto* game = new Hangman(hangmanWord, users);

    printRegisteredUsers(users);

    startGame(users,  buffer, *game);

    if(!users.empty())
    {
        for(auto& user : users)
        {
            delete user;
        }
    }
    delete game;

    close(socket_fd);

    return 0;
}
