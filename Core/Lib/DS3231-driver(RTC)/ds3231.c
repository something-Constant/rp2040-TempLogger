#include "ds3231.h"
#include "BCD_Conversion.h"

// lookup table *****************************************************************
void Generic_i2c_Write(uint8_t *date, uint8_t len) { i2c_write_timeout_us(i2c_bus, ds3231_add, date, len, false, 200); }

void Generic_i2c_Read(uint8_t reg, uint8_t *value, uint8_t len) {
    i2c_write_timeout_us(i2c_bus, ds3231_add, &reg, 1, true, 200);
    i2c_read_timeout_us(i2c_bus, ds3231_add, value, len, false, 200);
}

void Ds3231_Init(ds3231_init *data) {
    uint8_t buffer[2] = {Control_Reg, 0};

    if (data->INT_SQW_Function == Intrupt) {
        CLEAR_BIT(buffer[1], Control_INTCN_Mask);
    }
    else {
        SET_BIT(buffer[1], Control_INTCN_Mask);

        switch (data->SquareWaveFerq) {
            case _1hz :
                CLEAR_BIT(buffer[1], SQ_Wave_1hz_Mask);

                break;
            case _1_024hz :
                SET_BIT(buffer[1], SQ_Wave_1_024hz_Mask);

                break;

            case _4_096hz :
                SET_BIT(buffer[1], SQ_Wave_4_096hz_Mask);

                break;

            case _8_192hz :
                SET_BIT(buffer[1], SQ_Wave_8_192hz_Mask);

                break;
        }

        if (data->SqWave_onBat) {
            SET_BIT(buffer[1], Control_BBSQW_Mask);
        }
        else {
            CLEAR_BIT(buffer[1], Control_BBSQW_Mask);
        }
    }

    if (data->Osc_onBat) {
        CLEAR_BIT(buffer[1], Control_EOSC_Mask);
    }
    else {
        SET_BIT(buffer[1], Control_EOSC_Mask);
    }

    Generic_i2c_Write(buffer, 2);
    Ds3231_Enable_Oscillator();
}

void Ds3231_SetTime(ds3231_time *time, DateFormat format) {
    uint8_t buffer[4] = {Seconds_Reg, 0, 0, 0};

    if (format == Bcd) {
        buffer[1] = BIN2BCD(time->sec);
        buffer[2] = BIN2BCD(time->min);

        if (time->time_format == _24hour_mode) {
            buffer[3] = (BIN2BCD(time->hour) & 0x0FU);

            if (time->hour >= 10U) {
                if (time->hour >= 20U)
                    buffer[3] |= (1U << 5U);
                else
                    buffer[3] |= (1U << 4U);
            }

            CLEAR_BIT(buffer[3], Time_Format_Mask);
        }

        else {
            buffer[3] = (BIN2BCD(time->hour) & 0x0FU);

            if (time->hour >= 10U)
                buffer[3] |= (1U << 4U);

            if (time->AM_PM == AM)
                CLEAR_BIT(buffer[3], 0x20U);
            else
                SET_BIT(buffer[3], 0x20U);

            SET_BIT(buffer[3], Time_Format_Mask);
        }
    }

    /// format == Bin
    else {
        buffer[1] = BIN2BCD(time->sec);
        buffer[2] = BIN2BCD(time->min);

        if (time->time_format == _24hour_mode) {
            buffer[3] = (BIN2BCD(time->hour) & 0x0FU);

            if (time->hour >= 10U) {
                if (time->hour >= 20U)
                    buffer[3] |= (1U << 5U);
                else
                    buffer[3] |= (1U << 4U);
            }

            CLEAR_BIT(buffer[3], Time_Format_Mask);
        }
        else {
            buffer[3] = (BIN2BCD(time->hour) & 0x0FU);

            if (time->hour >= 10U)
                buffer[3] |= (1U << 4U);

            if (time->AM_PM == AM)
                buffer[3] &= ~(1U << 5U);
            else
                buffer[3] |= (1U << 5U);

            SET_BIT(buffer[3], Time_Format_Mask);
        }
    }

    Generic_i2c_Write(buffer, 4);
}

void Ds3231_SetDate(ds3231_date *date, DateFormat format) {}

void Ds3231_GetTime(ds3231_time *time, DateFormat format) {
    uint8_t buffer[3] = {0, 0, 0};

    Generic_i2c_Read(Seconds_Reg, buffer, 3);

    if (format == Bcd) {
        time->sec = buffer[0];
        time->min = buffer[1];

        if (time->time_format == _24hour_mode) {
            time->hour = (buffer[2] & 0x0FU);

            if ((buffer[2] & 0x10U) && ! ((buffer[2] & 0x20U)))
                time->hour |= 0x10U;

            if (! (buffer[2] & 0x10U) && ((buffer[2] & 0x20U)))
                time->hour |= 0x20U;
        }
        ///_12hour_mode
        else {
            time->hour = (buffer[2] & _12_Hours_Mask);

            if (buffer[2] & 0x20U)
                time->AM_PM = PM;
            else
                time->AM_PM = AM;
        }
    }
    /// format == Bin
    else {
        time->sec = BCD2BIN(buffer[0]);
        time->min = BCD2BIN(buffer[1]);

        if (time->time_format == _24hour_mode) {
            time->hour = BCD2BIN(buffer[2] & 0x0FU);

            if ((buffer[2] & _Tens_Hours_Mask) && (! (buffer[2] & _Twoenies_Hours_Mask))) {
                time->hour += 10;
            }

            else if ((! (buffer[2] & _Tens_Hours_Mask)) && (buffer[2] & _Twoenies_Hours_Mask)) {
                time->hour += 20;
            }
        }
        else {
            time->hour = BCD2BIN(buffer[2] & 0x0FU);

            if (buffer[2] & _Tens_Hours_Mask)
                time->hour += 10;

            if (buffer[2] & 0x20U)
                time->AM_PM = PM;
            else
                time->AM_PM = AM;
        }
    }
}

