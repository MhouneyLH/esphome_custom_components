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
    if (m_upPin != nullptr)
    {
        m_upPin->digital_write(false);
    }

    if (m_downPin != nullptr)
    {
        m_downPin->digital_write(false);
    }

    if (m_requestPin != nullptr)
    {
        m_requestPin->digital_write(true);
        m_requestTime = esphome::millis();
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
        switch (state)
        {
        case 0:
            if (readByte == 1)
            {
                state = 1;
            }

            break;
        case 1:
            if (readByte == 1)
            {
                state = 2;
            }
            else
            {
                state = 0;
            }

            break;
        case 2:
            high_byte = readByte;
            state = 3;

            break;
        case 3:
            value = (high_byte << 8) + readByte;
            m_currentPosition = value;

            if (m_heightSensor != nullptr)
            {
                m_heightSensor->publish_state(value);
            }

            state = 0;
            break;
        }
    }

    if (m_targetPosition >= 0)
    {
        if ((abs(m_targetPosition - m_currentPosition) < m_stoppingDistance) ||
            ((m_timeout >= 0) && (millis() - m_startTime >= m_timeout)))
        {
            stop();
        }
    }

    if ((m_requestTime > 0) && (millis() - m_requestTime >= 100))
    {
        m_requestPin->digital_write(false);
        m_requestTime = 0;
    }
}

void Desktronic::setLogConfig()
{
    ESP_LOGCONFIG(TAG, "DesktronicDesk:");

    LOG_SENSOR("", "Height", m_heightSensor);
    LOG_PIN("UpPin: ", m_upPin);
    LOG_PIN("DownPin: ", m_downPin);
    LOG_PIN("RequestPin: ", m_requestPin);
}

void Desktronic::move_to_position(const int targetPosition)
{
    if (abs(targetPosition - m_currentPosition) < m_stoppingDistance)
    {
        return;
    }

    if (targetPosition > m_currentPosition)
    {
        if (m_upPin == nullptr)
        {
            return;
        }

        m_upPin->digital_write(true);
        m_currentOperation = DESKTRONIC_OPERATION_RAISING;
    }
    else
    {
        if (m_downPin == nullptr)
        {
            return;
        }

        m_downPin->digital_write(true);
        m_currentOperation = DESKTRONIC_OPERATION_LOWERING;
    }

    m_targetPosition = targetPosition;

    if (m_timeout >= 0)
    {
        m_startTime = esphome::millis();
    }
}

void Desktronic::stop()
{
    m_targetPosition = -1;

    if (m_upPin != nullptr)
    {
        m_upPin->digital_write(false);
    }

    if (m_downPin != nullptr)
    {
        m_downPin->digital_write(false);
    }

    m_currentOperation = DESKTRONIC_OPERATION_IDLE;
}

} // namespace desktronic
} // namespace esphome
