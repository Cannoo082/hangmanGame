#include "User.hpp"
#include "Hangman.hpp"


using std::string;
using std::vector;


string Hangman::get_already_guessed() const
{
    string alreadyGuessed;
    if (m_alreadyGuessed[0] == 0) return string {"None"};
    for (int i{}; i < std::strlen(m_alreadyGuessed); i++)
    {
        alreadyGuessed += m_alreadyGuessed[i];
        alreadyGuessed += " ";
    }
    return alreadyGuessed;
}

void Hangman::update_guessed_part(char letter)
{
    for (int i{}; i < m_letterCount; ++i)
    {
        if (m_word[i] == letter)
        {
            m_guessedPart[i] = letter;
        }
    }
}

bool Hangman::process_guessed_letter(char letter, int socket_fd)
{
    bool found = false;
    for (int i{}; i < m_letterCount; ++i)
    {
        if (m_word[i] == letter)
        {
            m_guessedPart[i] = letter;
            found = true;
            update_guessed_part(letter);
        }
    }
    if (!found)
    {
        for (auto& player : m_players)
        {
            if (player->m_userSessionFD == socket_fd)
            {
                player->decrement_allowed_guesses();
            }
        }
    }
    update_guessed_letters(letter);

    return found;
}

void Hangman::update_guessed_letters(char letter)
{
    for (int i{}; i < 26; i++)
    {
        if (m_alreadyGuessed[i] == 0)
        {
            m_alreadyGuessed[i] = letter;
            break;
        }
    }
}

Hangman::Hangman(string word, vector<User*>& players) : m_word{std::move(word)}, m_letterCount{m_word.length()}, m_players{players}
{
    for (int i{} ; i < m_letterCount; ++i)
    {
        m_guessedPart += "_";
    }
    for (int i{}; i < 26; ++i)
    {
        m_alreadyGuessed[i] = 0;
    }
}
