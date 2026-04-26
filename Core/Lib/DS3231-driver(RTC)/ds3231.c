#include "ds3231.h"

void Generic_i2c_Send(uint8_t *date, uint8_t len) { i2c_write_timeout_us(i2c_bus, ds3231_add, date, len, false, 200); }

uint8_t Generic_i2c_Resive(uint8_t reg) {
    uint8_t data;

    i2c_write_timeout_us(i2c_bus, ds3231_add, &reg, 1, true, 200);
    i2c_read_timeout_us(i2c_bus, ds3231_add, &data, 1, false, 200);

    return data;
}

// void Ds3231_Set_INT_SQW_Pin(SquareWave_Ferq SquareWaveFerq, uint8_t status) {
//     uint8_t data[2] = {Control_Reg, 0x00};

//     if (status) {
//         data[1] &= ~(1 << 2);

//         switch (SquareWaveFerq) {
//             case _1hz :
//                 data[1] &= ~((1 << 4) | (1 << 3));

//                 break;

//             case _1_024hz :
//                 data[1] &= ~(1 << 4);
//                 data[1] |= (1 << 3);

//                 break;

//             case _4_096hz :
//                 data[1] &= ~(1 << 3);
//                 data[1] |= (1 << 4);

//                 break;

//             case _8_192hz :

//                 data[1] |= (1 << 2);

//                 break;
//         }
//     }
//     else {
//         data[1] &= ~(1 << 2);
//     }

//     Generic_i2c_Send(data, 2);
// }

void Ds3231_SetTime(ds3231_time *time, DateFormat format) {
    uint8_t data[2];
    data[0] = Seconds_Reg;
    data[1] = time->sec;

    Generic_i2c_Send(data, 2);

    data[0] = Status_Reg;

    i2c_write_timeout_us(i2c_bus, ds3231_add, &data[0], 1, true, 200);
    i2c_read_timeout_us(i2c_bus, ds3231_add, &data[1], 1, false, 200);

    data[1] = (data[1] & (~(1 << 7)));

    i2c_write_timeout_us(i2c_bus, ds3231_add, data, 2, false, 200);

    //     = Generic_i2c_Resive(Seconds_Reg);
    //     time->min  = Generic_i2c_Resive(Minutes_Reg);
    //     time->hour = Generic_i2c_Resive(Hours_Reg);
}
// void Ds3231_SetDate(ds3231_date *date, DateFormat format);

