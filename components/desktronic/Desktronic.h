#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"

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

const char* desktronicOperationToString(const DesktronicOperation operation);

class Desktronic : public Component
    , public sensor::Sensor
    , public uart::UARTDevice
{
public:
    virtual float get_setup_priority() const override { return esphome::setup_priority::LATE; }
    virtual void setup() override;
    virtual void loop() override;
    void setLogConfig();

public:
    void set_height_sensor(sensor::Sensor* sensor) { height_sensor_ = sensor; }
    void set_up_pin(GPIOPin* pin) { up_pin_ = pin; }
    void set_down_pin(GPIOPin* pin) { down_pin_ = pin; }
    void set_request_pin(GPIOPin* pin) { request_pin_ = pin; }
    void set_stopping_distance(const int distance) { stopping_distance_ = distance; }
    void set_timeout(const int timeout) { timeout_ = timeout; }

    void move_to_position(const int targetPosition);
    void stop();

    DesktronicOperation current_operation = DESKTRONIC_OPERATION_IDLE;
    std::string test_str = "";

protected:
    sensor::Sensor* height_sensor_ = nullptr;
    GPIOPin* up_pin_ = nullptr;
    GPIOPin* down_pin_ = nullptr;
    GPIOPin* request_pin_ = nullptr;

    int stopping_distance_;
    int current_pos_ = 0;
    int target_pos_ = -1;
    int timeout_ = -1;

    uint64_t start_time_ = 0;
    uint64_t request_time_ = 0;
};

} // namespace desktronic
} // namespace esphome