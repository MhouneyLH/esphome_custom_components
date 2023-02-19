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

    while (esphome::uart::UARTDevice::available())
    {
        uint8_t byte;
        esphome::uart::UARTDevice::read_byte(&byte);

        switch (state)
        {
        case 0:
            state = 1;
            break;
        case 1:
            state = 2;
            // height += tens_map.at(static_cast<Tens>(byte));
            break;
        case 2:
            state = 3;
            // height += units_map.at(static_cast<Units>(byte));
            break;
        case 3:
            state = 4;
            // height += first_decimal_map.at(static_cast<FirstDecimal>(byte));
            break;
        case 4:
            state = 5;
            break;
        case 5:
            state = 0;
            ESP_LOGI(TAG, "Received last byte: 0x%x", byte);
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
