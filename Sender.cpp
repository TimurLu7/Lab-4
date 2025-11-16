#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

struct Message {
    char text[20];
    bool active;
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "rus");
    if (argc < 2) {
        cout << "Ошибка: не указано имя файла" << endl;
        return 1;
    }

    const char* filename = argv[1];

    HANDLE hFileMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "FileMutex");
    HANDLE hReadyEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, "SenderReadyEvent");

    if (!hFileMutex) {
        cout << "Ошибка: не удалось открыть мьютекс FileMutex" << endl;
    }

    if (!hReadyEvent) {
        cout << "Ошибка: не удалось открыть событие SenderReadyEvent" << endl;
    }

    if (hReadyEvent) {
        SetEvent(hReadyEvent);
        cout << "Сигнал готовности отправлен Receiver" << endl;
    }
    else {
        cout << "Не удалось отправить сигнал готовности" << endl;
    }

    cout << "Готов к отправке сообщений" << endl;

    while (true) {
        cout << "\nВведите команду (send - отправить, exit - выйти): ";
        string command;
        cin >> command;

        if (command == "exit") {
            break;
        }
        else if (command == "send") {
            string text;
            cout << "Введите сообщение (до 20 символов): ";
            cin.ignore();
            getline(cin, text);

            if (text.length() >= 20) {
                cout << "Сообщение слишком длинное" << endl;
                continue;
            }

            if (hFileMutex) {
                WaitForSingleObject(hFileMutex, INFINITE);
            }

            fstream file(filename, ios::binary | ios::in | ios::out);
            if (!file) {
                cout << "Ошибка открытия файла " << filename << endl;
                if (hFileMutex) ReleaseMutex(hFileMutex);
                continue;
            }

            bool sent = false;
            for (int i = 0; i < 100; i++) {
                Message msg;
                file.seekg(i * sizeof(Message));
                file.read((char*)&msg, sizeof(Message));

                if (!msg.active) {
                    Message new_msg;
                    strcpy_s(new_msg.text, text.c_str());
                    new_msg.active = true;

                    file.seekp(i * sizeof(Message));
                    file.write((char*)&new_msg, sizeof(Message));
                    file.flush();

                    cout << "✓ Сообщение отправлено: " << text << endl;
                    sent = true;
                    break;
                }
            }

            file.close();

            if (hFileMutex) {
                ReleaseMutex(hFileMutex);
            }

            if (!sent) {
                cout << "Файл заполнен. Ожидаю освобождения места..." << endl;
                Sleep(2000);
            }
        }
    }

    if (hFileMutex) CloseHandle(hFileMutex);
    if (hReadyEvent) CloseHandle(hReadyEvent);

    cout << "Процесс Sender завершает работу..." << endl;
    return 0;
}