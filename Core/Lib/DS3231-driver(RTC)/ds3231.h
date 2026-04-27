#ifndef _DS3231_H
#define _DS3231_H

#include "hardware/i2c.h"
#include "BCD_Conversion.h"

#include <stdint.h>
#include <stdio.h>

#define ds3231_add 0x68U
#define i2c_bus i2c0

#define negative_sigh 1
#define posetive_sigh 0
#define FirstDayOfWeek Saturday

/** @addtogroup Exported_macro
 * @{
 */
#define SET_BIT(REG, BIT) ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT) ((REG) & (BIT))

#define CLEAR_REG(REG) ((REG) = (0x0))

#define WRITE_REG(REG, VAL) ((REG) = (VAL))

#define READ_REG(REG) ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

/// @brief  Time registers addrese - in BCD format
#define Seconds_Reg 0x00U
#define Minutes_Reg 0x01U
#define Hours_Reg 0x02U
#define Time_Format_Mask 0x40U

#define _12_Hours_Mask 0x1FU
#define _24_Hours_Mask 0x3FU

#define _Tens_Hours_Mask 0x10U
#define _Twoenies_Hours_Mask 0x20U

/// @brief  Date registers addrese - in BCD format
#define Day_Reg 0x03U
#define Date_Reg 0x04U
#define Month_Century_Reg 0x05U
#define Year_Reg 0x06U

/// @brief  Alarm 1 registers addrese - in BCD format
#define Alarm1_Seconds_Reg 0x07U
#define Alarm1_Minutes_Reg 0x08U
#define Alarm1_Hours_Reg 0x09U
#define Alarm1_Day_Date_Reg 0x0AU

/// @brief  Alarm 2 registers addrese - in BCD format
#define Alarm2_Minutes_Reg 0x0BU
#define Alarm2_Hours_Reg 0x0CU
#define Alarm2_Day_Date_Reg 0x0DU

/// @brief  Control registers addrese - in BCD format
#define Control_Reg 0x0EU
#define Status_Reg 0x0FU
#define Aging_Offset_Reg 0x10U
#define MSB_Temp_Reg 0x11U
#define LSB_Temp_Reg 0x12U

/// @brief  Control_Reg bit masks
#define Control_A1IE_Mask 0x01U
#define Control_A2IE_Mask 0x02U
#define Control_INTCN_Mask 0x04U
#define Control_RS1_Mask 0x08U
#define Control_RS2_Mask 0x10U
#define Control_CONV_Mask 0x20U
#define Control_BBSQW_Mask 0x40U
#define Control_EOSC_Mask 0x80U

#define SQ_Wave_1hz_Mask 0x18U
#define SQ_Wave_1_024hz_Mask 0x08U
#define SQ_Wave_4_096hz_Mask 0x10U
#define SQ_Wave_8_192hz_Mask 0x18U

/// @brief  Status_Reg bit masks
#define Status_A1F_Mask 0x01U
#define Status_A2F_Mask 0x02U
#define Status_BSY_Mask 0x04U
#define Status_EN32kHz_Mask 0x08U
#define Status_OSF_Mask 0x80U

/// @brief  Aging_Offset_Reg bit masks
#define Aging_Offset_Value_Mask 0x7FU
#define Aging_Offset_Sign_Mask 0x80U

/// @brief  MSB_Temp_Reg bit masks
#define MSB_Temp_Data_Mask 0x7FU
#define MSB_Temp_Sign_Mask 0x80U

/// @brief  LSB_Temp_Reg bit masks
#define Temp_LSB_Data_Mask 0xC0U

typedef enum { Posetive_Sigh, Negative_Sigh } Sign;

typedef enum { AM, PM } Cycle;
typedef enum { _24hour_mode, _12hour_mode } Time_Format;

typedef enum { Bcd, Bin } DateFormat;
typedef enum { Intrupt, SquareWave } Pin_INT_SQW;
typedef enum { _1hz, _1_024hz, _4_096hz, _8_192hz } SquareWave_Ferq;
typedef enum { Alarm1, Alarm2 } Alarm;
typedef enum { Osc_Disable, SqWave_Enable } Bat_Mode;

/// @brief Alarms cycles
typedef enum { per_sec, sec_match, sec_min_match, sec_min_hour_match, sec_min_hour_day_match, sec_min_hour_day_w_match } Alarm1_RATE;
typedef enum { per_min, min_match, min_hour_match, min_hour_day_match, min_hour_day_w_match } Alarm2_RATE;

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    Time_Format time_format;
    Cycle AM_PM;

} ds3231_time;

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

typedef struct {
    Pin_INT_SQW INT_SQW_Function;
    SquareWave_Ferq SquareWaveFerq;
    uint8_t Osc_onBat;
    uint8_t SqWave_onBat;
} ds3231_init;

void Generic_i2c_Write(uint8_t *date, uint8_t len);
void Generic_i2c_Read(uint8_t reg, uint8_t *value, uint8_t len);

void Ds3231_Init(ds3231_init *data);

void Ds3231_SetTime(ds3231_time *time, DateFormat format);
void Ds3231_SetDate(ds3231_date *date, DateFormat format);

void Ds3231_GetTime(ds3231_time *time, DateFormat format);
void Ds3231_GetDate(ds3231_date *date, DateFormat format);

void Ds3231_SetAlarm1(ds3231_Alarm1 *alarm1, Alarm1_RATE period);
void Ds3231_SetAlarm2(ds3231_Alarm2 *alarm2, Alarm2_RATE period);
void Ds3231_SetAlarm_Interrupt(Alarm alarm_num, uint8_t status);

/// @brief After Intrupt is set user must clear the flag or the INT\SQW is set until.
void Ds3231_Reset_Alarm_Flag(Alarm alarm_num);
uint8_t Ds3231_Read_Alarm_Flag(Alarm alarm_num);

void Ds3231_Set_INT_SQW_Pin(SquareWave_Ferq SquareWaveFerq, Pin_INT_SQW status);

uint8_t Ds3231_check_validity_timekeeping();
void Ds3231_Enable_Oscillator();
uint8_t Ds3231_check_if_busy();

void Ds3231_Set_32kHz_Output(uint8_t status);

void Ds3231_Set_Aging_Offset(int8_t value);
void Ds3231_UpdateTemp_TCXO();

int16_t Ds3231_Read_Temp();
uint8_t Ds3231_Get_Temp_Fraction(int16_t fixed_temp);
int8_t Ds3231_Get_Temp_Integer(int16_t fixed_temp);
#endif