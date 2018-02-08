#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <string>
#include <vector>

#include "origin.h"

class ErrorLog {
public:
    enum MessageType {
        Error,
        Warning
    };
    struct ErrorMessage {
        ErrorMessage(MessageType type, const Origin &origin, const std::string &message)
        : type(type), origin(origin), message(message)
        { }

        MessageType type;
        Origin origin;
        std::string message;
    };
    ErrorLog()
    : foundErrors(false)
    { }

    void add(MessageType type, const Origin &origin, const std::string &message) {
        messages.push_back(ErrorMessage(type, origin, message));
        if (type == Error) foundErrors = true;
    }

    unsigned count() const {
        return messages.size();
    }

    bool foundErrors;
    std::vector<ErrorMessage> messages;
};


#endif
