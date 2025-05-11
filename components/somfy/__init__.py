import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import remote_base
from esphome.const import CONF_ID


DEPENDENCIES = ["remote_transmitter", "api"]

AUTOLOAD = ["remote_base"]

somfy_ns = cg.esphome_ns.namespace("somfy")
Somfy = somfy_ns.class_("RFsomfy",
    cg.Component
)

CONF_ADDRESS = "address"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Somfy),
            cv.Required(CONF_ADDRESS): cv.int_,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(remote_base.REMOTE_TRANSMITTABLE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_address(config[CONF_ADDRESS]))
    await cg.register_component(var, config)
    await remote_base.register_transmittable(var, config)
