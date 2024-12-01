#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <fstream>
#include <stdarg.h>


#ifdef _WIN32

#include <conio.h>
#define CLEAR "cls"
#define NEWLINE "\n"

void ClearScreen() {
    system(CLEAR);
}

void InitNcurses() {
}

int __cdecl __getch() {
    return _getch();
}

#else

#include <ncurses.h>
#define NEWLINE "\n\r"
#define CLEAR "clear"

bool _kbhit() {
    return true;
}

void ClearScreen() {
    system(CLEAR);
}

void InitNcurses() {
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
}

int __cdecl __getch() {
    return getch();
}
#endif

using namespace std;

string newLine() {
    return NEWLINE;
}

string my_sprintf(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return string(buffer);
}

bool running = true;

class FileHandler {
public:
    void SaveWord(const string &word, const string &filename) {
        ofstream outFile(filename, ios::app); // Otwórz plik w trybie dopisywania
        if (outFile.is_open()) {
            outFile << word << endl; // Zapisz słowo w nowym wierszu
            outFile.close();
        } else {
            cerr << "Nie można otworzyć pliku do zapisu!" << endl;
        }
    }

    vector<string> LoadAllWords(const string &filename) {
        ifstream inFile(filename);
        vector<string> words;
        string word;

        if (inFile.is_open()) {
            while (getline(inFile, word)) {
                words.push_back(word);
            }
            inFile.close();
        } else {
            cerr << "Nie można otworzyć pliku do odczytu!" << endl;
        }

        return words;
    }
};

enum GameStage {
    Play = 0,
    Lose = 1,
    Win = 2,
    Menu = 27
};

class AGame {
public:
    int m_attempts = 0;
    GameStage m_gameStage = Menu;
    string m_word;

    void InitWord(string word) {
        m_word = word;
    }

    void Reset() {
        m_attempts = 0;
        m_gameStage = Play;
        m_guessedLetters.clear();
    }

    vector<char> m_guessedLetters;

    virtual void drawHangman() = 0;
};

class AGameStage {
public:
    virtual ~AGameStage() = default;

    AGameStage(AGame &game): m_game(game) {
        addKeyHandler('*', [](char key) {
        });
        addKeyHandler(27, [&](char key) {
            m_game.m_gameStage = static_cast<GameStage>(m_prevGameStage);
        });
    }

    int m_prevGameStage = -1;

    virtual void Display() = 0;

    void ProcessInput(char key) {
        if (m_keyHandlers.find(key) != m_keyHandlers.end()) {
            m_keyHandlers[key](key);
        } else {
            m_keyHandlers['*'](key);
        }
    }

    void addKeyHandler(char key, function<void(char)> handler) {
        m_keyHandlers[key] = std::move(handler);
    }

    unordered_map<char, function<void(char)> > m_keyHandlers;
    AGame &m_game;
};

class MenuStage : public AGameStage {
public:
    MenuStage(AGame &game): AGameStage(game) {
        addKeyHandler('q', [&](char key) { running = false; });
        addKeyHandler('1', [&](char key) {
            if (!m_game.m_word.empty()) {
                cout << "Od nowa (t/n):" << endl;
                if ('t' == __getch()) {
                    m_game.Reset();
                }
            }
            cout << "Wymysl sentencje do odgadniecia:" << endl;
            string word;
            getline(cin, word);
            saveCurrentWord(word);
            m_game.InitWord(word);
            m_game.Reset();
        });
        addKeyHandler('2', [&](char key) {
            if (!m_game.m_word.empty()) {
                cout << "Od nowa (t/n):" << endl;
                if ('t' == __getch()) {
                    m_game.Reset();
                }
            }

            m_game.InitWord(chooseWord());
            m_game.Reset();
        });
        addKeyHandler('h', [&](char key) { ; });
        addKeyHandler('r', [&](char key) {
            cout << "Zagraj od nowa (t/n):" << endl;
            if ('t' == __getch()) {
                m_game.Reset();
            }
        });
    }

    void Display() {
        cout << "[1] Zagraj z wlasnym slowem" << endl;
        cout << "[2] Zagraj wybierajac slowo z listy" << endl;
        cout << "[q] Wyjscie" << endl;
        cout << "[Esc] Powrot" << endl;
    }

private:
    static void saveCurrentWord(const string &word) {
        FileHandler fileHandler;
        fileHandler.SaveWord(word, "words.txt");
    }

