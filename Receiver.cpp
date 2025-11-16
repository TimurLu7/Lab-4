#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

struct Message {
    char text[20];
    bool active;
};

int main() {
    setlocale(LC_ALL, "rus");
    string filename;
    int count;

    cout << "Введите имя бинарного файла: ";
    cin >> filename;
    cout << "Введите количество записей: ";
    cin >> count;

    HANDLE hFileMutex = CreateMutexA(NULL, FALSE, "FileMutex");
    HANDLE hReadyEvent = CreateEventA(NULL, TRUE, FALSE, "SenderReadyEvent");

    if (!hFileMutex || !hReadyEvent) {
        cout << "Ошибка создания объектов синхронизации" << endl;
        return 1;
    }

    ResetEvent(hReadyEvent);

    ofstream out(filename, ios::binary);
    Message msg = { "", false };

    for (int i = 0; i < count; i++) {
        out.write((char*)&msg, sizeof(Message));
    }
    out.close();

    int senderCount;
    cout << "Введите количество процессов Sender: ";
    cin >> senderCount;

    for (int i = 0; i < senderCount; i++) {
        string command = "Sender.exe " + filename;

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        if (CreateProcessA(
            NULL,
            (LPSTR)command.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            cout << "Процесс Sender " << i + 1 << " успешно запущен" << endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            DWORD error = GetLastError();
            cout << "Ошибка запуска Sender " << i + 1 << ". Код ошибки: " << error << endl;
        }
    }

    cout << "Ожидаю готовности всех " << senderCount << " процессов Sender..." << endl;

    int readyCount = 0;
    while (readyCount < senderCount) {
        DWORD result = WaitForSingleObject(hReadyEvent, 5000);
        if (result == WAIT_OBJECT_0) {
            readyCount++;
            cout << "Готовых процессов: " << readyCount << "/" << senderCount << endl;
            ResetEvent(hReadyEvent);
        }
        else if (result == WAIT_TIMEOUT) {
            cout << "Таймаут ожидания. Готовых: " << readyCount << "/" << senderCount << endl;
            break;
        }
    }

    if (readyCount == senderCount) {
        cout << "Все процессы Sender готовы к работе!" << endl;
    }
    else {
        cout << "Не все процессы Sender ответили. Продолжаем работу..." << endl;
    }

    while (true) {
        cout << "\nВведите команду (read - прочитать, exit - выйти): ";
        string command;
        cin >> command;

        if (command == "exit") {
            break;
        }
        else if (command == "read") {
            WaitForSingleObject(hFileMutex, INFINITE);

            fstream file(filename, ios::binary | ios::in | ios::out);
            if (!file) {
                cout << "Ошибка открытия файла" << endl;
                ReleaseMutex(hFileMutex);
                continue;
            }

            bool found = false;
            for (int i = 0; i < count; i++) {
                Message msg;
                file.seekg(i * sizeof(Message));
                file.read((char*)&msg, sizeof(Message));

                if (msg.active) {
                    cout << "Получено сообщение: " << msg.text << endl;
                    msg.active = false;
                    file.seekp(i * sizeof(Message));
                    file.write((char*)&msg, sizeof(Message));
                    found = true;
                    break;
                }
            }

            file.close();
            ReleaseMutex(hFileMutex);

            if (!found) {
                cout << "Нет новых сообщений. Ожидаю..." << endl;
                Sleep(1000);
            }
        }
    }

    CloseHandle(hFileMutex);
    CloseHandle(hReadyEvent);

    return 0;
}