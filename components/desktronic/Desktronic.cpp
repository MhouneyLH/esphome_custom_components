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
    static uint8_t high_byte;

    while (esphome::uart::UARTDevice::available())
    {
        uint8_t readByte;
        int value;

        esphome::uart::UARTDevice::read_byte(&readByte);
        ESP_LOGI(TAG, "Received byte: 0x%x", readByte);

        //     switch (state)
        //     {
        //     case 0:
        //         if (readByte == 1)
        //         {
        //             state = 1;
        //         }

        //         break;
        //     case 1:
        //         if (readByte == 1)
        //         {
        //             state = 2;
        //         }
        //         else
        //         {
        //             state = 0;
        //         }

        //         break;
        //     case 2:
        //         high_byte = readByte;
        //         state = 3;

        //         break;
        //     case 3:
        //         value = (high_byte << 8) + readByte;
        //         current_pos_ = value;

        //         if (!height_sensor_)
        //         {
        //             height_sensor_->publish_state(value);
        //         }

        //         state = 0;
        //         break;
        //     case 4:
        //         break;
        //     case 5:
        //         break;
        //     default:
        //         break;
        //     }
        // }

        // if (target_pos_ >= 0)
        // {
        //     if ((abs(target_pos_ - current_pos_) < stopping_distance_) ||
        //         ((timeout_ >= 0) && (millis() - start_time_ >= timeout_)))
        //     {
        //         stop();
        //     }
        // }

        // if ((request_time_ > 0) && (millis() - request_time_ >= 100))
        // {
        //     request_pin_->digital_write(false);
        //     request_time_ = 0;
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
