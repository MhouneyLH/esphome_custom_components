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
    void set_height_sensor(sensor::Sensor* sensor) { m_heightSensor = sensor; }
    void set_up_pin(GPIOPin* pin) { m_upPin = pin; }
    void set_down_pin(GPIOPin* pin) { m_downPin = pin; }
    void set_request_pin(GPIOPin* pin) { m_requestPin = pin; }
    void set_stopping_distance(const int distance) { m_stoppingDistance = distance; }
    void set_timeout(const int timeout) { m_timeout = timeout; }

    void move_to_position(const int targetPosition);
    void stop();

protected:
    DesktronicOperation m_currentOperation = DESKTRONIC_OPERATION_IDLE;

    sensor::Sensor* m_heightSensor = nullptr;
    GPIOPin* m_upPin = nullptr;
    GPIOPin* m_downPin = nullptr;
    GPIOPin* m_requestPin = nullptr;

    int m_stoppingDistance;
    int m_currentPosition = 0;
    int m_targetPosition = -1;
    int m_timeout = -1;

    uint64_t m_startTime = 0;
    uint64_t m_requestTime = 0;
};

} // namespace desktronic
} // namespace esphome