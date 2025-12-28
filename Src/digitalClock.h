#ifndef DIGITAL_CLOCK_H
#define DIGITAL_CLOCK_H

#include <stdint.h>

typedef enum {
    MODE_12_HOUR,
    MODE_24_HOUR
} ClockMode;

typedef enum {
    DAY_SUNDAY    = 1 << 7,
    DAY_MONDAY    = 1 << 6,
    DAY_TUESDAY   = 1 << 5,
    DAY_WEDNESDAY = 1 << 4,
    DAY_THURSDAY  = 1 << 3,
    DAY_FRIDAY    = 1 << 2,
    DAY_SATURDAY  = 1 << 1
} DayOfWeek;

// AM/PM bit definition (bit 0)
#define AM_PM_BIT     (1 << 0)
#define IS_PM(x)      ((x) & AM_PM_BIT)
#define IS_AM(x)      (!((x) & AM_PM_BIT))
#define SET_PM(x)     ((x) |= AM_PM_BIT)
#define SET_AM(x)     ((x) &= ~AM_PM_BIT)

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t temparature;
    union {
        uint8_t dayOfWeekAmPm;    // Combined: bits 7-1 for day, bit 0 for AM/PM
        struct {
            uint8_t amPm : 1;     // bit 0: 0=AM, 1=PM
            uint8_t dayFlags : 7; // bits 7-1: day of week flags
        };
    } dayOfWeek;
    struct {
        ClockMode mode12Or24;
        uint8_t   brightnessLevel;
    } metadata;

} ClockTime;


typedef struct {
    uint64_t lastBlinkMillis;
} MillisTracker;

#endif // DIGITAL_CLOCK_H
