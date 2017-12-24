#ifndef PLAYERROR_H
#define PLAYERROR_H

#include <stdexcept>
#include <string>

class PlayError : public std::runtime_error {
public:
        PlayError(const std::string &msg)
        : std::runtime_error(msg)
        { }
};

#endif