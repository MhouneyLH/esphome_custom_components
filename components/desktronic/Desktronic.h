#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

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

enum Segment : uint8_t
{
    SEGMENT_INVALID = 0x00,
    SEGMENT_0 = 0x3f,
    SEGMENT_1 = 0x06,
    SEGMENT_2 = 0x5b,
    SEGMENT_3 = 0x4f,
    SEGMENT_4 = 0x67,
    SEGMENT_5 = 0x6d,
    SEGMENT_6 = 0x7d,
    SEGMENT_7 = 0x07,
    SEGMENT_8 = 0x7f,
    SEGMENT_9 = 0x6f,
};

enum MovingIdentifier : uint8_t
{
    MOVING_IDENTIFIER_UP = 0x20,
    MOVING_IDENTIFIER_DOWN = 0x40,
    MOVING_IDENTIFIER_MEMORY_1 = 0x02,
    MOVING_IDENTIFIER_MEMORY_2 = 0x04,
    MOVING_IDENTIFIER_MEMORY_3 = 0x08,
};

static const char* desktronic_operation_to_string(const DesktronicOperation operation);
static int segment_to_number(const uint8_t segment);

class Desktronic : public Component
{
public:
    float get_setup_priority() const override { return setup_priority::LATE; }
    void setup() override;
    void loop() override;
    void dump_config() override;

    void set_remote_uart(uart::UARTComponent* uart) { remote_uart_ = uart; }
    void set_desk_uart(uart::UARTComponent* uart) { desk_uart_ = uart; }
    void set_move_pin(GPIOPin* pin) { move_pin_ = pin; }
    void set_height_sensor(sensor::Sensor* sensor) { height_sensor_ = sensor; }
    void set_up_bsensor(binary_sensor::BinarySensor* sensor) { up_bsensor_ = sensor; }
    void set_down_bsensor(binary_sensor::BinarySensor* sensor) { down_bsensor_ = sensor; }
    void set_memory1_bsensor(binary_sensor::BinarySensor* sensor) { memory1_bsensor_ = sensor; }
    void set_memory2_bsensor(binary_sensor::BinarySensor* sensor) { memory2_bsensor_ = sensor; }
    void set_memory3_bsensor(binary_sensor::BinarySensor* sensor) { memory3_bsensor_ = sensor; }

    void move_to(const float height_in_cm);
    void stop();

public:
    DesktronicOperation current_operation{DesktronicOperation::DESKTRONIC_OPERATION_IDLE};

private:
    void read_remote_uart();
    void read_desk_uart();
    void publish_remote_states(const uint8_t data);
    void reset_remote_buffer();
    void reset_desk_buffer();

    bool must_move_up(const float height_in_cm) const;
    void move_to_target_height();
    void move_up();
    void move_down();

    bool isCurrentHeightValid() const;
    bool isCurrentHeightInTargetBoundaries() const;

protected:
    uart::UARTComponent* remote_uart_{nullptr};
    uart::UARTComponent* desk_uart_{nullptr};
    GPIOPin* move_pin_{nullptr};
    sensor::Sensor* height_sensor_{nullptr};
    binary_sensor::BinarySensor* up_bsensor_{nullptr};
    binary_sensor::BinarySensor* down_bsensor_{nullptr};
    binary_sensor::BinarySensor* memory1_bsensor_{nullptr};
    binary_sensor::BinarySensor* memory2_bsensor_{nullptr};
    binary_sensor::BinarySensor* memory3_bsensor_{nullptr};

    std::vector<uint8_t> remote_buffer_;
    std::vector<uint8_t> desk_buffer_;
    bool is_remote_rx_uart_message_start_found{false};
    bool is_desk_rx_uart_message_start_found{false};
    float current_height_{0.0};
    float target_height_{-1.0};
};
} // namespace desktronic
} // namespace esphome