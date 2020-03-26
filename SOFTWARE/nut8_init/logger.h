#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class Logger
{
public:
    Logger();
    Logger(std::string _name);

    template <class T>
    Logger & operator << (T str) {
        if (name.size() > 0)
            std::cout << " [" << name << "] " << str << std::endl;
        else
            std::cout << str << std::endl;

        return *this;
    }

    void out(Logger l);
private:
    std::string name;
};



#endif // LOGGER_H