void Ds3231_GetDate(ds3231_date *date, DateFormat format);

void Ds3231_SetAlarm1(ds3231_Alarm1 *alarm1, Alarm1_RATE period);
void Ds3231_SetAlarm2(ds3231_Alarm2 *alarm2, Alarm2_RATE period);

void Ds3231_SetAlarm_Interrupt(Alarm alarm_num, uint8_t status) {
    uint8_t buffer[2] = {Control_Reg, 0};

    Generic_i2c_Read(Control_Reg, &buffer[1], 1);

    if (alarm_num == Alarm1) {
        if (status)
            SET_BIT(buffer[1], Control_A1IE_Mask);

        else
            CLEAR_BIT(buffer[1], Control_A1IE_Mask);
    }
    else {
        if (status)
            SET_BIT(buffer[1], Control_A2IE_Mask);

        else
            CLEAR_BIT(buffer[1], Control_A2IE_Mask);
    }

    Generic_i2c_Write(buffer, 2);
}

/// @brief After Intrupt is set user must clear the flag or the INT\SQW is set until.
void Ds3231_Reset_Alarm_Flag(Alarm alarm_num) {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    if (alarm_num == Alarm1) {
        CLEAR_BIT(buffer[1], Status_A1F_Mask);
    }
    else {
        CLEAR_BIT(buffer[1], Status_A2F_Mask);
    }

    Generic_i2c_Write(buffer, 2);
}

uint8_t Ds3231_Read_Alarm_Flag(Alarm alarm_num) {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    if (alarm_num == Alarm1) {
        if (buffer[1] & Status_A1F_Mask)
            return 1;
    }
    else {
        if (buffer[1] & Status_A2F_Mask)
            return 1;
    }

    return 0;
}

void Ds3231_Set_INT_SQW_Pin(SquareWave_Ferq SquareWaveFerq, Pin_INT_SQW status) {
    uint8_t buffer[2] = {Control_Reg, 0};

    Generic_i2c_Read(Control_Reg, &buffer[1], 1);

    if (status == Intrupt) {
        CLEAR_BIT(buffer[1], Control_INTCN_Mask);
    }

    else {
        SET_BIT(buffer[1], Control_INTCN_Mask);

        switch (SquareWaveFerq) {
            case _1hz :
                CLEAR_BIT(buffer[1], SQ_Wave_1hz_Mask);

                break;
            case _1_024hz :
                SET_BIT(buffer[1], SQ_Wave_1_024hz_Mask);

                break;

            case _4_096hz :
                SET_BIT(buffer[1], SQ_Wave_4_096hz_Mask);

                break;

            case _8_192hz :
                SET_BIT(buffer[1], SQ_Wave_8_192hz_Mask);

                break;
        }
    }
}

uint8_t Ds3231_check_validity_timekeeping() {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    if (buffer[1] & Status_OSF_Mask)
        return 0;
    else
        return 1;
}

void Ds3231_Enable_Oscillator() {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    CLEAR_BIT(buffer[1], Status_OSF_Mask);

    Generic_i2c_Write(buffer, 2);
}

uint8_t Ds3231_check_if_busy() {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    if (buffer[1] & Status_BSY_Mask)
        return 1;
    else
        return 0;
}

void Ds3231_Set_32kHz_Output(uint8_t status) {
    uint8_t buffer[2] = {Status_Reg, 0};

    Generic_i2c_Read(Status_Reg, &buffer[1], 1);

    SET_BIT(buffer[1], Status_EN32kHz_Mask);

    Generic_i2c_Write(buffer, 2);
}

void Ds3231_UpdateTemp_TCXO() {
    uint8_t buffer[2] = {Control_Reg, 0};

    Generic_i2c_Read(Control_Reg, &buffer[1], 1);

    SET_BIT(buffer[1], Control_CONV_Mask);

    Generic_i2c_Write(buffer, 2);
}

/// need to fix
void Ds3231_Set_Aging_Offset(int8_t value) {
    uint8_t buffer[2] = {Aging_Offset_Reg, 0};

    if (value >= 0) {
        buffer[1] = (value & Aging_Offset_Value_Mask);
    }
    else {
        buffer[1] = (value & Aging_Offset_Value_Mask) | Aging_Offset_Sign_Mask;
    }

    Generic_i2c_Write(buffer, 2);
}

int16_t Ds3231_Read_Temp() {
    uint8_t buffer[2] = {0, 0};
    int16_t temp;

    Generic_i2c_Read(MSB_Temp_Reg, buffer, 2);

    temp = (int16_t) (((int8_t) buffer[0]) << 2);


    // if (temp >= 0) {
    //     temp += (buffer[1] >> 6) & 0x03;
    // }
    // else {
    //     temp -= (buffer[1] >> 6) & 0x03;
    // }

    temp |= (buffer[1] >> 6) & 0x03;
    return temp;
}

// Get integer part
int8_t Ds3231_Get_Temp_Integer(int16_t fixed_temp) { return (int8_t) (fixed_temp >> 2); }

// Get fractional part (returns 0, 25, 50, or 75)
uint8_t Ds3231_Get_Temp_Fraction(int16_t fixed_temp) {
    uint8_t temp_lookup[4] = {0, 25, 50, 75};

    return temp_lookup[(fixed_temp & 0x03)];
}
