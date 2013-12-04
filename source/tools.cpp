#include "tools.h"

void alert(const std::string &title, const std::string &message)
{
#ifdef WIN32
    MessageBox(NULL, message.c_str(), title.c_str(), MB_ICONEXCLAMATION | MB_OK);
#else
    std::cout "<" << title.c_str() << "> " << message.c_str() << std::endl;
#endif
}
