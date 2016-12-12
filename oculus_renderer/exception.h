#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception
{
public:
    Exception(std::string object, std::string message)
    {
        if (!object.empty())
            object_ = object;

        if (!message.empty())
            message_ = message;
    }

    std::string what() const
    {
        return "[" + object_ + "] threw exception: " + message_;
    }

protected:
    std::string message_{"EMPTY_MESSAGE"};
    std::string object_{"UNKNOWN_OBJECT"};
};

#endif // EXCEPTION_H