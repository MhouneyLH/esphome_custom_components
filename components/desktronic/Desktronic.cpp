#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace desktronic
{

static const char* TAG = "desktronic";
static const uint8_t REMOTE_UART_MESSAGE_LENGTH = 5U;
static const uint8_t REMOTE_UART_MESSAGE_START = 0xa5;
static const uint8_t DESK_UART_MESSAGE_LENGTH = 6U;
static const uint8_t DESK_UART_MESSAGE_START = 0x5a;

const char* desktronic_operation_to_string(const DesktronicOperation operation)
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

static int segment_to_number(const uint8_t segment)
{
    switch (segment & 0x7f)
    {
    case SEGMENT_0:
        return 0;
    case SEGMENT_1:
        return 1;
    case SEGMENT_2:
        return 2;
    case SEGMENT_3:
        return 3;
    case SEGMENT_4:
        return 4;
    case SEGMENT_5:
        return 5;
    case SEGMENT_6:
        return 6;
    case SEGMENT_7:
        return 7;
    case SEGMENT_8:
        return 8;
    case SEGMENT_9:
        return 9;
    default:
        ESP_LOGE(TAG, "unknown digit: %02f", segment & 0x7f);
    }

    return -1;
}

void Desktronic::setup()
{
    if (move_pin_ != nullptr)
    {
        move_pin_->digital_write(false);
    }
}

void Desktronic::move_to(const float height_in_cm)
{
    if (height_in_cm < 72.0 || height_in_cm > 119.0)
    {
        ESP_LOGE(TAG, "Moving: Height must be between 720 and 1190 mm");
        return;
    }

    if (move_pin_ == nullptr)
    {
        ESP_LOGE(TAG, "Moving: Move pin is not configured");
        return;
    }

    ESP_LOGE(TAG, "current_height: %f cm", current_height_);
}

void Desktronic::read_remote_uart()
{
    if (remote_uart_ == nullptr)
    {
        return;
    }

    uint8_t byte;
    while (remote_uart_->available())
    {
        remote_uart_->read_byte(&byte);

        // are we at the first-message and have to filter
        // some unnecessary bytes before the actual REMOTE_UART_MESSAGE_START?
        if (!remote_rx_)
        {
            if (byte != REMOTE_UART_MESSAGE_START)
            {
                continue;
            }

            remote_rx_ = true;
            continue;
        }

        remote_buffer_.push_back(byte);

        // -1, because of the start byte
        // important for the right order of the bytes
        if (remote_buffer_.size() < REMOTE_UART_MESSAGE_LENGTH - 1)
        {
            continue;
        }

        remote_rx_ = false;
        uint8_t* data = remote_buffer_.data();

        const uint8_t checksum = data[1] + data[2];
        if (checksum != data[3])
        {
            ESP_LOGE(TAG, "remote checksum mismatch: %02x (calculated checksum) != %02x (actual checksum)", checksum, data[3]);
            remote_buffer_.clear();

            continue;
        }

        publish_remote_states(data[1]);
        remote_buffer_.clear();
    }
}

void Desktronic::read_desk_uart()
{
    if (desk_uart_ == nullptr)
    {
        return;
    }

    uint8_t byte;
    while (desk_uart_->available())
    {
        desk_uart_->read_byte(&byte);

        // are we at the first-message and have to filter
        // some unnecessary bytes before the actual DESK_UART_MESSAGE_START?
        if (!desk_rx_)
        {
            if (byte != DESK_UART_MESSAGE_START)
            {
                continue;
            }

            desk_buffer_.clear();
            desk_buffer_.resize(0);

            desk_rx_ = true;
            continue;
        }

        desk_buffer_.push_back(byte);

        // -1, because of the start byte
        // important for the right order of the bytes
        if (desk_buffer_.size() < DESK_UART_MESSAGE_LENGTH - 1)
        {
            continue;
        }

        desk_rx_ = false;
        uint8_t* data = desk_buffer_.data();

        const uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        if (checksum != data[4])
        {
            ESP_LOGE(TAG, "desk checksum mismatch: %02x != %02x", checksum, data[4]);
            desk_buffer_.clear();
            desk_buffer_.resize(0);

            continue;
        }

        if (height_sensor_ != nullptr)
        {
            if (data[3] != 0x01)
            {
                ESP_LOGE(TAG, "unknown message type %02x must be 0x01", data[3]);
                break;
            }

            if ((data[0] | data[1] | data[2]) == 0x00)
            {
                break;
            }

            const int data0 = segment_to_number(data[0]);
            const int data1 = segment_to_number(data[1]);
            const int data2 = segment_to_number(data[2]);

            if (data0 < 0x00 || data1 < 0x00 || data2 < 0x00)
            {
                break;
            }

            float height = segment_to_number(data[0]) * 100 + segment_to_number(data[1]) * 10 + segment_to_number(data[2]);
            if (data[1] & 0x80)
            {
                height /= 10.0;
            }

            current_height_ = height;
            height_sensor_->publish_state(height);
        }

        desk_buffer_.clear();
        desk_buffer_.resize(0);
    }
}

void Desktronic::publish_remote_states(const uint8_t data)
{
    if (up_bsensor_ != nullptr)
    {
        up_bsensor_->publish_state(data & MovingIdentifier::MOVING_IDENTIFIER_UP);
    }
    if (down_bsensor_ != nullptr)
    {
        down_bsensor_->publish_state(data & MovingIdentifier::MOVING_IDENTIFIER_DOWN);
    }
    if (memory1_bsensor_ != nullptr)
    {
        memory1_bsensor_->publish_state(data & MovingIdentifier::MOVING_IDENTIFIER_MEMORY_1);
    }
    if (memory2_bsensor_ != nullptr)
    {
        memory2_bsensor_->publish_state(data & MovingIdentifier::MOVING_IDENTIFIER_MEMORY_2);
    }
    if (memory3_bsensor_ != nullptr)
    {
        memory3_bsensor_->publish_state(data & MovingIdentifier::MOVING_IDENTIFIER_MEMORY_3);
    }
}

void Desktronic::loop()
{
    read_remote_uart();
    read_desk_uart();
}

void Desktronic::dump_config()
{
    ESP_LOGCONFIG(TAG, "Desktronic Desk");
    LOG_SENSOR("", "Height", height_sensor_);
    LOG_BINARY_SENSOR("  ", "Up", up_bsensor_);
    LOG_BINARY_SENSOR("  ", "Down", down_bsensor_);
    LOG_BINARY_SENSOR("  ", "Memory1", memory1_bsensor_);
    LOG_BINARY_SENSOR("  ", "Memory2", memory2_bsensor_);
    LOG_BINARY_SENSOR("  ", "Memory3", memory3_bsensor_);
}

} // namespace desktronic
} // namespace esphome