void Ds3231_GetTime(ds3231_time *time, DateFormat format) {
#ifdef _12hour_mode

    uint8_t data;

    if (format == Bin) {
        // Read seconds
        data      = Generic_i2c_Resive(Seconds_Reg);
        time->sec = (data & 0x0F);
        data      = (data & 0xF0) >> 4; // Use 0xF0 for upper nibble
        time->sec += data * 10;

        // Read minutes
        data      = Generic_i2c_Resive(Minutes_Reg);
        time->min = (data & 0x0F);
        data      = (data & 0x70) >> 4; // ✅ This is OK for minutes (bits 6-4)
        // But bits 7 and 6 should be 0 anyway
        time->min += data * 10;

        // Read hours
        data = Generic_i2c_Resive(Hours_Reg);

        // Check if 12-hour or 24-hour mode (bit 6)
        if (data & (1 << 6)) {
            // Check AM/PM (bit 5)
            if (data & (1 << 5))
                time->AM_PM = PM;
            else
                time->AM_PM = AM;

            // Get hours in 12-hour format (only bit 4 for tens)
            time->hour = (data & 0x0F);
            time->hour += ((data >> 4) & 0x01) * 10; // Only bit 4

            // Validate 12-hour range (1-12)
            if (time->hour == 0 || time->hour > 12) {
                time->hour = 12; // Fix invalid hour
            }
        }

        // if (format == Bin) {
        //     data = Generic_i2c_Resive(Seconds_Reg);

        //     time->sec = (data & 0x0f);
        //     data      = (data & 0xf0) >> 4;
        //     time->sec += data * 10;

        //     data = Generic_i2c_Resive(Minutes_Reg);

        //     time->min = (data & 0x0f);
        //     data      = (data & 0x70) >> 4;
        //     time->min += data * 10;

        //     data = Generic_i2c_Resive(Hours_Reg);

        //     if (data & (1 << 5))
        //         time->AM_PM = PM;
        //     else
        //         time->AM_PM = AM;

        //     time->hour = (data & 0x0f);

        //     time->hour += ((data & 0x10) >> 4) * 10;

        //     // data = ((data & 0x20) >> 5);

        //     // time->hour += data * 20;
        // }

        else {
            time->sec  = Generic_i2c_Resive(Seconds_Reg);
            time->min  = Generic_i2c_Resive(Minutes_Reg);
            time->hour = Generic_i2c_Resive(Hours_Reg);

            time->hour = Generic_i2c_Resive(Hours_Reg);

            if (time->hour & (1 << 5))
                time->AM_PM = PM;
            else
                time->AM_PM = AM;

            time->hour &= 0x1F;
        }
    }
#else

    uint8_t data;

    if (format == Bin) {
        data = Generic_i2c_Resive(Seconds_Reg);

        time->sec = (data & 0x0f);
        data      = (data & 0xf0) >> 4;
        time->sec += data * 10;

        data = Generic_i2c_Resive(Minutes_Reg);

        time->min = (data & 0x0f);
        data      = (data & 0x10) >> 4;
        time->min += data * 10;

        data = Generic_i2c_Resive(Hours_Reg);

        time->hour = (data & 0x0f);

        time->hour += ((data & 0x10) >> 4) * 10;

        data = ((data & 0x20) >> 5);

        time->hour += data * 20;
    }
    else {
        time->sec  = Generic_i2c_Resive(Seconds_Reg);
        time->min  = Generic_i2c_Resive(Minutes_Reg);
        time->hour = Generic_i2c_Resive(Hours_Reg);
    }
#endif
}

// void Ds3231_GetDate(ds3231_date *date, DateFormat format);

// void Ds3231_SetAlarm1(ds3231_Alarm1 *alarm1, Alarm1_RATE period);
// void Ds3231_SetAlarm2(ds3231_Alarm2 *alarm2, Alarm2_RATE period);

// void Ds3231_SetAlarm_Interrupt(Alarm alarm_num, uint8_t status) {
//     uint8_t data[2] = {Control_Reg, 0x00};

//     if (alarm_num == Alarm1) {
//         if (status) {
//             data[1] |= (1 << 0);
//         }
//         else {
//             data[1] &= ~(1 << 0);
//         }
//     }
//     else {
//         if (status) {
//             data[1] |= (1 << 1);
//         }
//         else {
//             data[1] &= ~(1 << 1);
//         }
//     }

//     Generic_i2c_Send(data, 2);
// }

// /// @brief After Intrupt is set user must clear the flag or the INT\SQW is set.
// void Ds3231_Reset_Alarm_Flag(Alarm alarm_num) {
//     uint8_t data[2] = {Status_Reg, 0x00};

//     if (alarm_num == Alarm1) {
//         data[1] &= ~(1 << 0);
//     }
//     else {
//         data[1] &= ~(1 << 1);
//     }

//     Generic_i2c_Send(data, 2);
// }

// uint8_t Ds3231_check_validity_timekeeping() {
//     uint8_t data;
//     uint8_t reg = Status_Reg;

//     Generic_i2c_Resive(reg, &data, 1);

//     return (data & (1 << 7));
// }

// void Ds3231_Set_32kHz_Output(uint8_t status) {
//     uint8_t data[2] = {Status_Reg, 0x00};

//     if (status) {
//         data[1] |= (1 << 3);
//     }
//     else {
//         data[1] &= ~(1 << 3);
//     }

//     Generic_i2c_Send(data, 2);
// }

// /**
//  * @brief to add to or subtract from the codes in the capacitance array registers.
//  *
//  * @param sign 0:
//  * @param value: 7 bit
//  */
// void Ds3231_Set_Aging_Offset(uint8_t sign, uint8_t value);

// uint16_t Ds3231_Read_Temp();
