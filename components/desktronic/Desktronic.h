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
            TENS_INVALID = 1,
            TENS_70 = 0x07,
            TENS_80 = 0x7F,
            TENS_90 = 0x6F,
            TENS_100 = 0x06,
        };

        enum Units : uint8_t
        {
            UNITS_INVALID = 1,
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
            DECIMAL_INVALID = 1,
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

        enum Id : uint8_t
        {
            ID_INVALID = 0,
            ID_72_0 = 0x22,
            ID_72_1 = 0xE9,
            ID_72_2 = 0x3E,
            ID_72_3 = 0x32,
            ID_72_4 = 0x49,
            ID_72_5 = 0x50,
            ID_72_6 = 0x60,
            ID_72_7 = 0xEA,
            ID_72_8 = 0x62,
            ID_72_9 = 0x52,
            ID_73_0 = 0x16,
            ID_73_1 = 0xDD,
            ID_73_2 = 0x32,
            ID_73_3 = 0x26,
            ID_73_4 = 0x3D,
            ID_73_5 = 0x44,
            ID_73_6 = 0x54,
            ID_73_7 = 0xDE,
            ID_73_8 = 0x56,
            ID_73_9 = 0x46,
            ID_74_0 = 0x2D,
            ID_74_1 = 0xF4,
            ID_74_2 = 0x49,
            ID_74_3 = 0x3D,
            ID_74_4 = 0x54,
            ID_74_5 = 0x5B,
            ID_74_6 = 0x6B,
            ID_74_7 = 0xF5,
            ID_74_8 = 0x6D,
            ID_74_9 = 0x5D,
            ID_75_0 = 0x34,
            ID_75_1 = 0xFB,
            ID_75_2 = 0x50,
            ID_75_3 = 0x44,
            ID_75_4 = 0x5B,
            ID_75_5 = 0x62,
            ID_75_6 = 0x72,
            ID_75_7 = 0xFC,
            ID_75_8 = 0x74,
            ID_75_9 = 0x64,
            ID_76_0 = 0x44,
            ID_76_1 = 0x0B,
            ID_76_2 = 0x60,
            ID_76_3 = 0x54,
            ID_76_4 = 0x6B,
            ID_76_5 = 0x72,
            ID_76_6 = 0x82,
            ID_76_7 = 0x0C,
            ID_76_8 = 0x84,
            ID_76_9 = 0x74,
            ID_77_0 = 0xCE,
            ID_77_1 = 0x95,
            ID_77_2 = 0xEA,
            ID_77_3 = 0xDE,
            ID_77_4 = 0xF5,
            ID_77_5 = 0xFC,
            ID_77_6 = 0x0C,
            ID_77_7 = 0x96,
            ID_77_8 = 0x0E,
            ID_77_9 = 0x01,
            ID_78_0 = 0x46,
            ID_78_1 = 0x0D,
            ID_78_2 = 0x62,
            ID_78_3 = 0x56,
            ID_78_4 = 0x6D,
            ID_78_5 = 0x74,
            ID_78_6 = 0x84,
            ID_78_7 = 0x0E,
            ID_78_8 = 0x86,
            ID_78_9 = 0x76,
            ID_79_0 = 0x36,
            ID_79_1 = 0xFD,
            ID_79_2 = 0x52,
            ID_79_3 = 0x46,
            ID_79_4 = 0x5D,
            ID_79_5 = 0x64,
            ID_79_6 = 0x74,
            ID_79_7 = 0xFE,
            ID_79_8 = 0x76,
            ID_79_9 = 0x66,
            ID_80_0 = 0x7E,
            ID_80_1 = 0x45,
            ID_80_2 = 0x9A,
            ID_80_3 = 0x8E,
            ID_80_4 = 0xA5,
            ID_80_5 = 0xAC,
            ID_80_6 = 0xBC,
            ID_80_7 = 0x46,
            ID_80_8 = 0xBE,
            ID_80_9 = 0xAE,
            ID_81_0 = 0x45,
            ID_81_1 = 0x0C,
            ID_81_2 = 0x61,
            ID_81_3 = 0x55,
            ID_81_4 = 0x6C,
            ID_81_5 = 0x73,
            ID_81_6 = 0x83,
            ID_81_7 = 0x0D,
            ID_81_8 = 0x0D,
            ID_81_9 = 0x75,
            ID_82_0 = 0x9A,
            ID_82_1 = 0x61,
            ID_82_2 = 0xB6,
            ID_82_3 = 0xAA,
            ID_82_4 = 0xC1,
            ID_82_5 = 0xC8,
            ID_82_6 = 0xD8,
            ID_82_7 = 0x62,
            ID_82_8 = 0xDA,
            ID_82_9 = 0xCA,
            ID_83_0 = 0x8E,
            ID_83_1 = 0x55,
            ID_83_2 = 0xAA,
            ID_83_3 = 0x9E,
            ID_83_4 = 0xB5,
            ID_83_5 = 0xBC,
            ID_83_6 = 0xCC,
            ID_83_7 = 0x56,
            ID_83_8 = 0xCE,
            ID_83_9 = 0xBE,
            ID_84_0 = 0xA5,
            ID_84_1 = 0x6C,
            ID_84_2 = 0xC1,
            ID_84_3 = 0xB5,
            ID_84_4 = 0xCC,
            ID_84_5 = 0xD3,
            ID_84_6 = 0xE3,
            ID_84_7 = 0x6D,
            ID_84_8 = 0xE5,
            ID_84_9 = 0xD5,
            ID_85_0 = 0xAC,
            ID_85_1 = 0x73,
            ID_85_2 = 0xC8,
            ID_85_3 = 0xBC,
            ID_85_4 = 0xD3,
            ID_85_5 = 0xDA,
            ID_85_6 = 0xEA,
            ID_85_7 = 0x74,
            ID_85_8 = 0xEC,
            ID_85_9 = 0xDC,
            ID_86_0 = 0xBC,
            ID_86_1 = 0x83,
            ID_86_2 = 0xD8,
            ID_86_3 = 0xCC,
            ID_86_4 = 0xE3,
            ID_86_5 = 0xEA,
            ID_86_6 = 0xFA,
            ID_86_7 = 0x84,
            ID_86_8 = 0xFC,
            ID_86_9 = 0xEC,
            ID_87_0 = 0x46,
            ID_87_1 = 0x0D,
            ID_87_2 = 0x62,
            ID_87_3 = 0x56,
            ID_87_4 = 0x6D,
            ID_87_5 = 0x74,
            ID_87_6 = 0x84,
            ID_87_7 = 0x0E,
            ID_87_8 = 0x86,
            ID_87_9 = 0x76,
            ID_88_0 = 0xBE,
            ID_88_1 = 0x85,
            ID_88_2 = 0xDA,
            ID_88_3 = 0xCE,
            ID_88_4 = 0xE5,
            ID_88_5 = 0xEC,
            ID_88_6 = 0xFC,
            ID_88_7 = 0x86,
            ID_88_8 = 0xFE,
            ID_88_9 = 0xEE,
            ID_89_0 = 0xAE,
            ID_89_1 = 0x75,
            ID_89_2 = 0xCA,
            ID_89_3 = 0xBE,
            ID_89_4 = 0xD5,
            ID_89_5 = 0xDC,
            ID_89_6 = 0xEC,
            ID_89_7 = 0x76,
            ID_89_8 = 0xEE,
            ID_89_9 = 0xDE,
            ID_90_0 = 0x6E,
            ID_90_1 = 0x35,
            ID_90_2 = 0x8A,
            ID_90_3 = 0x7E,
            ID_90_4 = 0x95,
            ID_90_5 = 0x9C,
            ID_90_6 = 0xAC,
            ID_90_7 = 0x36,
            ID_90_8 = 0xAE,
            ID_90_9 = 0x9E,
            ID_91_0 = 0x35,
            ID_91_1 = 0xFC,
            ID_91_2 = 0x45,
            ID_91_3 = 0x45,
            ID_91_4 = 0x5C,
            ID_91_5 = 0x63,
            ID_91_6 = 0x73,
            ID_91_7 = 0xFD,
            ID_91_8 = 0x75,
            ID_91_9 = 0x65,
            ID_92_0 = 0x8A,
            ID_92_1 = 0x51,
            ID_92_2 = 0xA6,
            ID_92_3 = 0x9A,
            ID_92_4 = 0xB1,
            ID_92_5 = 0xB8,
            ID_92_6 = 0xC8,
            ID_92_7 = 0x52,
            ID_92_8 = 0xCA,
            ID_92_9 = 0xBA,
            ID_93_0 = 0x7E,
            ID_93_1 = 0x45,
            ID_93_2 = 0x9A,
            ID_93_3 = 0x8E,
            ID_93_4 = 0xA5,
            ID_93_5 = 0xAC,
            ID_93_6 = 0xBC,
            ID_93_7 = 0x46,
            ID_93_8 = 0xBE,
            ID_93_9 = 0xAE,
            ID_94_0 = 0x95,
            ID_94_1 = 0x5C,
            ID_94_2 = 0xB1,
            ID_94_3 = 0xA5,
            ID_94_4 = 0xBC,
            ID_94_5 = 0xC3,
            ID_94_6 = 0xD3,
            ID_94_7 = 0x5D,
            ID_94_8 = 0xD5,
            ID_94_9 = 0xC5,
            ID_95_0 = 0x9C,
            ID_95_1 = 0x63,
            ID_95_2 = 0xB8,
            ID_95_3 = 0xAC,
            ID_95_4 = 0xC3,
            ID_95_5 = 0xCA,
            ID_95_6 = 0xDA,
            ID_95_7 = 0x64,
            ID_95_8 = 0xDC,
            ID_95_9 = 0xCC,
            ID_96_0 = 0xAC,
            ID_96_1 = 0x73,
            ID_96_2 = 0xC8,
            ID_96_3 = 0xBC,
            ID_96_4 = 0xD3,
            ID_96_5 = 0xDA,
            ID_96_6 = 0xEA,
            ID_96_7 = 0x74,
            ID_96_8 = 0xEC,
            ID_96_9 = 0xDC,
            ID_97_0 = 0x36,
            ID_97_1 = 0xFD,
            ID_97_2 = 0x52,
            ID_97_3 = 0x46,
            ID_97_4 = 0x5D,
            ID_97_5 = 0x64,
            ID_97_6 = 0x74,
            ID_97_7 = 0xFE,
            ID_97_8 = 0x76,
            ID_97_9 = 0x66,
            ID_98_0 = 0xAE,
            ID_98_1 = 0x75,
            ID_98_2 = 0xCA,
            ID_98_3 = 0xBE,
            ID_98_4 = 0xD5,
            ID_98_5 = 0xDC,
            ID_98_6 = 0xEC,
            ID_98_7 = 0x76,
            ID_98_8 = 0xEE,
            ID_98_9 = 0xDE,
            ID_99_0 = 0x9E,
            ID_99_1 = 0x65,
            ID_99_2 = 0xBA,
            ID_99_3 = 0xAE,
            ID_99_4 = 0xC5,
            ID_99_5 = 0xCC,
            ID_99_6 = 0xDC,
            ID_99_7 = 0x66,
            ID_99_8 = 0xDE,
            ID_99_9 = 0xCE,
            ID_100 = 0x85,
            ID_101 = 0x4C,
            ID_102 = 0xA1,
            ID_103 = 0x95,
            ID_104 = 0xAC,
            ID_105 = 0xB3,
            ID_106 = 0xC3,
            ID_107 = 0x4D,
            ID_108 = 0xC5,
            ID_109 = 0xB5,
            ID_110 = 0x4C,
            ID_111 = 0x13,
            ID_112 = 0x68,
            ID_113 = 0x5C,
            ID_114 = 0x73,
            ID_115 = 0x7A,
            ID_116 = 0x8A,
            ID_117 = 0x14,
            ID_118 = 0x8C,
            ID_119 = 0x7C,
        };

        const char *desktronicOperationToString(const DesktronicOperation operation);

        class Desktronic : public Component, public sensor::Sensor, public uart::UARTDevice
        {
        public:
            virtual float get_setup_priority() const override { return esphome::setup_priority::LATE; }
            virtual void setup() override;
            virtual void loop() override;
            void setLogConfig();

        public:
            void set_height_sensor(sensor::Sensor *sensor) { height_sensor_ = sensor; }
            void set_up_pin(GPIOPin *pin) { up_pin_ = pin; }
            void set_down_pin(GPIOPin *pin) { down_pin_ = pin; }
            void set_request_pin(GPIOPin *pin) { request_pin_ = pin; }
            void set_stopping_distance(const int distance) { stopping_distance_ = distance; }
            void set_timeout(const int timeout) { timeout_ = timeout; }

            void increase_height_by_0_1_cm();
            void decrease_height_by_0_1_cm();

            void move_to_position(const double targetPositionInCm);
            double get_delta_height(const double newHeight) const;
            bool is_moving_up(const double targetHeight) const;
            void stop();

            DesktronicOperation current_operation = DESKTRONIC_OPERATION_IDLE;

        private:
            int get_tens_digit(const uint8_t byte);
            int get_units_digit(const uint8_t byte);
            double get_first_decimal_digit(const uint8_t byte);
            Tens get_tens_byte_by_height(const double height) const;
            Units get_units_digit_by_height(const double height) const;
            FirstDecimal get_first_decimal_byte_by_height(const double height) const;
            Id get_id_by_height(const double height) const;
            bool is_skipping_garbage_byte(const uint8_t byte);
            void handle_byte(const uint8_t byte, int &bytePosition, double &height);

        protected:
            sensor::Sensor *height_sensor_ = nullptr;
            GPIOPin *up_pin_ = nullptr;
            GPIOPin *down_pin_ = nullptr;
            GPIOPin *request_pin_ = nullptr;

            int stopping_distance_;
            double current_pos_ = 0.0;
            double target_pos_ = -1.0;
            int timeout_ = -1;

            uint64_t start_time_ = 0;
            uint64_t request_time_ = 0;
        };

    } // namespace desktronic
} // namespace esphome