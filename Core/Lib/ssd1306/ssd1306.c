#include "ssd1306.h"

byte Buffer[BufferSize] = {};

void SendLcd(byte Register, byte Data) {
    byte I2CBuffer[2] = {Register, Data};
    i2c_write_blocking(I2Ch, LcdAddres, I2CBuffer, 2, false);
}

void SetXY(uint8_t x, uint8_t y) {
    SendLcd(Command_Reg, Column_Address_Reg);
    SendLcd(Command_Reg, x);
    SendLcd(Command_Reg, Column_end_address);

    SendLcd(Command_Reg, Page_Address_Reg);
    SendLcd(Command_Reg, y);
    SendLcd(Command_Reg, Page_end_address);
}

void ClearLcd(uint8_t x, uint8_t y) {
    int Aria = (((x + 1) * (y + 1)) << 3);

    SetXY(x, y);

    for (int i = 0; i < Aria; i++) {
        SendLcd(Data_Reg, 0);
    }
}

void SetPixel(uint8_t x, uint8_t y, uint8_t Status) {
    SetXY(x, (y / 8));

    if (Status)
        SendLcd(Data_Reg, (1 << (y % 8)));

    else
        SendLcd(Data_Reg, ~(1 << (y % 8)));
}

void LCD_init(void) {

    SendLcd(Command_Reg, Display_OFF);


    SendLcd(Command_Reg, Contrast_Reg);
    SendLcd(Command_Reg, Contrast_Value);

    SendLcd(Command_Reg, Memory_Address_Reg);
    SendLcd(Command_Reg, Set_Memory_Horizental_Mode);

    SendLcd(Command_Reg, Normal_Display);

    SendLcd(Command_Reg, Display_Offset_Reg);
    SendLcd(Command_Reg, Display_Offset_Value);

    SendLcd(Command_Reg, Display_Oscillator_Frequency_Reg);
    SendLcd(Command_Reg, Display_Oscillator_Frequency_Value);

#ifdef LCD_128x32
    SendLcd(Command_Reg, Display_COM_Pins_Hardware_Config_Reg);
    SendLcd(Command_Reg, Display_COM_Pins_Hardware_Config_Value);
#endif

#ifdef ROTATE_180
    SendLcd(Command_Reg, 0XA1); // FLEEP 180 DEG
    SendLcd(Command_Reg, 0XC8); // FLEEP 180 DEG
#endif
    SendLcd(Command_Reg, Entir_Display_OFF);

    SetXY(Column_start_address, Page_start_address);

    SendLcd(Command_Reg, Charge_Pump_Reg);
    SendLcd(Command_Reg, Charge_Pump_Enable);

    SendLcd(Command_Reg, Display_ON);
}

void BufferSetPixel(uint8_t XA, uint8_t YA, uint8_t Status, uint8_t *buffer) {
    int Index = (((XA * 8) + ((YA >> 3) * 128 * 8)) >> 3);

    if (Status)
        buffer[Index] |= (0x01 << (YA % 8));

    else
        buffer[Index] &= ~(0x01 << (YA % 8));
}

void Bitmap(uint8_t x, uint8_t y, uint8_t *Data, uint16_t wight, uint16_t hight) {
    char k = 0;

    if (hight % 8) {
        k = (hight / 8) + 1;
    }
    else
        k = (hight / 8);

    for (int i = 0; i < k; i++) {
        SetXY(x, i + y);

        for (int x = 0; x < wight; x++) {
            SendLcd(Data_Reg, *Data);
            Data++;
        }
    }
}

void FillBuffer(uint8_t XOffset, uint8_t YOffset, uint8_t *Data, uint8_t Width, uint8_t Height, uint8_t *buffer) {
    uint8_t ORGX = XOffset;
    uint8_t Row  = (Width >> 3);
    uint16_t Index;

    for (uint8_t Y = 0; Y < Height; Y++) {
        for (uint8_t X = 0; X < Row; X++) {
            for (uint8_t Bit = 0; Bit < 8; Bit++) {
                Index = (XOffset + (YOffset * 128)) >> 3;

                if (*Data & (0x80 >> Bit))
                    buffer[Index] |= (0x80 >> (XOffset % 8));
                else
                    buffer[Index] &= ~(0x80 >> (XOffset % 8));

                XOffset++;
            }
            Data++;
        }
        YOffset++;
        XOffset = ORGX;
    }
}

void SendBuffer(uint8_t *buffer) {
    SetXY(0, 0);
    for (int x = 0; x < BufferSize; x++) {
        SendLcd(Data_Reg, *buffer++);
    }
}

void ClearBuffer(uint8_t *buffer) {
    for (int x = 0; x < BufferSize; x++) {
        *buffer = 0;
        buffer++;
    }
}

