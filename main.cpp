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

void drawHangman(int attempts) {
    cout << "  +---+" << endl;
    cout << "  |   |" << endl;

    if (attempts >= 1) cout << "  O   |" << endl;
    else cout << "      |" << endl;

    if (attempts == 2) cout << "  |   |" << endl;
    else if (attempts == 3) cout << "  |\\  |" << endl;
    else if (attempts >= 4) cout << " /|\\  |" << endl;
    else cout << "      |" << endl;

    if (attempts == 5) cout << " /    |" << endl;
    if (attempts >= 6) cout << " / \\  |" << endl;
    else cout << "      |" << endl;

    cout << "=========" << endl;
}


class InputManager {
public:
    void ProcessInput() const {
        if (!_kbhit()) return;
        char key = __getch();

        switch (key) {
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
    static void printHelp() {
    };
};

class GameManager {
public:
    void Display() {
        drawHangman(7);
    }
};

int main() {
    InitNcurses();

    InputManager inputManager;
    GameManager gameManager;

    while (running) {
        ClearScreen();
        inputManager.ProcessInput();
        gameManager.Display();

        this_thread::sleep_for(chrono::milliseconds(50));
    }

    return 0;
}
