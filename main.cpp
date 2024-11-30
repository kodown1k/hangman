#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

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

bool running = true;

class AGame {
public:
    int m_attempts = 0;
    int m_gameStage = 27;
    string m_word;

    void InitWord(string word) {
        m_word = word;
    }

    void Reset() {
        m_attempts = 0;
        m_gameStage = 0;
        m_guessedLetters.clear();
    }

    vector<char> m_guessedLetters;

    virtual void drawHangman() =0;
};

class AGameStage {
public:
    AGameStage(AGame &game): m_game(game) {
        addKeyHandler('*', [](char key) {
        });
        addKeyHandler(27, [&](char key) {
            m_game.m_gameStage = m_prevGameStage;
        });
    }

    int m_prevGameStage = -1;

    void virtual Display() =0;

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
                cout << "Zagraj od nowa (t/n):" << endl;
                if ('t' == __getch()) {
                    m_game.Reset();
                }
            }
            cout << "Wymysl sentencje do odgadniecia:" << endl;
            string word;
            getline(cin, word);
            m_game.InitWord(word);
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
        cout << "[1] Zagraj" << endl;
        cout << "[h] Pomoc" << endl;
        cout << "[q] Wyjscie" << endl;
        cout << "[Esc] Powrot" << endl;
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
        });
    }

    void Display() {
        m_game.drawHangman();
        for (char letter: m_game.m_word) {
            if (count(m_game.m_guessedLetters.begin(), m_game.m_guessedLetters.end(), letter)) {
                cout << letter;
            } else {
                cout << '_';
            }
        }
        cout << endl;
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
        int tmpGameStage = m_gameStage;
        if (key == 27) {
            if (gameStageHandlers[m_gameStage]->m_prevGameStage < 0) {
                m_gameStage = 27;
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
        gameStageHandlers[27] = make_unique<MenuStage>(*this);
        gameStageHandlers[0] = make_unique<PlayStage>(*this);
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