void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color, uint8_t *buffer) {
    int16_t adx = (((x1 >= x0) ? x1 - x0 : x0 - x1) + 1) << 1;
    int16_t ady = (((y1 >= y0) ? y1 - y0 : y0 - y1) + 1) << 1;

    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;

    int16_t eps;

    if (adx > ady) {
        eps = (ady - adx) >> 1;

        for (int16_t x = x0, y = y0; sx < 0 ? x >= x1 : x <= x1; x += sx) {
            setpixel(x, y, color, buffer);

            eps += ady;
            if (eps << 1 >= adx) {
                y += sy;
                eps -= adx;
            }
        }
    }
    else {
        eps = (adx - ady) >> 1;
        for (int16_t x = x0, y = y0; sy < 0 ? y >= y1 : y <= y1; y += sy) {
            setpixel(x, y, color, buffer);

            eps += adx;
            if (eps << 1 >= ady) {
                x += sx;
                eps -= ady;
            }
        }
    }
}

void DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t filled, uint8_t color, uint8_t *buffer) {
    if (x1 > x2) {
        x1 = x1 ^ x2;
        x2 = x1 ^ x2;
        x1 = x1 ^ x2;
    }
    if (y1 > y2) {
        y1 = y1 ^ y2;
        y2 = y1 ^ y2;
        y1 = y1 ^ y2;
    }

    x1 = (x1 > 127) ? x1 = 127 : x1;
    x2 = (x2 > 127) ? x2 = 127 : x2;

    y1 = (y1 > 63) ? y1 = 63 : y1;
    y2 = (y2 > 63) ? y2 = 63 : y2;

    if (filled) {
        for (uint8_t y = y1; y <= y2; y++) {
            for (uint8_t x = x1; x <= x2; x++) {
                BufferSetPixel(x, y, 1, buffer);
            }
        }
    }

    else {
        DrawLine(x1, y1, x2, y1, color, buffer); // Top
        DrawLine(x1, y2, x2, y2, color, buffer); // Bottom
        DrawLine(x1, y1, x1, y2, color, buffer); // Left
        DrawLine(x2, y1, x2, y2, color, buffer); // Right
    }
}

void DrawCircleQuarter(uint8_t x, uint8_t y, uint8_t radius, uint8_t quadrant, uint8_t color, uint8_t *buffer) {
    uint8_t j = 0;
    // Simple implementation - you can enhance this with proper circle algorithm
    for (uint8_t i = 0; i <= radius; i++) {
        j = radius - i;
        if (quadrant & 1)
            BufferSetPixel(x - j, y - i, color, buffer); // Top-left
        if (quadrant & 2)
            BufferSetPixel(x + j, y - i, color, buffer); // Top-right
        if (quadrant & 4)
            BufferSetPixel(x + j, y + i, color, buffer); // Bottom-right
        if (quadrant & 8)
            BufferSetPixel(x - j, y + i, color, buffer); // Bottom-left
    }
}

void DrawRoundedRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t radius, uint8_t color, uint8_t *buffer) {
    // Draw straight sections
    DrawLine(x1 + radius, y1, x2 - radius, y1, color, buffer); // Top
    DrawLine(x1 + radius, y2, x2 - radius, y2, color, buffer); // Bottom
    DrawLine(x1, y1 + radius, x1, y2 - radius, color, buffer); // Left
    DrawLine(x2, y1 + radius, x2, y2 - radius, color, buffer); // Right

    // Draw rounded corners (simplified with small arcs)
    // Top-left corner
    DrawCircleQuarter((x1 + radius), (y1 + radius), radius, 1, color, buffer);
    // Top-right corner
    DrawCircleQuarter(x2 - radius, y1 + radius, radius, 2, color, buffer);
    // Bottom-right corner
    DrawCircleQuarter(x2 - radius, y2 - radius, radius, 4, color, buffer);
    // Bottom-left corner
    DrawCircleQuarter(x1 + radius, y2 - radius, radius, 8, color, buffer);
}

/**
 * Draw a circle (filled or outline)
 * @param x0: Center X coordinate
 * @param y0: Center Y coordinate
 * @param radius: Circle radius
 * @param filled: 0 for outline, 1 for filled
 * @param buffer: Display buffer
 */
void DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint8_t filled, uint8_t color, uint8_t *buffer) {
    int16_t x   = radius;
    int16_t y   = 0;
    int16_t err = 1 - radius;

    while (x >= y) {
        if (filled) {
            DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color, buffer);
            DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color, buffer);

            if (x != y) {
                DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color, buffer);
                DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color, buffer);
            }
        }
        else {
            BufferSetPixel(x0 + x, y0 + y, 1, buffer);
            BufferSetPixel(x0 - x, y0 + y, 1, buffer);
            BufferSetPixel(x0 + x, y0 - y, 1, buffer);
            BufferSetPixel(x0 - x, y0 - y, 1, buffer);

            BufferSetPixel(x0 + y, y0 + x, 1, buffer);
            BufferSetPixel(x0 - y, y0 + x, 1, buffer);
            BufferSetPixel(x0 + y, y0 - x, 1, buffer);
            BufferSetPixel(x0 - y, y0 - x, 1, buffer);
        }

        y++;
        if (err < 0) {
            err += 2 * y + 1;
        }
        else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}
