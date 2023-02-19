#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace desktronic
{

static const char* TAG = "desktronic";

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

void Desktronic::loop()
{
    static int state = 0;
    static double height = 0.0;
    bool skipGarbage = true;

    while (esphome::uart::UARTDevice::available())
    {
        uint8_t byte;
        esphome::uart::UARTDevice::read_byte(&byte);

        if (skipGarbage)
        {
            if (byte == 0x5A)
            {
                state = 0;
                height = 0;
                skipGarbage = false;
            }
            else
            {
                continue;
            }
        }

        ESP_LOGI(TAG, "Byte %d: 0x%x", state, byte);

        switch (state)
        {
        case 0:
            state = 1;
            break;
        case 1:
            state = 2;
            switch (byte)
            {
            case Tens::TENS_70:
                height = 70;
                break;
            case Tens::TENS_80:
                height = 80;
                break;
            case Tens::TENS_90:
                height = 90;
                break;
            case Tens::TENS_100:
                height = 100;
                break;
            default:
                break;
            }
            break;
        case 2:
            state = 3;
            switch (byte)
            {
            case Units::UNITS_0:
                break;
            case Units::UNITS_1:
                height += 1;
                break;
            case Units::UNITS_2:
                height += 2;
                break;
            case Units::UNITS_3:
                height += 3;
                break;
            case Units::UNITS_4:
                height += 4;
                break;
            case Units::UNITS_5:
                height += 5;
                break;
            case Units::UNITS_6:
                height += 6;
                break;
            case Units::UNITS_7:
                height += 7;
                break;
            case Units::UNITS_8:
                height += 8;
                break;
            case Units::UNITS_9:
                height += 9;
                break;
            default:
                break;
            }
            break;
        case 3:
            state = 4;
            switch (byte)
            {
            case FirstDecimal::DECIMAL_0:
                break;
            case FirstDecimal::DECIMAL_1:
                height += 0.1;
                break;
            case FirstDecimal::DECIMAL_2:
                height += 0.2;
                break;
            case FirstDecimal::DECIMAL_3:
                height += 0.3;
                break;
            case FirstDecimal::DECIMAL_4:
                height += 0.4;
                break;
            case FirstDecimal::DECIMAL_5:
                height += 0.5;
                break;
            case FirstDecimal::DECIMAL_6:
                height += 0.6;
                break;
            case FirstDecimal::DECIMAL_7:
                height += 0.7;
                break;
            case FirstDecimal::DECIMAL_8:
                height += 0.8;
                break;
            case FirstDecimal::DECIMAL_9:
                height += 0.9;
                break;
            default:
                break;
            }
            break;
        case 4:
            state = 5;
            break;
        case 5:
            ESP_LOGI(TAG, "Received last byte: 0x%x", byte);
            ESP_LOGI(TAG, "Current Height: %f", height);

            state = 0;
            height = 0.0;

            break;
        default:
            break;
        }
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
