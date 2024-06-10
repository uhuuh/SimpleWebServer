#include "base.hpp"

void assertm(bool res) {
    if (!res) {
        if (errno != 0) {
            throw std::runtime_error(strerror(errno));
        } else {
            throw std::runtime_error("");
        }
    }
}

void assertm(bool res, const char* error_msg) {
    if (!res) {
        throw std::runtime_error(error_msg);
    }
}
