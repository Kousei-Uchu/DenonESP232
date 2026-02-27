import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import media_player
from esphome.const import CONF_ID, CONF_NAME

from .. import denon232_ns, Denon232Receiver, CONF_CABLE_MODE, CABLE_MODES

Denon232MediaPlayer = denon232_ns.class_("Denon232MediaPlayer", media_player.MediaPlayer, cg.Component)

CONF_POLLING_INTERVAL = "polling_interval"

PLATFORM_SCHEMA = media_player.MEDIA_PLAYER_PLATFORM_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(Denon232MediaPlayer),
        cv.GenerateID("denon232_id"): cv.use_id(Denon232Receiver),
        cv.Optional(CONF_CABLE_MODE, default="null_modem"): cv.enum(CABLE_MODES),
        cv.Optional(CONF_POLLING_INTERVAL, default=5000): cv.int_range(min=1000, max=60000),
    }
)


async def to_code(config):
    receiver = await cg.get_variable(config["denon232_id"])
    
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_NAME])
    cg.add(var.set_denon_receiver(receiver))
    cg.add(var.set_cable_mode(config[CONF_CABLE_MODE]))
    cg.add(var.set_polling_interval(config[CONF_POLLING_INTERVAL]))
    await cg.register_component(var, config)
    await media_player.register_media_player(var, config)
