#include <iostream>
#include <string>

using namespace std;

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

int main() {
    int attempts = 0;

    while (attempts < 7) {
        drawHangman(attempts);
        cout << "Wprowadź kolejną próbę (0-7): ";
        cin >> attempts;
    }

    drawHangman(attempts);

    cout << "Gra zakończona!" << endl;

    return 0;
}
