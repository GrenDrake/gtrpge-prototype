#ifndef BUILDERROR_H
#define BUILDERROR_H

class Origin;

class BuildError : public std::runtime_error {
public:
    BuildError(const std::string &msg);
    BuildError(const Origin &origin, const std::string &msg);
    virtual const char* what() const throw();
private:
    static const int errorMessageLength = 128;
    char errorMessage[errorMessageLength];
};

#endif
