#ifndef ORIGIN_H
#define ORIGIN_H

#include <string>

class Origin {
public:
    Origin()
    : file("(no-file)"), line(0), column(0)
    { }
    Origin(const std::string &file, int line, int column)
    : file(file), line(line), column(column)
    { }

    std::string file;
    int line, column;
};

std::ostream& operator<<(std::ostream &out, const Origin &origin);

#endif
