#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace desktronic
    {

        static const char *TAG = "desktronic";
        static const unsigned int UART_MESSAGE_LENGTH = 6U;
        static const double DESKTRONIC_MOVE_STEP = 0.1;
        static const uint8_t UART_MESSAGE_START = 0x5A;
        static const uint8_t UART_MESSAGE_BEFORE_CHECKSUM = 0x01;

        const char *desktronicOperationToString(const DesktronicOperation operation)
        {
            switch (operation)
            {
            case DESKTRONIC_OPERATION_IDLE:
                return "IDLE";
            case DESKTRONIC_OPERATION_RAISING:
                return "RAISING";
            case DESKTRONIC_OPERATION_LOWERING:
                return "LOWERING";
            default:
                return "UNKNOWN";
            }
        }

        void Desktronic::setup()
        {
            if (up_pin_)
            {
                up_pin_->digital_write(false);
            }

            if (down_pin_)
            {
                down_pin_->digital_write(false);
            }

            if (request_pin_)
            {
                request_pin_->digital_write(true);
                request_time_ = esphome::millis();
            }
        }

        void Desktronic::loop()
        {
            static int bytePositionInUARTMessage = 0;
            static double height = 0.0;
            bool beginning_skipping_garbage_bytes = true;

            while (esphome::uart::UARTDevice::available())
            {
                uint8_t byte;
                esphome::uart::UARTDevice::read_byte(&byte);
                ESP_LOGI(TAG, "Received byte: 0x%x", byte);

                if (beginning_skipping_garbage_bytes)
                {
                    if (is_skipping_garbage_byte(byte))
                    {
                        continue;
                    }
                    else
                    {
                        beginning_skipping_garbage_bytes = false;
                        bytePositionInUARTMessage = 0;
                        height = 0.0;
                    }
                }

                handle_byte(byte, bytePositionInUARTMessage, height);
            }
        }

        void Desktronic::setLogConfig()
        {
            ESP_LOGCONFIG(TAG, "DesktronicDesk:");

            LOG_SENSOR("", "Height", height_sensor_);
            LOG_PIN("UpPin: ", up_pin_);
            LOG_PIN("DownPin: ", down_pin_);
            LOG_PIN("RequestPin: ", request_pin_);
        }

        void Desktronic::increase_height_by_0_1_cm()
        {
            ESP_LOGI(TAG, "Increase height bei 0.1cm");

            if (!up_pin_)
            {
                ESP_LOGI(TAG, "Increase height bei 0.1cm failed. No up pin.");
                return;
            }

            up_pin_->digital_write(true);
            current_operation = DESKTRONIC_OPERATION_RAISING;

            if (!uart::UARTDevice::available())
            {
                ESP_LOGI(TAG, "UART not available for writing increase height by 0.1cm");
                return;
            }

            esphome::uart::UARTDevice::write_byte(0xA5);
            esphome::uart::UARTDevice::write_byte(0x00);
            esphome::uart::UARTDevice::write_byte(0x20);
            esphome::uart::UARTDevice::write_byte(0xDF);
            esphome::uart::UARTDevice::write_byte(0xFF);

            target_pos_ = current_pos_ + DESKTRONIC_MOVE_STEP;

            ESP_LOGI(TAG, "Increase height bei 0.1cm finished");

            // ------------------------------
            // ESP_LOGI(TAG, "Increase height bei 0.1cm");

            // if (!up_pin_)
            // {
            //     ESP_LOGI(TAG, "Increase height bei 0.1cm failed. No up pin.");
            //     return;
            // }

            // up_pin_->digital_write(true);
            // current_operation = DESKTRONIC_OPERATION_RAISING;

            // target_pos_ = current_pos_ + DESKTRONIC_MOVE_STEP;

            // const Tens tens = get_tens_byte_by_height(target_pos_);
            // const Units units = get_units_digit_by_height(target_pos_);
            // const FirstDecimal first_decimal = get_first_decimal_byte_by_height(target_pos_);
            // const Id id = get_id_by_height(target_pos_);

            // ESP_LOGI(TAG, "Target Height: %f", target_pos_);
            // ESP_LOGI(TAG, "Tens: 0x%x; Units: 0x%x; FirstDecimal: 0x%x; Id: 0x%x", tens, units, first_decimal, id);

            // if (!uart::UARTDevice::available())
            // {
            //     ESP_LOGI(TAG, "UART not available for writing increase height by 0.1cm");
            //     return;
            // }

            // esphome::uart::UARTDevice::write_byte(UART_MESSAGE_START);
            // esphome::uart::UARTDevice::write_byte(tens);
            // esphome::uart::UARTDevice::write_byte(units);
            // esphome::uart::UARTDevice::write_byte(first_decimal);
            // esphome::uart::UARTDevice::write_byte(UART_MESSAGE_BEFORE_CHECKSUM);
            // esphome::uart::UARTDevice::write_byte(id);

            // ESP_LOGI(TAG, "Increase height bei 0.1cm finished");
        }

        void Desktronic::decrease_height_by_0_1_cm()
        {
            ESP_LOGI(TAG, "Decrease height bei 0.1cm");

            if (!down_pin_)
            {
                ESP_LOGI(TAG, "Decrease height bei 0.1cm failed. No down pin.");
                return;
            }

            down_pin_->digital_write(true);
            current_operation = DESKTRONIC_OPERATION_LOWERING;

            target_pos_ = current_pos_ - DESKTRONIC_MOVE_STEP;
            ESP_LOGI(TAG, "Decrease height bei 0.1cm finished");
        }

        void Desktronic::move_to_position(const double targetPositionInCm)
        {
            // should not move if the stopping distance is exceeded
            ESP_LOGI(TAG, "1 Moving to position: %f", targetPositionInCm);
            if (get_delta_height(targetPositionInCm) < stopping_distance_)
            {
                return;
            }

            if (is_moving_up(targetPositionInCm))
            {
                ESP_LOGI(TAG, "up Moving to position: %f", targetPositionInCm);
                if (!up_pin_)
                {
                    return;
                }
                ESP_LOGI(TAG, "up 1 Moving to position: %f", targetPositionInCm);

                up_pin_->digital_write(true);
                current_operation = DESKTRONIC_OPERATION_RAISING;
            }
            else
            {
                ESP_LOGI(TAG, "down Moving to position: %f", targetPositionInCm);
                if (!down_pin_)
                {
                    return;
                }

                ESP_LOGI(TAG, "down 1 Moving to position: %f", targetPositionInCm);
                down_pin_->digital_write(true);
                current_operation = DESKTRONIC_OPERATION_LOWERING;
            }

            ESP_LOGI(TAG, "end Moving to position: %f", targetPositionInCm);

            // @todo: @future-me what the hell does this do??
            target_pos_ = targetPositionInCm;

            if (timeout_ >= 0)
            {
                start_time_ = esphome::millis();
            }
        }

        double Desktronic::get_delta_height(const double newHeight) const
        {
            return abs(newHeight - current_pos_);
        }

        bool Desktronic::is_moving_up(const double targetHeight) const
        {
            return targetHeight > current_pos_;
        }

        void Desktronic::stop()
        {
            target_pos_ = -1;

            if (up_pin_)
            {
                up_pin_->digital_write(false);
            }

            if (down_pin_)
            {
                down_pin_->digital_write(false);
            }

            current_operation = DESKTRONIC_OPERATION_IDLE;
        }

        int Desktronic::get_tens_digit(const uint8_t byte)
        {
            switch (byte)
            {
            case Tens::TENS_70:
                return 70;
            case Tens::TENS_80:
                return 80;
            case Tens::TENS_90:
                return 90;
            case Tens::TENS_100:
                return 100;
            default:
                return -1;
            }
        }

        int Desktronic::get_units_digit(const uint8_t byte)
        {
            switch (byte)
            {
            case Units::UNITS_0:
                return 0;
            case Units::UNITS_1:
                return 1;
            case Units::UNITS_2:
                return 2;
            case Units::UNITS_3:
                return 3;
            case Units::UNITS_4:
                return 4;
            case Units::UNITS_5:
                return 5;
            case Units::UNITS_6:
                return 6;
            case Units::UNITS_7:
                return 7;
            case Units::UNITS_8:
                return 8;
            case Units::UNITS_9:
                return 9;
            default:
                return -1;
            }
        }

        double Desktronic::get_first_decimal_digit(const uint8_t byte)
        {
            switch (byte)
            {
            case FirstDecimal::DECIMAL_0:
                return 0.0;
            case FirstDecimal::DECIMAL_1:
                return 0.1;
            case FirstDecimal::DECIMAL_2:
                return 0.2;
            case FirstDecimal::DECIMAL_3:
                return 0.3;
            case FirstDecimal::DECIMAL_4:
                return 0.4;
            case FirstDecimal::DECIMAL_5:
                return 0.5;
            case FirstDecimal::DECIMAL_6:
                return 0.6;
            case FirstDecimal::DECIMAL_7:
                return 0.7;
            case FirstDecimal::DECIMAL_8:
                return 0.8;
            case FirstDecimal::DECIMAL_9:
                return 0.9;
            default:
                return -1.0;
            }
        }

        Id Desktronic::get_id_by_height(const double height) const
        {
            if (height == 72.0)
            {
                return Id::ID_72_0;
            }
            else if (height == 72.1)
            {
                return Id::ID_72_1;
            }
            else if (height == 72.2)
            {
                return Id::ID_72_2;
            }
            else if (height == 72.3)
            {
                return Id::ID_72_3;
            }
            else if (height == 72.4)
            {
                return Id::ID_72_4;
            }
            else if (height == 72.5)
            {
                return Id::ID_72_5;
            }
            else if (height == 72.6)
            {
                return Id::ID_72_6;
            }
            else if (height == 72.7)
            {
                return Id::ID_72_7;
            }
            else if (height == 72.8)
            {
                return Id::ID_72_8;
            }
            else if (height == 72.9)
            {
                return Id::ID_72_9;
            }
            else if (height == 73.0)
            {
                return Id::ID_73_0;
            }
            else if (height == 73.1)
            {
                return Id::ID_73_1;
            }
            else if (height == 73.2)
            {
                return Id::ID_73_2;
            }
            else if (height == 73.3)
            {
                return Id::ID_73_3;
            }
            else if (height == 73.4)
            {
                return Id::ID_73_4;
            }
            else if (height == 73.5)
            {
                return Id::ID_73_5;
            }
            else if (height == 73.6)
            {
                return Id::ID_73_6;
            }
            else if (height == 73.7)
            {
                return Id::ID_73_7;
            }
            else if (height == 73.8)
            {
                return Id::ID_73_8;
            }
            else if (height == 73.9)
            {
                return Id::ID_73_9;
            }
            else if (height == 74.0)
            {
                return Id::ID_74_0;
            }
            else if (height == 74.1)
            {
                return Id::ID_74_1;
            }
            else if (height == 74.2)
            {
                return Id::ID_74_2;
            }
            else if (height == 74.3)
            {
                return Id::ID_74_3;
            }
            else if (height == 74.4)
            {
                return Id::ID_74_4;
            }
            else if (height == 74.5)
            {
                return Id::ID_74_5;
            }
            else if (height == 74.6)
            {
                return Id::ID_74_6;
            }
            else if (height == 74.7)
            {
                return Id::ID_74_7;
            }
            else if (height == 74.8)
            {
                return Id::ID_74_8;
            }
            else if (height == 74.9)
            {
                return Id::ID_74_9;
            }
            else if (height == 75.0)
            {
                return Id::ID_75_0;
            }
            else if (height == 75.1)
            {
                return Id::ID_75_1;
            }
            else if (height == 75.2)
            {
                return Id::ID_75_2;
            }
            else if (height == 75.3)
            {
                return Id::ID_75_3;
            }
            else if (height == 75.4)
            {
                return Id::ID_75_4;
            }
            else if (height == 75.5)
            {
                return Id::ID_75_5;
            }
            else if (height == 75.6)
            {
                return Id::ID_75_6;
            }
            else if (height == 75.7)
            {
                return Id::ID_75_7;
            }
            else if (height == 75.8)
            {
                return Id::ID_75_8;
            }
            else if (height == 75.9)
            {
                return Id::ID_75_9;
            }
            else if (height == 76.0)
            {
                return Id::ID_76_0;
            }
            else if (height == 76.1)
            {
                return Id::ID_76_1;
            }
            else if (height == 76.2)
            {
                return Id::ID_76_2;
            }
            else if (height == 76.3)
            {
                return Id::ID_76_3;
            }
            else if (height == 76.4)
            {
                return Id::ID_76_4;
            }
            else if (height == 76.5)
            {
                return Id::ID_76_5;
            }
            else if (height == 76.6)
            {
                return Id::ID_76_6;
            }
            else if (height == 76.7)
            {
                return Id::ID_76_7;
            }
            else if (height == 76.8)
            {
                return Id::ID_76_8;
            }
            else if (height == 76.9)
            {
                return Id::ID_76_9;
            }
            else if (height == 77.0)
            {
                return Id::ID_77_0;
            }
            else if (height == 77.1)
            {
                return Id::ID_77_1;
            }
            else if (height == 77.2)
            {
                return Id::ID_77_2;
            }
            else if (height == 77.3)
            {
                return Id::ID_77_3;
            }
            else if (height == 77.4)
            {
                return Id::ID_77_4;
            }
            else if (height == 77.5)
            {
                return Id::ID_77_5;
            }
            else if (height == 77.6)
            {
                return Id::ID_77_6;
            }
            else if (height == 77.7)
            {
                return Id::ID_77_7;
            }
            else if (height == 77.8)
            {
                return Id::ID_77_8;
            }
            else if (height == 77.9)
            {
                return Id::ID_77_9;
            }
            else if (height == 78.0)
            {
                return Id::ID_78_0;
            }
            else if (height == 78.1)
            {
                return Id::ID_78_1;
            }
            else if (height == 78.2)
            {
                return Id::ID_78_2;
            }
            else if (height == 78.3)
            {
                return Id::ID_78_3;
            }
            else if (height == 78.4)
            {
                return Id::ID_78_4;
            }
            else if (height == 78.5)
            {
                return Id::ID_78_5;
            }
            else if (height == 78.6)
            {
                return Id::ID_78_6;
            }
            else if (height == 78.7)
            {
                return Id::ID_78_7;
            }
            else if (height == 78.8)
            {
                return Id::ID_78_8;
            }
            else if (height == 78.9)
            {
                return Id::ID_78_9;
            }
            else if (height == 79.0)
            {
                return Id::ID_79_0;
            }
            else if (height == 79.1)
            {
                return Id::ID_79_1;
            }
            else if (height == 79.2)
            {
                return Id::ID_79_2;
            }
            else if (height == 79.3)
            {
                return Id::ID_79_3;
            }
            else if (height == 79.4)
            {
                return Id::ID_79_4;
            }
            else if (height == 97.5)
            {
                return Id::ID_79_5;
            }
            else if (height == 79.6)
            {
                return Id::ID_79_6;
            }
            else if (height == 79.7)
            {
                return Id::ID_79_7;
            }
            else if (height == 79.8)
            {
                return Id::ID_79_8;
            }
            else if (height == 79.9)
            {
                return Id::ID_79_9;
            }
            else if (height == 80.0)
            {
                return Id::ID_80_0;
            }
            else if (height == 80.1)
            {
                return Id::ID_80_1;
            }
            else if (height == 80.2)
            {
                return Id::ID_80_2;
            }
            else if (height == 80.3)
            {
                return Id::ID_80_3;
            }
            else if (height == 80.4)
            {
                return Id::ID_80_4;
            }
            else if (height == 80.5)
            {
                return Id::ID_80_5;
            }
            else if (height == 80.6)
            {
                return Id::ID_80_6;
            }
            else if (height == 80.7)
            {
                return Id::ID_80_7;
            }
            else if (height == 80.8)
            {
                return Id::ID_80_8;
            }
            else if (height == 80.9)
            {
                return Id::ID_80_9;
            }
            else if (height == 81.0)
            {
                return Id::ID_81_0;
            }
            else if (height == 81.1)
            {
                return Id::ID_81_1;
            }
            else if (height == 81.2)
            {
                return Id::ID_81_2;
            }
            else if (height == 81.3)
            {
                return Id::ID_81_3;
            }
            else if (height == 81.4)
            {
                return Id::ID_81_4;
            }
            else if (height == 81.5)
            {
                return Id::ID_81_5;
            }
            else if (height == 81.6)
            {
                return Id::ID_81_6;
            }
            else if (height == 81.7)
            {
                return Id::ID_81_7;
            }
            else if (height == 81.8)
            {
                return Id::ID_81_8;
            }
            else if (height == 81.9)
            {
                return Id::ID_81_9;
            }
            else if (height == 82.0)
            {
                return Id::ID_82_0;
            }
            else if (height == 82.1)
            {
                return Id::ID_82_1;
            }
            else if (height == 82.2)
            {
                return Id::ID_82_2;
            }
            else if (height == 82.3)
            {
                return Id::ID_82_3;
            }
            else if (height == 82.4)
            {
                return Id::ID_82_4;
            }
            else if (height == 82.5)
            {
                return Id::ID_82_5;
            }
            else if (height == 82.6)
            {
                return Id::ID_82_6;
            }
            else if (height == 82.7)
            {
                return Id::ID_82_7;
            }
            else if (height == 82.8)
            {
                return Id::ID_82_8;
            }
            else if (height == 82.9)
            {
                return Id::ID_82_9;
            }
            else if (height == 83.0)
            {
                return Id::ID_83_0;
            }
            else if (height == 83.1)
            {
                return Id::ID_83_1;
            }
            else if (height == 83.2)
            {
                return Id::ID_83_2;
            }
            else if (height == 83.3)
            {
                return Id::ID_83_3;
            }
            else if (height == 83.4)
            {
                return Id::ID_83_4;
            }
            else if (height == 83.5)
            {
                return Id::ID_83_5;
            }
            else if (height == 83.6)
            {
                return Id::ID_83_6;
            }
            else if (height == 83.7)
            {
                return Id::ID_83_7;
            }
            else if (height == 83.8)
            {
                return Id::ID_83_8;
            }
            else if (height == 83.9)
            {
                return Id::ID_83_9;
            }
            else if (height == 84.0)
            {
                return Id::ID_84_0;
            }
            else if (height == 84.1)
            {
                return Id::ID_84_1;
            }
            else if (height == 84.2)
            {
                return Id::ID_84_2;
            }
            else if (height == 84.3)
            {
                return Id::ID_84_3;
            }
            else if (height == 84.4)
            {
                return Id::ID_84_4;
            }
            else if (height == 84.5)
            {
                return Id::ID_84_5;
            }
            else if (height == 84.6)
            {
                return Id::ID_84_6;
            }
            else if (height == 84.7)
            {
                return Id::ID_84_7;
            }
            else if (height == 84.8)
            {
                return Id::ID_84_8;
            }
            else if (height == 84.9)
            {
                return Id::ID_84_9;
            }
            else if (height == 85.0)
            {
                return Id::ID_85_0;
            }
            else if (height == 85.1)
            {
                return Id::ID_85_1;
            }
            else if (height == 85.2)
            {
                return Id::ID_85_2;
            }
            else if (height == 85.3)
            {
                return Id::ID_85_3;
            }
            else if (height == 85.4)
            {
                return Id::ID_85_4;
            }
            else if (height == 85.5)
            {
                return Id::ID_85_5;
            }
            else if (height == 85.6)
            {
                return Id::ID_85_6;
            }
            else if (height == 85.7)
            {
                return Id::ID_85_7;
            }
            else if (height == 85.8)
            {
                return Id::ID_85_8;
            }
            else if (height == 85.9)
            {
                return Id::ID_85_9;
            }
            else if (height == 86.0)
            {
                return Id::ID_86_0;
            }
            else if (height == 86.1)
            {
                return Id::ID_86_1;
            }
            else if (height == 86.2)
            {
                return Id::ID_86_2;
            }
            else if (height == 86.3)
            {
                return Id::ID_86_3;
            }
            else if (height == 86.4)
            {
                return Id::ID_86_4;
            }
            else if (height == 86.5)
            {
                return Id::ID_86_5;
            }
            else if (height == 86.6)
            {
                return Id::ID_86_6;
            }
            else if (height == 86.7)
            {
                return Id::ID_86_7;
            }
            else if (height == 86.8)
            {
                return Id::ID_86_8;
            }
            else if (height == 86.9)
            {
                return Id::ID_86_9;
            }
            else if (height == 87.0)
            {
                return Id::ID_87_0;
            }
            else if (height == 87.1)
            {
                return Id::ID_87_1;
            }
            else if (height == 87.2)
            {
                return Id::ID_87_2;
            }
            else if (height == 87.3)
            {
                return Id::ID_87_3;
            }
            else if (height == 87.4)
            {
                return Id::ID_87_4;
            }
            else if (height == 87.5)
            {
                return Id::ID_87_5;
            }
            else if (height == 87.6)
            {
                return Id::ID_87_6;
            }
            else if (height == 87.7)
            {
                return Id::ID_87_7;
            }
            else if (height == 87.8)
            {
                return Id::ID_87_8;
            }
            else if (height == 87.9)
            {
                return Id::ID_87_9;
            }
            else if (height == 88.0)
            {
                return Id::ID_88_0;
            }
            else if (height == 88.1)
            {
                return Id::ID_88_1;
            }
            else if (height == 88.2)
            {
                return Id::ID_88_2;
            }
            else if (height == 88.3)
            {
                return Id::ID_88_3;
            }
            else if (height == 88.4)
            {
                return Id::ID_88_4;
            }
            else if (height == 88.5)
            {
                return Id::ID_88_5;
            }
            else if (height == 88.6)
            {
                return Id::ID_88_6;
            }
            else if (height == 88.7)
            {
                return Id::ID_88_7;
            }
            else if (height == 88.8)
            {
                return Id::ID_88_8;
            }
            else if (height == 88.9)
            {
                return Id::ID_88_9;
            }
            else if (height == 89.0)
            {
                return Id::ID_89_0;
            }
            else if (height == 89.1)
            {
                return Id::ID_89_1;
            }
            else if (height == 89.2)
            {
                return Id::ID_89_2;
            }
            else if (height == 89.3)
            {
                return Id::ID_89_3;
            }
            else if (height == 89.4)
            {
                return Id::ID_89_4;
            }
            else if (height == 89.5)
            {
                return Id::ID_89_5;
            }
            else if (height == 89.6)
            {
                return Id::ID_89_6;
            }
            else if (height == 89.7)
            {
                return Id::ID_89_7;
            }
            else if (height == 89.8)
            {
                return Id::ID_89_8;
            }
            else if (height == 89.9)
            {
                return Id::ID_89_9;
            }
            else if (height == 90.0)
            {
                return Id::ID_90_0;
            }
            else if (height == 90.1)
            {
                return Id::ID_90_1;
            }
            else if (height == 90.2)
            {
                return Id::ID_90_2;
            }
            else if (height == 90.3)
            {
                return Id::ID_90_3;
            }
            else if (height == 90.4)
            {
                return Id::ID_90_4;
            }
            else if (height == 90.5)
            {
                return Id::ID_90_5;
            }
            else if (height == 90.6)
            {
                return Id::ID_90_6;
            }
            else if (height == 90.7)
            {
                return Id::ID_90_7;
            }
            else if (height == 90.8)
            {
                return Id::ID_90_8;
            }
            else if (height == 90.9)
            {
                return Id::ID_90_9;
            }
            else if (height == 91.0)
            {
                return Id::ID_91_0;
            }
            else if (height == 91.1)
            {
                return Id::ID_91_1;
            }
            else if (height == 91.2)
            {
                return Id::ID_91_2;
            }
            else if (height == 91.3)
            {
                return Id::ID_91_3;
            }
            else if (height == 91.4)
            {
                return Id::ID_91_4;
            }
            else if (height == 91.5)
            {
                return Id::ID_91_5;
            }
            else if (height == 91.6)
            {
                return Id::ID_91_6;
            }
            else if (height == 91.7)
            {
                return Id::ID_91_7;
            }
            else if (height == 91.8)
            {
                return Id::ID_91_8;
            }
            else if (height == 91.9)
            {
                return Id::ID_91_9;
            }
            else if (height == 92.0)
            {
                return Id::ID_92_0;
            }
            else if (height == 92.1)
            {
                return Id::ID_92_1;
            }
            else if (height == 92.2)
            {
                return Id::ID_92_2;
            }
            else if (height == 92.3)
            {
                return Id::ID_92_3;
            }
            else if (height == 92.4)
            {
                return Id::ID_92_4;
            }
            else if (height == 92.5)
            {
                return Id::ID_92_5;
            }
            else if (height == 92.6)
            {
                return Id::ID_92_6;
            }
            else if (height == 92.7)
            {
                return Id::ID_92_7;
            }
            else if (height == 92.8)
            {
                return Id::ID_92_8;
            }
            else if (height == 92.9)
            {
                return Id::ID_92_9;
            }
            else if (height == 93.0)
            {
                return Id::ID_93_0;
            }
            else if (height == 93.1)
            {
                return Id::ID_93_1;
            }
            else if (height == 93.2)
            {
                return Id::ID_93_2;
            }
            else if (height == 93.3)
            {
                return Id::ID_93_3;
            }
            else if (height == 93.4)
            {
                return Id::ID_93_4;
            }
            else if (height == 93.5)
            {
                return Id::ID_93_5;
            }
            else if (height == 93.6)
            {
                return Id::ID_93_6;
            }
            else if (height == 93.7)
            {
                return Id::ID_93_7;
            }
            else if (height == 93.8)
            {
                return Id::ID_93_8;
            }
            else if (height == 93.9)
            {
                return Id::ID_93_9;
            }
            else if (height == 94.0)
            {
                return Id::ID_94_0;
            }
            else if (height == 94.1)
            {
                return Id::ID_94_1;
            }
            else if (height == 94.2)
            {
                return Id::ID_94_2;
            }
            else if (height == 94.3)
            {
                return Id::ID_94_3;
            }
            else if (height == 94.4)
            {
                return Id::ID_94_4;
            }
            else if (height == 94.5)
            {
                return Id::ID_94_5;
            }
            else if (height == 94.6)
            {
                return Id::ID_94_6;
            }
            else if (height == 94.7)
            {
                return Id::ID_94_7;
            }
            else if (height == 94.8)
            {
                return Id::ID_94_8;
            }
            else if (height == 94.9)
            {
                return Id::ID_94_9;
            }
            else if (height == 95.0)
            {
                return Id::ID_95_0;
            }
            else if (height == 95.1)
            {
                return Id::ID_95_1;
            }
            else if (height == 95.2)
            {
                return Id::ID_95_2;
            }
            else if (height == 95.3)
            {
                return Id::ID_95_3;
            }
            else if (height == 95.4)
            {
                return Id::ID_95_4;
            }
            else if (height == 95.5)
            {
                return Id::ID_95_5;
            }
            else if (height == 95.6)
            {
                return Id::ID_95_6;
            }
            else if (height == 95.7)
            {
                return Id::ID_95_7;
            }
            else if (height == 95.8)
            {
                return Id::ID_95_8;
            }
            else if (height == 95.9)
            {
                return Id::ID_95_9;
            }
            else if (height == 96.0)
            {
                return Id::ID_96_0;
            }
            else if (height == 96.1)
            {
                return Id::ID_96_1;
            }
            else if (height == 96.2)
            {
                return Id::ID_96_2;
            }
            else if (height == 96.3)
            {
                return Id::ID_96_3;
            }
            else if (height == 96.4)
            {
                return Id::ID_96_4;
            }
            else if (height == 96.5)
            {
                return Id::ID_96_5;
            }
            else if (height == 96.6)
            {
                return Id::ID_96_6;
            }
            else if (height == 96.7)
            {
                return Id::ID_96_7;
            }
            else if (height == 96.7)
            {
                return Id::ID_96_8;
            }
            else if (height == 96.8)
            {
                return Id::ID_96_9;
            }
            else if (height == 96.9)
            {
                return Id::ID_97_0;
            }
            else if (height == 97.1)
            {
                return Id::ID_97_1;
            }
            else if (height == 97.2)
            {
                return Id::ID_97_2;
            }
            else if (height == 97.3)
            {
                return Id::ID_97_3;
            }
            else if (height == 97.4)
            {
                return Id::ID_97_4;
            }
            else if (height == 97.5)
            {
                return Id::ID_97_5;
            }
            else if (height == 97.6)
            {
                return Id::ID_97_6;
            }
            else if (height == 97.7)
            {
                return Id::ID_97_7;
            }
            else if (height == 97.8)
            {
                return Id::ID_97_8;
            }
            else if (height == 97.9)
            {
                return Id::ID_97_9;
            }
            else if (height == 98.0)
            {
                return Id::ID_98_0;
            }
            else if (height == 98.1)
            {
                return Id::ID_98_1;
            }
            else if (height == 98.2)
            {
                return Id::ID_98_2;
            }
            else if (height == 98.3)
            {
                return Id::ID_98_3;
            }
            else if (height == 98.4)
            {
                return Id::ID_98_4;
            }
            else if (height == 98.5)
            {
                return Id::ID_98_5;
            }
            else if (height == 98.6)
            {
                return Id::ID_98_6;
            }
            else if (height == 98.7)
            {
                return Id::ID_98_7;
            }
            else if (height == 98.8)
            {
                return Id::ID_98_8;
            }
            else if (height == 98.9)
            {
                return Id::ID_98_9;
            }
            else if (height == 99.0)
            {
                return Id::ID_99_0;
            }
            else if (height == 99.1)
            {
                return Id::ID_99_1;
            }
            else if (height == 99.2)
            {
                return Id::ID_99_2;
            }
            else if (height == 99.3)
            {
                return Id::ID_99_3;
            }
            else if (height == 99.4)
            {
                return Id::ID_99_4;
            }
            else if (height == 99.5)
            {
                return Id::ID_99_5;
            }
            else if (height == 99.6)
            {
                return Id::ID_99_6;
            }
            else if (height == 99.7)
            {
                return Id::ID_99_7;
            }
            else if (height == 99.8)
            {
                return Id::ID_99_8;
            }
            else if (height == 99.9)
            {
                return Id::ID_99_9;
            }
            else if (height == 100.0)
            {
                return Id::ID_100;
            }
            else if (height == 100)
            {
                return Id::ID_100;
            }
            else if (height == 101)
            {
                return Id::ID_101;
            }
            else if (height == 102)
            {
                return Id::ID_102;
            }
            else if (height == 103)
            {
                return Id::ID_103;
            }
            else if (height == 104)
            {
                return Id::ID_104;
            }
            else if (height == 105)
            {
                return Id::ID_105;
            }
            else if (height == 106)
            {
                return Id::ID_106;
            }
            else if (height == 107)
            {
                return Id::ID_107;
            }
            else if (height == 108)
            {
                return Id::ID_108;
            }
            else if (height == 109)
            {
                return Id::ID_109;
            }
            else if (height == 110)
            {
                return Id::ID_110;
            }
            else if (height == 111)
            {
                return Id::ID_111;
            }
            else if (height == 112)
            {
                return Id::ID_112;
            }
            else if (height == 113)
            {
                return Id::ID_113;
            }
            else if (height == 114)
            {
                return Id::ID_114;
            }
            else if (height == 115)
            {
                return Id::ID_115;
            }
            else if (height == 116)
            {
                return Id::ID_116;
            }
            else if (height == 117)
            {
                return Id::ID_117;
            }
            else if (height == 118)
            {
                return Id::ID_118;
            }
            else if (height == 119)
            {
                return Id::ID_119;
            }
            else
            {
                return Id::ID_INVALID;
            }
        }

        Tens Desktronic::get_tens_byte_by_height(const double height) const
        {
            const int pureDigit = static_cast<int>(height) / 10;
            switch (pureDigit)
            {
            case 7:
                return Tens::TENS_70;
            case 8:
                return Tens::TENS_80;
            case 9:
                return Tens::TENS_90;
            case 10:
                return Tens::TENS_100;
            default:
                return Tens::TENS_INVALID;
            }
        }

        Units Desktronic::get_units_digit_by_height(const double height) const
        {
            const int pureDigit = static_cast<int>(height) % 10;
            switch (pureDigit)
            {
            case 0:
                return Units::UNITS_0;
            case 1:
                return Units::UNITS_1;
            case 2:
                return Units::UNITS_2;
            case 3:
                return Units::UNITS_3;
            case 4:
                return Units::UNITS_4;
            case 5:
                return Units::UNITS_5;
            case 6:
                return Units::UNITS_6;
            case 7:
                return Units::UNITS_7;
            case 8:
                return Units::UNITS_8;
            case 9:
                return Units::UNITS_9;
            default:
                return Units::UNITS_INVALID;
            }
        }

        FirstDecimal Desktronic::get_first_decimal_byte_by_height(const double height) const
        {
            const int pureDigit = static_cast<int>(height * 10) % 10;
            switch (pureDigit)
            {
            case 0:
                return FirstDecimal::DECIMAL_0;
            case 1:
                return FirstDecimal::DECIMAL_1;
            case 2:
                return FirstDecimal::DECIMAL_2;
            case 3:
                return FirstDecimal::DECIMAL_3;
            case 4:
                return FirstDecimal::DECIMAL_4;
            case 5:
                return FirstDecimal::DECIMAL_5;
            case 6:
                return FirstDecimal::DECIMAL_6;
            case 7:
                return FirstDecimal::DECIMAL_7;
            case 8:
                return FirstDecimal::DECIMAL_8;
            case 9:
                return FirstDecimal::DECIMAL_9;
            default:
                return FirstDecimal::DECIMAL_INVALID;
            }
        }

        bool Desktronic::is_skipping_garbage_byte(const uint8_t byte)
        {
            return byte != UART_MESSAGE_START;
        }

        void Desktronic::handle_byte(const uint8_t byte, int &bytePosition, double &height)
        {
            switch (bytePosition)
            {
            case 0:
                bytePosition = 1;
                break;
            case 1:
                height += get_tens_digit(byte);
                bytePosition = 2;
                break;
            case 2:
                height += get_units_digit(byte);
                bytePosition = 3;
                break;
            case 3:
                height += get_first_decimal_digit(byte);
                bytePosition = 4;
                break;
            case 4:
                bytePosition = 5;
                break;
            case 5:
                current_pos_ = height;
                ESP_LOGI(TAG, "-----");
                // if (height_sensor_)
                // {
                //     // accuracy is set in __init__.py
                //     height_sensor_->publish_state(height);
                // }

                bytePosition = 0;
                height = 0.0;

                break;
            default:
                break;
            }
        }

    } // namespace desktronic
} // namespace esphome
