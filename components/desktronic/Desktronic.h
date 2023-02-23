#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"
#include <iostream>

namespace esphome
{
namespace desktronic
{

enum DesktronicOperation : uint8_t
{
    DESKTRONIC_OPERATION_IDLE = 0U,
    DESKTRONIC_OPERATION_RAISING,
    DESKTRONIC_OPERATION_LOWERING,
};

enum Tens : uint8_t
{
    TENS_70 = 0x07,
    TENS_80 = 0x7F,
    TENS_90 = 0x6F,
    TENS_100 = 0x06,
};

enum Units : uint8_t
{
    UNITS_0 = 0xBF,
    UNITS_1 = 0x86,
    UNITS_2 = 0xDB,
    UNITS_3 = 0xCF,
    UNITS_4 = 0xE6,
    UNITS_5 = 0xED,
    UNITS_6 = 0xFD,
    UNITS_7 = 0x87,
    UNITS_8 = 0xFF,
    UNITS_9 = 0xEF,
};

enum FirstDecimal : uint8_t
{
    DECIMAL_0 = 0x3F,
    DECIMAL_1 = 0x06,
    DECIMAL_2 = 0x5B,
    DECIMAL_3 = 0x4F,
    DECIMAL_4 = 0x66,
    DECIMAL_5 = 0x6D,
    DECIMAL_6 = 0x7D,
    DECIMAL_7 = 0x07,
    DECIMAL_8 = 0x7F,
    DECIMAL_9 = 0x6F,
};

// enum Id : uint8_t
// {
//     _72_0 = 0x22,
//     _72_1 = 0xE9,
//     _72_2 = 0x3E,
//     _72_3 = 0x32,
//     _72_4 = 0x49,
//     _72_5 = 0x50,
//     _72_6 = 0x60,
//     _72_7 = 0xEA,
//     _72_8 = 0x62,
//     _72_9 = 0x52,
//     _73_0 = 0x16,
//     _73_1 = 0xDD,
//     _73_2 = 0x32,
//     _73_3 = 0x26,
//     _73_4 = 0x3D,
//     _73_5 = 0x44,
//     _73_6 = 0x54,
//     _73_7 = 0xDE,
//     _73_8 = 0x56,
//     _73_9 = 0x46,
//     _74_0 = 0x2D,
//     _74_1 = 0xF4,
//     _74_2 = 0x49,
//     _74_3 = 0x3D,
//     _74_4 = 0x54,
//     _74_5 = 0x5B,
//     _74_6 = 0x6B,
//     _74_7 = 0xF5,
//     _74_8 = 0x6D,
//     _74_9 = 0x5D,
//     _75_0 = 0x34,
//     _75_1 = 0xFB,
//     _75_2 = 0x50,
//     _75_3 = 0x44,
//     _75_4 = 0x5B,
//     _75_5 = 0x62,
//     _75_6 = 0x72,
//     _75_7 = 0xFC,
//     _75_8 = 0x74,
//     _75_9 = 0x64,
//     _76_0 = 0x44,
//     _76_1 = 0x0B,
//     _76_2 = 0x60,
//     _76_3 = 0x54,
//     _76_4 = 0x6B,
//     _76_5 = 0x72,
//     _76_6 = 0x82,
//     _76_7 = 0x0C,
//     _76_8 = 0x84,
//     _76_9 = 0x74,
//     _77_0 = 0xCE,
//     _77_1 = 0x95,
//     _77_2 = 0xEA,
//     _77_3 = 0xDE,
//     _77_4 = 0xF5,
//     _77_5 = 0xFC,
//     _77_6 = 0x0C,
//     _77_7 = 0x96,
//     _77_8 = 0x0E,
//     _77_9 = 0x01,
//     _78_0 = 0x46,
//     _78_1 = 0x0D,
//     _78_2 = 0x62,
//     _78_3 = 0x56,
//     _78_4 = 0x6D,
//     _78_5 = 0x74,
//     _78_6 = 0x84,
//     _78_7 = 0x0E,
//     _78_8 = 0x86,
//     _78_9 = 0x76,
//     _79_0 = 0x36,
//     _79_1 = 0xFD,
//     _79_2 = 0x52,
//     _79_3 = 0x46,
//     _79_4 = 0x5D,
//     _79_5 = 0x64,
//     _79_6 = 0x74,
//     _79_7 = 0xFE,
//     _79_8 = 0x76,
//     _79_9 = 0x66,
//     _80_0 = 0x7E,
//     _80_1 = 0x45,
//     _80_2 = 0x9A,
//     _80_3 = 0x8E,
//     _80_4 = 0xA5,
//     _80_5 = 0xAC,
//     _80_6 = 0xBC,
//     _80_7 = 0x46,
//     _80_8 = 0xBE,
//     _80_9 = 0xAE,
//     _81_0 = 0x45,
//     _81_1 = 0x0C,
//     _81_2 = 0x61,
//     _81_3 = 0x55,
//     _81_4 = 0x6C,
//     _81_5 = 0x73,
//     _81_6 = 0x83,
//     _81_7 = 0x0D,
//     _81_8 = 0x0D,
//     _81_9 = 0x75,
//     _82_0 = 0x9A,
//     _82_1 = 0x61,
//     _82_2 = 0xB6,
//     _82_3 = 0xAA,
//     _82_4 = 0xC1,
//     _82_5 = 0xC8,
//     _82_6 = 0xD8,
//     _82_7 = 0x62,
//     _82_8 = 0xDA,
//     _82_9 = 0xCA,
//     _83_0 = 0x8E,
//     _83_1 = 0x55,
//     _83_2 = 0xAA,
//     _83_3 = 0x9E,
//     _83_4 = 0xB5,
//     _83_5 = 0xBC,
//     _83_6 = 0xCC,
//     _83_7 = 0x56,
//     _83_8 = 0xCE,
//     _83_9 = 0xBE,
//     _84_0 = 0xA5,
//     _84_1 = 0x6C,
//     _84_2 = 0xC1,
//     _84_3 = 0xB5,
//     _84_4 = 0xCC,
//     _84_5 = 0xD3,
//     _84_6 = 0xE3,
//     _84_7 = 0x6D,
//     _84_8 = 0xE5,
//     _84_9 = 0xD5,
//     _85_0 = 0xAC,
//     _85_1 = 0x73,
//     _85_2 = 0xC8,
//     _85_3 = 0xBC,
//     _85_4 = 0xD3,
//     _85_5 = 0xDA,
//     _85_6 = 0xEA,
//     _85_7 = 0x74,
//     _85_8 = 0xEC,
//     _85_9 = 0xDC,
//     _86_0 = 0xBC,
//     _86_1 = 0x83,
//     _86_2 = 0xD8,
//     _86_3 = 0xCC,
//     _86_4 = 0xE3,
//     _86_5 = 0xEA,
//     _86_6 = 0xFA,
//     _86_7 = 0x84,
//     _86_8 = 0xFC,
//     _86_9 = 0xEC,
//     _87_0 = 0x46,
//     _87_1 = 0x0D,
//     _87_2 = 0x62,
//     _87_3 = 0x56,
//     _87_4 = 0x6D,
//     _87_5 = 0x74,
//     _87_6 = 0x84,
//     _87_7 = 0x0E,
//     _87_8 = 0x86,
//     _87_9 = 0x76,
//     _88_0 = 0xBE,
//     _88_1 = 0x85,
//     _88_2 = 0xDA,
//     _88_3 = 0xCE,
//     _88_4 = 0xE5,
//     _88_5 = 0xEC,
//     _88_6 = 0xFC,
//     _88_7 = 0x86,
//     _88_8 = 0xFE,
//     _88_9 = 0xEE,
//     _89_0 = 0xAE,
//     _89_1 = 0x75,
//     _89_2 = 0xCA,
//     _89_3 = 0xBE,
//     _89_4 = 0xD5,
//     _89_5 = 0xDC,
//     _89_6 = 0xEC,
//     _89_7 = 0x76,
//     _89_8 = 0xEE,
//     _89_9 = 0xDE,
//     _90_0 = 0x6E,
//     _90_1 = 0x35,
//     _90_2 = 0x8A,
//     _90_3 = 0x7E,
//     _90_4 = 0x95,
//     _90_5 = 0x9C,
//     _90_6 = 0xAC,
//     _90_7 = 0x36,
//     _90_8 = 0xAE,
//     _90_9 = 0x9E,
//     _91_0 = 0x35,
//     _91_1 = 0xFC,
//     _91_2 = 0x45,
//     _91_3 = 0x45,
//     _91_4 = 0x5C,
//     _91_5 = 0x63,
//     _91_6 = 0x73,
//     _91_7 = 0xFD,
//     _91_8 = 0x75,
//     _91_9 = 0x65,
//     _92_0 = 0x8A,
//     _92_1 = 0x51,
//     _92_2 = 0xA6,
//     _92_3 = 0x9A,
//     _92_4 = 0xB1,
//     _92_5 = 0xB8,
//     _92_6 = 0xC8,
//     _92_7 = 0x52,
//     _92_8 = 0xCA,
//     _92_9 = 0xBA,
//     _93_0 = 0x7E,
//     _93_1 = 0x45,
//     _93_2 = 0x9A,
//     _93_3 = 0x8E,
//     _93_4 = 0xA5,
//     _93_5 = 0xAC,
//     _93_6 = 0xBC,
//     _93_7 = 0x46,
//     _93_8 = 0xBE,
//     _93_9 = 0xAE,
//     _94_0 = 0x95,
//     _94_1 = 0x5C,
//     _94_2 = 0xB1,
//     _94_3 = 0xA5,
//     _94_4 = 0xBC,
//     _94_5 = 0xC3,
//     _94_6 = 0xD3,
//     _94_7 = 0x5D,
//     _94_8 = 0xD5,
//     _94_9 = 0xC5,
//     _95_0 = 0x9C,
//     _95_1 = 0x63,
//     _95_2 = 0xB8,
//     _95_3 = 0xAC,
//     _95_4 = 0xC3,
//     _95_5 = 0xCA,
//     _95_6 = 0xDA,
//     _95_7 = 0x64,
//     _95_8 = 0xDC,
//     _95_9 = 0xCC,
//     _96_0 = 0xAC,
//     _96_1 = 0x73,
//     _96_2 = 0xC8,
//     _96_3 = 0xBC,
//     _96_4 = 0xD3,
//     _96_5 = 0xDA,
//     _96_6 = 0xEA,
//     _96_7 = 0x74,
//     _96_8 = 0xEC,
//     _96_9 = 0xDC,
//     _97_0 = 0x36,
//     _97_1 = 0xFD,
//     _97_2 = 0x52,
//     _97_3 = 0x46,
//     _97_4 = 0x5D,
//     _97_5 = 0x64,
//     _97_6 = 0x74,
//     _97_7 = 0xFE,
//     _97_8 = 0x76,
//     _97_9 = 0x66,
//     _98_0 = 0xAE,
//     _98_1 = 0x75,
//     _98_2 = 0xCA,
//     _98_3 = 0xBE,
//     _98_4 = 0xD5,
//     _98_5 = 0xDC,
//     _98_6 = 0xEC,
//     _98_7 = 0x76,
//     _98_8 = 0xEE,
//     _98_9 = 0xDE,
//     _99_0 = 0x9E,
//     _99_1 = 0x65,
//     _99_2 = 0xBA,
//     _99_3 = 0xAE,
//     _99_4 = 0xC5,
//     _99_5 = 0xCC,
//     _99_6 = 0xDC,
//     _99_7 = 0x66,
//     _99_8 = 0xDE,
//     _99_9 = 0xCE,
//     _100 = 0x85,
//     _101 = 0x4C,
//     _102 = 0xA1,
//     _103 = 0x95,
//     _104 = 0xAC,
//     _105 = 0xB3,
//     _106 = 0xC3,
//     _107 = 0x4D,
//     _108 = 0xC5,
//     _109 = 0xB5,
//     _110 = 0x4C,
//     _111 = 0x13,
//     _112 = 0x68,
//     _113 = 0x5C,
//     _114 = 0x73,
//     _115 = 0x7A,
//     _116 = 0x8A,
//     _117 = 0x14,
//     _118 = 0x8C,
//     _119 = 0x7C,
// };

const char* desktronicOperationToString(const DesktronicOperation operation);

class Desktronic : public Component
    , public sensor::Sensor
    , public uart::UARTDevice
{
public:
    virtual float get_setup_priority() const override { return esphome::setup_priority::LATE; }
    virtual void setup() override;
    virtual void loop() override;
    void setLogConfig();

public:
    void set_height_sensor(sensor::Sensor* sensor) { height_sensor_ = sensor; }
    void set_up_pin(GPIOPin* pin) { up_pin_ = pin; }
    void set_down_pin(GPIOPin* pin) { down_pin_ = pin; }
    void set_request_pin(GPIOPin* pin) { request_pin_ = pin; }
    void set_stopping_distance(const int distance) { stopping_distance_ = distance; }
    void set_timeout(const int timeout) { timeout_ = timeout; }

    void move_to_position(const int targetPosition);
    void stop();

    DesktronicOperation current_operation = DESKTRONIC_OPERATION_IDLE;

private:
    int get_tens_digit(const uint8_t byte);
    int get_units_digit(const uint8_t byte);
    double get_first_decimal_digit(const uint8_t byte);
    bool is_skipping_garbage_byte(const uint8_t byte);
    void handle_byte(const uint8_t byte, int& bytePosition, double& height);

protected:
    sensor::Sensor* height_sensor_ = nullptr;
    GPIOPin* up_pin_ = nullptr;
    GPIOPin* down_pin_ = nullptr;
    GPIOPin* request_pin_ = nullptr;

    int stopping_distance_;
    double current_pos_ = 0.0;
    double target_pos_ = -1.0;
    int timeout_ = -1;

    uint64_t start_time_ = 0;
    uint64_t request_time_ = 0;
};

} // namespace desktronic
} // namespace esphome