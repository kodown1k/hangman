#include <iostream>
#include <string>
#include <thread>

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


class Game {
public:
    int m_attempts = 0;

    void Display() {
        drawHangman();
    }

    void addAttempt() {
        ++m_attempts;
    }

    void drawHangman() {
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
};

class InputManager {
public:
    InputManager(Game &game): m_game(game) {
    }

    void ProcessInput() {
        if (!_kbhit()) return;
        char key = __getch();

        switch (key) {
            case 's':
                m_game.addAttempt();
                break;
            case 'h':
            case 'H':
                printHelp();
                break;
            case 'e':
                exit(0);
                break;
            default:
                break;
        }
    }

private:
    Game &m_game;

    static void printHelp() {
    };
};


int main() {
    InitNcurses();

    Game game;
    InputManager inputManager(game);

    while (running) {
        ClearScreen();
        inputManager.ProcessInput();
        game.Display();

        this_thread::sleep_for(chrono::milliseconds(50));
    }

    return 0;
}
