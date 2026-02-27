import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import web_server
from esphome.const import CONF_ID

from . import denon232_ns, Denon232Receiver

Denon232WebServer = denon232_ns.class_("Denon232WebServer", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Denon232WebServer),
        cv.GenerateID("denon232_id"): cv.use_id(Denon232Receiver),
        cv.GenerateID("web_server_id"): cv.use_id(web_server.WebServer),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    receiver = await cg.get_variable(config["denon232_id"])
    web_server_var = await cg.get_variable(config["web_server_id"])
    
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_denon_receiver(receiver))
    cg.add(var.set_web_server(web_server_var))
    await cg.register_component(var, config)
