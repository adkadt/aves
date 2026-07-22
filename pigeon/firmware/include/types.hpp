#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

struct DateTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t day;
    uint8_t month;
    uint16_t year;
};

#endif // TYPES_HPP