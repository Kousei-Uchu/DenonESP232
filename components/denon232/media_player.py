import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import media_player
from esphome.const import CONF_ID, CONF_NAME, CONF_DISABLED_BY_DEFAULT

from . import denon232_ns, Denon232Receiver, CONF_CABLE_MODE, CABLE_MODES

Denon232MediaPlayer = denon232_ns.class_("Denon232MediaPlayer", media_player.MediaPlayer, cg.Component)

CONF_POLLING_INTERVAL = "polling_interval"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Denon232MediaPlayer),
        cv.Required("denon232_id"): cv.use_id(Denon232Receiver),
        cv.Optional(CONF_CABLE_MODE, default="null_modem"): cv.enum(CABLE_MODES),
        cv.Optional(CONF_POLLING_INTERVAL, default=5000): cv.int_range(min=1000, max=60000),
        cv.Optional(CONF_NAME, default="Denon Receiver"): cv.string,
        cv.Optional(CONF_DISABLED_BY_DEFAULT, default=False): cv.boolean,  # <-- NEW
    }
)

async def to_code(config):
    receiver = await cg.get_variable(config["denon232_id"])

    var = cg.new_Pvariable(config[CONF_ID], config.get(CONF_NAME, "Denon Receiver"))
    cg.add(var.set_denon_receiver(receiver))
    cg.add(var.set_cable_mode(config.get(CONF_CABLE_MODE, "null_modem")))
    cg.add(var.set_polling_interval(config.get(CONF_POLLING_INTERVAL, 5000)))

    await cg.register_component(var, config)
    await media_player.register_media_player(var, config)