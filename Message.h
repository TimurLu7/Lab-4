#pragma once
#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

struct Message {
    char text[20];
    bool active;

    Message() : active(false) {
        text[0] = '\0';
    }

    Message(const char* str, bool isActive = true) : active(isActive) {
        setText(str);
    }

    Message(const std::string& str, bool isActive = true) : active(isActive) {
        setText(str);
    }

    void setText(const char* str) {
        if (str) {
            strncpy_s(text, str, 19);
            text[19] = '\0';
        }
        else {
            text[0] = '\0';
        }
    }

    void setText(const std::string& str) {
        setText(str.c_str());
    }
};

#endif // MESSAGE_H