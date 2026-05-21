#include <iostream>
using namespace std;

int main() {
    char op;
    double num1, num2;

    cout << "Enter first number: ";
    cin >> num1;
    cout << "Enter an operator (+, -, *, /): ";
    cin >> op;
    cout << "Enter second number: ";
    cin >> num2;

    if (op == '+') {
        cout << "Result: " << num1 + num2 << endl;
    }
    else if (op == '-') {
        cout << "Result: " << num1 - num2 << endl;
    }
    else if (op == '*') {
        cout << "Result: " << num1 * num2 << endl;
    }
    else if (op == '/') {
        if (num2 == 0) {
            cout << "Division by zero is not allowed." << endl;
        }
        else {
            cout << "Result: " << num1 / num2 << endl;
        }
    }
    else {
        cout << "Invalid operator." << endl;
    }

    return 0;
}
