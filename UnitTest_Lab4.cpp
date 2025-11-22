#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "Message.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTestLab4
{
	TEST_CLASS(UnitTestLab4)
	{
	public:
		
        TEST_METHOD(Message_DefaultConstructor)
        {
            Message msg{};
            Assert::AreEqual(false, msg.active);
            Assert::AreEqual('\0', msg.text[0]);
        }

        TEST_METHOD(Message_ParameterizedConstructor)
        {
            Message msg("Test message", true);
            Assert::AreEqual(true, msg.active);
            Assert::AreEqual(string("Test message"), string(msg.text));
        }

        TEST_METHOD(Message_SetText_Short)
        {
            Message msg;
            msg.setText("Hello");
            Assert::AreEqual(string("Hello"), string(msg.text));
        }

        TEST_METHOD(Message_SetText_Long_Truncated)
        {
            Message msg;
            string longText = "This is a very long message that exceeds 20 characters";
            msg.setText(longText);
            Assert::AreEqual(19,(int)strlen(msg.text)); 
        }
    };

    TEST_CLASS(FileOperationTests)
    {
    public:

        TEST_METHOD(File_CreateAndReadMessage)
        {
            const char* filename = "test_single.bin";
            Message original("Test message", true);

            ofstream out(filename, ios::binary);
            out.write((char*)&original, sizeof(Message));
            out.close();

            ifstream in(filename, ios::binary);
            Message readMsg;
            in.read((char*)&readMsg, sizeof(Message));
            in.close();

            Assert::AreEqual(original.active, readMsg.active);
            Assert::AreEqual(string(original.text), string(readMsg.text));
        }

        TEST_METHOD(File_MultipleMessages)
        {
            const char* filename = "test_multi.bin";
            int count = 5;

            ofstream out(filename, ios::binary);
            Message emptyMsg;
            for (int i = 0; i < count; i++) {
                out.write((char*)&emptyMsg, sizeof(Message));
            }
            out.close();

            ifstream in(filename, ios::binary);
            in.seekg(0, ios::end);
            streampos size = in.tellg();
            in.close();

            Assert::AreEqual((long long)(count * sizeof(Message)), (long long)size);
        }

        TEST_METHOD(File_SeekAndReadSpecificMessage)
        {
            const char* filename = "test_seek.bin";

            vector<Message> messages = {
                Message("First", true),
                Message("Second", false),
                Message("Third", true)
            };

            ofstream out(filename, ios::binary);
            for (const auto& msg : messages) {
                out.write((char*)&msg, sizeof(Message));
            }
            out.close();

            fstream file(filename, ios::binary | ios::in);
            Message readMsg;
            file.seekg(1 * sizeof(Message));
            file.read((char*)&readMsg, sizeof(Message));
            file.close();

            Assert::AreEqual(messages[1].active, readMsg.active);
            Assert::AreEqual(string(messages[1].text), string(readMsg.text));
        }
    };

    TEST_CLASS(SynchronizationTests)
    {
    public:

        TEST_METHOD(Mutex_Creation)
        {
            HANDLE hMutex = CreateMutexA(NULL, FALSE, "TestMutex");
            Assert::IsNotNull(hMutex);

            if (hMutex) {
                DWORD result = WaitForSingleObject(hMutex, 1000);
                Assert::AreEqual(WAIT_OBJECT_0, result);

                BOOL releaseResult = ReleaseMutex(hMutex);
                Assert::AreEqual(TRUE, releaseResult);

                CloseHandle(hMutex);
            }
        }

        TEST_METHOD(Event_CreationAndSignaling)
        {
            HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, "TestEvent");
            Assert::IsNotNull(hEvent);

            if (hEvent) {
                BOOL setResult = SetEvent(hEvent);
                Assert::AreEqual(TRUE, setResult);

                DWORD waitResult = WaitForSingleObject(hEvent, 1000);
                Assert::AreEqual(WAIT_OBJECT_0, waitResult);

                BOOL resetResult = ResetEvent(hEvent);
                Assert::AreEqual(TRUE, resetResult);

                CloseHandle(hEvent);
            }
        }
    };

    TEST_CLASS(IntegrationTests)
    {
    public:

        TEST_METHOD(MessageFlow_SendAndReceive)
        {
            const char* filename = "test_flow.bin";
            int messageCount = 3;

            ofstream out(filename, ios::binary);
            Message empty;
            for (int i = 0; i < messageCount; i++) {
                out.write((char*)&empty, sizeof(Message));
            }
            out.close();

            vector<string> testMessages = { "Hello", "World", "Test" };
            fstream file(filename, ios::binary | ios::in | ios::out);

            for (int i = 0; i < testMessages.size(); i++) {
                Message msg(testMessages[i], true);
                file.seekp(i * sizeof(Message));
                file.write((char*)&msg, sizeof(Message));
            }
            file.close();

            file.open(filename, ios::binary | ios::in);
            int activeCount = 0;
            for (int i = 0; i < messageCount; i++) {
                Message msg;
                file.seekg(i * sizeof(Message));
                file.read((char*)&msg, sizeof(Message));
                if (msg.active) {
                    activeCount++;
                }
            }
            file.close();

            Assert::AreEqual((int)testMessages.size(), activeCount);
        }

        TEST_METHOD(Message_Deactivation)
        {
            const char* filename = "test_deactivate.bin";

            Message activeMsg("Active message", true);
            ofstream out(filename, ios::binary);
            out.write((char*)&activeMsg, sizeof(Message));
            out.close();

            fstream file(filename, ios::binary | ios::in | ios::out);
            Message msg;
            file.read((char*)&msg, sizeof(Message));
            msg.active = false;
            file.seekp(0);
            file.write((char*)&msg, sizeof(Message));
            file.close();

            ifstream in(filename, ios::binary);
            Message readMsg;
            in.read((char*)&readMsg, sizeof(Message));
            in.close();

            Assert::AreEqual(false, readMsg.active);
            Assert::AreEqual(string("Active message"), string(readMsg.text));
        }
    };

    TEST_CLASS(ErrorHandlingTests)
    {
    public:

        TEST_METHOD(File_OpenNonExistent_ShouldFail)
        {
            fstream file("nonexistent_file.bin", ios::binary | ios::in);
            Assert::IsFalse(file.is_open());
        }

        TEST_METHOD(Synchronization_InvalidHandle)
        {
            HANDLE invalidMutex = NULL;
            DWORD result = WaitForSingleObject(invalidMutex, 100);
            Assert::AreEqual(WAIT_FAILED, result);
        }
    };
}
