"""Home Assistant integration for ESPDenon232."""
import logging
from typing import Any, Dict, Optional
import voluptuous as vol

from homeassistant.config_entries import ConfigEntry, ConfigFlow, OptionsFlow
from homeassistant.const import CONF_HOST, CONF_NAME, CONF_PORT
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import config_validation as cv

_LOGGER = logging.getLogger(__name__)

DOMAIN = "esphome_denon232"
DEFAULT_PORT = 6053

class Denon232ConfigFlow(ConfigFlow, domain=DOMAIN):
    """Handle a config flow for ESPDenon232."""

    VERSION = 1

    async def async_step_user(
        self, user_input: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """Handle the initial step."""
        errors = {}

        if user_input is not None:
            return self.async_create_entry(
                title=user_input[CONF_NAME],
                data=user_input
            )

        data_schema = vol.Schema({
            vol.Required(CONF_NAME, default="Denon Receiver"): cv.string,
            vol.Required(CONF_HOST): cv.string,
            vol.Optional(CONF_PORT, default=DEFAULT_PORT): cv.port,
            vol.Optional("cable_mode", default="null_modem"): vol.In(
                ["null_modem", "pass_through"]
            ),
            vol.Optional("polling_interval", default=5000): cv.int_range(
                min=1000, max=60000
            ),
        })

        return self.async_show_form(
            step_id="user",
            data_schema=data_schema,
            errors=errors
        )

    @staticmethod
    @callback
    def async_get_options_flow(config_entry: ConfigEntry):
        """Get the options flow for this config entry."""
        return Denon232OptionsFlow(config_entry)


class Denon232OptionsFlow(OptionsFlow):
    """Handle options flow for ESPDenon232."""

    def __init__(self, config_entry: ConfigEntry):
        """Initialize options flow."""
        self.config_entry = config_entry

    async def async_step_init(self, user_input: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Manage the options."""
        if user_input is not None:
            self.hass.config_entries.async_update_entry(
                self.config_entry,
                data={**self.config_entry.data, **user_input}
            )
            return self.async_abort(reason="reconfigure_successful")

        options_schema = vol.Schema({
            vol.Optional(
                "cable_mode",
                default=self.config_entry.options.get("cable_mode", "null_modem")
            ): vol.In(["null_modem", "pass_through"]),
            vol.Optional(
                "polling_interval",
                default=self.config_entry.options.get("polling_interval", 5000)
            ): cv.int_range(min=1000, max=60000),
        })

        return self.async_show_form(
            step_id="init",
            data_schema=options_schema,
            description_placeholders={
                "cable_info": "Null Modem: Standard cable with twisted pairs. Pass-Through: Straight cable."
            }
        )


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Set up Denon232 from a config entry."""
    hass.data.setdefault(DOMAIN, {})
    hass.data[DOMAIN][entry.entry_id] = {
        "host": entry.data[CONF_HOST],
        "port": entry.data.get(CONF_PORT, DEFAULT_PORT),
        "cable_mode": entry.data.get("cable_mode", "null_modem"),
        "polling_interval": entry.data.get("polling_interval", 5000)
    }
    
    await hass.config_entries.async_forward_entry_setups(entry, ["media_player"])
    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    if unload_ok := await hass.config_entries.async_unload_platforms(entry, ["media_player"]):
        hass.data[DOMAIN].pop(entry.entry_id)
    return unload_ok
