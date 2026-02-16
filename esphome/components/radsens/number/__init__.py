from esphome.components import number
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    UNIT_SECOND,
)

from ..const import (
    CONF_POLLING_INTERVAL,
)

from .. import CONF_RADSENS_ID, RadSensComponent, radsens_ns

DEPENDENCIES = ["radsens"]

PollingIntervalControl = radsens_ns.class_("PollingIntervalControl", number.Number)

polling_interval_schema = number.number_schema(
    PollingIntervalControl,
    unit_of_measurement=UNIT_SECOND,
    entity_category=ENTITY_CATEGORY_CONFIG,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_RADSENS_ID): cv.use_id(RadSensComponent),
            cv.Optional(CONF_POLLING_INTERVAL): polling_interval_schema,
        }
    )
)


async def to_code(config):
    var = await cg.get_variable(config[CONF_RADSENS_ID])
    if CONF_POLLING_INTERVAL in config:
        num = await number.new_number(
            config[CONF_POLLING_INTERVAL],
            min_value=5,
            max_value=300,
            step=1,
        )
        await cg.register_parented(num, config[CONF_RADSENS_ID])
        cg.add(var.set_polling_interval_number(num))

