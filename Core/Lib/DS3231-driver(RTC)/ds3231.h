#ifndef _DS3231_H
#define _DS3231_H

#include "hardware/i2c.h"
#include <stdint.h>
#include <stdio.h>

#define ds3231_add 0x00

// #define _12hour_mode
#define FirstDayOfWeek Saturday

/// @brief  Time registers addrese - in BCD format
#define Seconds_Reg 0x00
#define Minutes_Reg 0x01
#define Hours_Reg 0x02

/// @brief  Date registers addrese - in BCD format
#define Day_Reg 0x03
#define Date_Reg 0x04
#define Month_Century_Reg 0x05
#define Year_Reg 0x06

/// @brief  Alarm 1 registers addrese - in BCD format
#define Alarm1_Seconds_Reg 0x07
#define Alarm1_Minutes_Reg 0x08
#define Alarm1_Hours_Reg 0x09
#define Alarm1_Day_Date_Reg 0x0A

/// @brief  Alarm 2 registers addrese - in BCD format
#define Alarm2_Minutes_Reg 0x0B
#define Alarm2_Hours_Reg 0x0C
#define Alarm2_Day_Date_Reg 0x0D

/// @brief  Control registers addrese - in BCD format
#define Control_Reg 0x0E
#define Control_Status_Reg 0x0F
#define Aging_Offset_Reg 0x10
#define MSB_of_Temp_Reg 0x11
#define LSB_of_Temp_Reg 0x12

#ifdef _12hour_mode
typedef enum { AM, PM } Cycle;

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    Cycle AM_PM;

} ds3231_time;

#else
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;

} ds3231_time;
#endif

typedef struct {
    uint8_t day_w;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} ds3231_date;

typedef struct {
    uint8_t A1_sec;
    uint8_t A1_min;
    uint8_t A1_hour;
    uint8_t A1_day_w;
    uint8_t A1_day;
} ds3231_Alarm1;

typedef struct {
    uint8_t A2_min;
    uint8_t A2_hour;
    uint8_t A2_day_w;
    uint8_t A2_day;
} ds3231_Alarm2;

typedef enum { Bcd, Bin } DateFormat;

typedef enum { Intrupt, SquareWave } Pin_INT_SQW;
typedef enum { _1hz, _1_024hz, _4_096hz, _8_192hz } SquareWave_Ferq;
typedef enum { _12hour, _24hour } Time_Mode;

typedef struct {
    Pin_INT_SQW INT_SQW_Function;
    SquareWave_Ferq SquareWaveFerq;
    Time_Mode TimeMode;

} ds3231_init;

/// @brief Alarms cycles
typedef enum { per_sec, sec_match, sec_min_match, sec_min_hour_match, sec_min_hour_day_match, sec_min_hour_day_w_match } Alarm1_RATE;
typedef enum { per_min, min_match, min_hour_match, min_hour_day_match, min_hour_day_w_match } Alarm2_RATE;

void Ds3231_init(ds3231_init *init);


void Ds3231_GetTime(ds3231_time *time, DateFormat format);
void Ds3231_GetDate(ds3231_date *date, DateFormat format);

void Ds3231_SetAlarm1(ds3231_Alarm1 *alarm1, Alarm1_RATE period);
void Ds3231_SetAlarm2(ds3231_Alarm2 *alarm2, Alarm2_RATE period);

void Ds3231_Alarm_Interrupt_Set(ds3231_init *init_status, uint8_t alarm_num, uint8_t status);

#endif