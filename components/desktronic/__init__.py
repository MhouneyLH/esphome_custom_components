import esphome.codegen as codegen
import esphome.config_validation as config_validation
from esphome import pins
from esphome.components import uart
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_HEIGHT, CONF_TIMEOUT, ICON_GAUGE

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor']

desktronic_ns = codegen.esphome_ns.namespace('desktronic')
Desktronic = desktronic_ns.class_('Desktronic', codegen.Component, uart.UARTDevice)

CONF_UP = "up"
CONF_DOWN = "down"
CONF_REQUEST = "request"
CONF_STOPPING_DISTANCE = "stopping_distance"
DEFAULT_STOPPING_DISTANCE = 15

CONFIG_SCHEMA = config_validation.COMPONENT_SCHEMA.extend({
    config_validation.GenerateID(): config_validation.declare_id(Desktronic),
    config_validation.Optional(CONF_UP): pins.gpio_output_pin_schema,
    config_validation.Optional(CONF_DOWN): pins.gpio_output_pin_schema,
    config_validation.Optional(CONF_REQUEST): pins.gpio_output_pin_schema,
    config_validation.Optional(CONF_HEIGHT): sensor.sensor_schema(icon=ICON_GAUGE, accuracy_decimals=0),
    config_validation.Optional(CONF_STOPPING_DISTANCE, default=DEFAULT_STOPPING_DISTANCE): config_validation.positive_int,
    config_validation.Optional(CONF_TIMEOUT): config_validation.time_period,
}).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = codegen.new_Pvariable(config[CONF_ID])
    await codegen.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_UP in config:
        pin = await codegen.gpio_pin_expression(config[CONF_UP])
        codegen.add(var.setUpPin(pin))
    if CONF_DOWN in config:
        pin = await codegen.gpio_pin_expression(config[CONF_DOWN])
        codegen.add(var.setDownPinpin))
    if CONF_REQUEST in config:
        pin = await codegen.gpio_pin_expression(config[CONF_REQUEST])
        codegen.add(var.setRequestPin(pin))
    if CONF_HEIGHT in config:
        sensor = await sensor.new_sensor(config[CONF_HEIGHT])
        codegen.add(var.setHeightSensor(sensor))

    # don't have to check if it's in config, because it has a default value
    codegen.add(var.set_stopping_distance(config[CONF_STOPPING_DISTANCE]))
    
    if CONF_TIMEOUT in config:
        codegen.add(var.setTimeout(config[CONF_TIMEOUT].total_milliseconds))