    static string chooseWord() {
        FileHandler fileHandler;
        auto words = fileHandler.LoadAllWords("words.txt");
        while (true) {
            int i = 0;
            cout << "Wybierz numer slowa:" << endl;
            for (const string &word: words) {
                cout << my_sprintf("[%d] %s", ++i, word.c_str()) << endl;
            }
            int index;
            cin >> index;
            if (index < 1 || index > words.size()) {
                ClearScreen();
                cout << "Wybierz poprawny numer slowa" << endl << endl;
            } else {
                return words[index - 1];
            }
        }
    }
};

class PlayStage : public AGameStage {
public:
    PlayStage(AGame &game): AGameStage(game) {
        m_game.m_guessedLetters.resize(m_game.m_word.length(), '_');

        addKeyHandler('*', [&](char key) {
            auto pos = m_game.m_word.find(key);
            if (pos != string::npos) {
                m_game.m_guessedLetters.push_back(key);
            } else {
                m_game.m_attempts++;
            }
            bool allGuessed = true;
            for (char c: m_game.m_word) {
                if (find(m_game.m_guessedLetters.begin(), m_game.m_guessedLetters.end(), c) == m_game.
                    m_guessedLetters.
                    end()) {
                    allGuessed = false;
                    break;
                }
            }
            if (allGuessed) {
                m_gameStage = Win;
            }
            if (m_game.m_attempts >= 6) {
                m_game.m_gameStage = Lose;
            }
        });
    }

    void Display() {
        m_game.drawHangman();
        if (m_gameStage == Win) {
            for (size_t i = 0; i < m_game.m_word.length(); i++) {
                char letter = m_game.m_word[i];
                if (count(m_game.m_guessedLetters.begin(), m_game.m_guessedLetters.end(), letter)) {
                    cout << "\033[1;" << (31 + (i % 7)) << "m" << letter << "\033[0m";
                } else {
                    cout << '_';
                }
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            cout << endl << "Wygrana!!!";
            this_thread::sleep_for(chrono::seconds(2));
        } else {
            for (char letter: m_game.m_word) {
                if (count(m_game.m_guessedLetters.begin(), m_game.m_guessedLetters.end(), letter)) {
                    cout << letter;
                } else {
                    cout << '_';
                }
            }
        }
        cout << endl;
        if (m_gameStage > -1) {
            m_game.m_gameStage = m_gameStage;
        }
    }

private:
    GameStage m_gameStage;
};

class LoseStage : public MenuStage {
public:
    LoseStage(AGame &game): MenuStage(game) {
    }

    void Display() {
        MenuStage::Display();
        cout << endl << "Przegrana!!!" << endl;
    }
};

class WinStage : public MenuStage {
public:
    WinStage(AGame &game): MenuStage(game) {
    }

    void Display() {
        MenuStage::Display();
        cout << endl << "Wygrana!!!" << endl;
    }
};

class Game : public AGame {
public:
    unordered_map<int, unique_ptr<AGameStage> > gameStageHandlers;

    void Display() {
        gameStageHandlers[m_gameStage]->Display();
    }

    void processInput() {
        if (!_kbhit()) return;
        char key = __getch();
        GameStage tmpGameStage = m_gameStage;
        if (key == 27) {
            if (gameStageHandlers[m_gameStage]->m_prevGameStage < 0) {
                m_gameStage = Menu;
                gameStageHandlers[m_gameStage]->m_prevGameStage = tmpGameStage;
                return;
            }
        }

        gameStageHandlers[m_gameStage]->ProcessInput(key);
    }

    void addAttempt() {
        ++m_attempts;
    }

    void drawHangman() override {
        cout << "  +---+" << endl;
        cout << "  |   |" << endl;

        if (m_attempts >= 1) cout << "  O   |" << endl;
        else cout << "      |" << endl;

        if (m_attempts == 2) cout << "  |   |" << endl;
        else if (m_attempts == 3) cout << "  |\\  |" << endl;
        else if (m_attempts >= 4) cout << " /|\\  |" << endl;
        else cout << "      |" << endl;

        if (m_attempts == 5) cout << " /    |" << endl;
        if (m_attempts >= 6) cout << " / \\  |" << endl;
        else cout << "      |" << endl;

        cout << "=========" << endl;
    }

    void InitHandlers() {
        gameStageHandlers[Menu] = make_unique<MenuStage>(*this);
        gameStageHandlers[Play] = make_unique<PlayStage>(*this);
        gameStageHandlers[Lose] = make_unique<LoseStage>(*this);
        gameStageHandlers[Win] = make_unique<WinStage>(*this);
    }
};

int main() {
    InitNcurses();

    Game game;
    game.InitHandlers();

    while (running) {
        ClearScreen();
        game.processInput();
        game.Display();

        this_thread::sleep_for(chrono::milliseconds(50));
    }

    return 0;
}
