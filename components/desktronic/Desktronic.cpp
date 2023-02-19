#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace desktronic
{

static const char* TAG = "desktronic";
static const unsigned int UART_MESSAGE_LENGTH = 6U;

const char* desktronicOperationToString(const DesktronicOperation operation)
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
    if (!up_pin_)
    {
        up_pin_->digital_write(false);
    }

    if (!down_pin_)
    {
        down_pin_->digital_write(false);
    }

    if (!request_pin_)
    {
        request_pin_->digital_write(true);
        request_time_ = esphome::millis();
    }
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

bool Desktronic::is_skipping_garbage_byte(const uint8_t byte)
{
    return byte != 0x5A;
}

void Desktronic::handle_byte(const uint8_t byte, int& bytePosition, double& height)
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
        ESP_LOGI(TAG, "Current Height: %f", height);

        bytePosition = 0;
        height = 0.0;

        break;
    default:
        break;
    }
}

void Desktronic::loop()
{
    static int bytePositionInUARTMessage = 0;
    static double height = 0.0;

    while (esphome::uart::UARTDevice::available())
    {
        uint8_t byte;
        esphome::uart::UARTDevice::read_byte(&byte);
        if (is_skipping_garbage_byte(byte))
        {
            continue;
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

void Desktronic::move_to_position(const int targetPosition)
{
    if (abs(targetPosition - current_pos_) < stopping_distance_)
    {
        return;
    }

    if (targetPosition > current_pos_)
    {
        if (up_pin_ == nullptr)
        {
            return;
        }
        up_pin_->digital_write(true);
        current_operation = DESKTRONIC_OPERATION_RAISING;
    }
    else
    {
        if (down_pin_ == nullptr)
        {
            return;
        }

        down_pin_->digital_write(true);
        current_operation = DESKTRONIC_OPERATION_LOWERING;
    }

    target_pos_ = targetPosition;

    if (timeout_ >= 0)
    {
        start_time_ = esphome::millis();
    }
}

void Desktronic::stop()
{
    target_pos_ = -1;

    if (!up_pin_)
    {
        up_pin_->digital_write(false);
    }

    if (!down_pin_)
    {
        down_pin_->digital_write(false);
    }

    current_operation = DESKTRONIC_OPERATION_IDLE;
}

} // namespace desktronic
} // namespace esphome
