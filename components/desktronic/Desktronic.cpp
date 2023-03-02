#include "Desktronic.h"
#include "esphome/core/log.h"

namespace esphome
{
namespace desktronic
{

static const char* TAG = "desktronic";
static const uint8_t REMOTE_UART_MESSAGE_LENGTH = 5U;
static const uint8_t REMOTE_UART_MESSAGE_START = 0xA5;
static const uint8_t DESK_UART_MESSAGE_LENGTH = 6U;
static const uint8_t DESK_UART_MESSAGE_START = 0x5A;

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
        ESP_LOGV(TAG, "unknown digit: %02f", segment & 0x7f);
    }

    return -1;
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
        ESP_LOGE(TAG, "remote byte: %02x", byte);

        // is it a rx-message?
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
        if (remote_buffer_.size() < REMOTE_UART_MESSAGE_LENGTH - 1)
        {
            continue;
        }

        remote_rx_ = false;
        uint8_t* data = remote_buffer_.data();

        uint8_t checksum = data[2] + data[3];
        if (checksum != data[4])
        {
            ESP_LOGE(TAG, "remote checksum mismatch: %02x != %02x", checksum, data[3]);
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

        if (!desk_rx_)
        {
            if (byte != DESK_UART_MESSAGE_START)
            {
                continue;
            }

            desk_rx_ = true;
            continue;
        }

        desk_buffer_.push_back(byte);
        // -1, because of the start byte
        if (desk_buffer_.size() < DESK_UART_MESSAGE_LENGTH - 1)
        {
            continue;
        }

        desk_rx_ = false;
        uint8_t* data = desk_buffer_.data();

        uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        if (checksum != data[4])
        {
            ESP_LOGE(TAG, "desk checksum mismatch: %02x != %02x", checksum, data[4]);
            desk_buffer_.clear();

            continue;
        }

        if (height_sensor_ != nullptr)
        {
            if (data[3] != 1)
            {
                ESP_LOGV(TAG, "unknown message type %02x", data[3]);
                break;
            }

            if ((data[0] | data[1] | data[2]) == 0)
            {
                break;
            }

            const int data0 = segment_to_number(data[0]);
            const int data1 = segment_to_number(data[1]);
            const int data2 = segment_to_number(data[2]);
            if (data0 < 0 || data1 < 0 || data2 < 0)
            {
                break;
            }

            float height = segment_to_number(data[0]) * 100 + segment_to_number(data[1]) * 10 + segment_to_number(data[2]);
            if (data[1] & 0x80)
            {
                height /= 10.0;
            }

            height_sensor_->publish_state(height);
        }

        desk_buffer_.clear();
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
