#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace desktronic
{

static const char* TAG = "desktronic";
static const unsigned int UART_MESSAGE_LENGTH = 6U;
static const double DESKTRONIC_MOVE_STEP = 0.1;
static const uint8_t UART_MESSAGE_START = 0x5A;

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

    target_pos_ = current_pos_ + DESKTRONIC_MOVE_STEP;
    ESP_LOGI(TAG, "Increase height bei 0.1cm finished");
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

bool Desktronic::is_skipping_garbage_byte(const uint8_t byte)
{
    return byte != UART_MESSAGE_START;
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
        current_pos_ = height;
        if (height_sensor_)
        {
            // accuracy is set in __init__.py
            height_sensor_->publish_state(height);
        }

        bytePosition = 0;
        height = 0.0;

        break;
    default:
        break;
    }
}

} // namespace desktronic
} // namespace esphome
