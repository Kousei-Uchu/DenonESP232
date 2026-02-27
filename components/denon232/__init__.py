import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

CODEOWNERS = ["@Kousei-Uchu"]

denon232_ns = cg.esphome_ns.namespace("denon232")
Denon232Receiver = denon232_ns.class_("Denon232Receiver", cg.Component)
CableMode = denon232_ns.enum("CableMode")

CONF_CABLE_MODE = "cable_mode"
CABLE_MODES = {
    "null_modem": CableMode.NULL_MODEM,
    "pass_through": CableMode.PASS_THROUGH,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Denon232Receiver),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_CABLE_MODE, default="null_modem"): cv.enum(CABLE_MODES),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_uart_parent(uart_component))
    cg.add(var.set_cable_mode(config[CONF_CABLE_MODE]))
    await cg.register_component(var, config)
