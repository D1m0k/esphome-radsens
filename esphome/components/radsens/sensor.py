import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    ICON_RADIOACTIVE,
    UNIT_EMPTY,
    UNIT_SECOND,
    ICON_CHIP,
    DEVICE_CLASS_FIRMWARE,
    DEVICE_CLASS_EMPTY,
    ENTITY_CATEGORY_DIAGNOSTIC
)

from .const import (
    UNIT_MICROROENTGEN_PER_HOUR,
    UNIT_MICROROENTGEN,
    UNIT_MILLISIEVERT,
    UNIT_COUNT_PER_MINUTE,
    CONF_DYNAMIC_INTENSITY,
    CONF_STATIC_INTENSITY,
    CONF_COUNTS_PER_POLLING,
    CONF_COUNTS_PER_MINUTE,
    CONF_RADSENS_MAX_CPM,
    CONF_RADSENS_MAX_CPP,
    CONF_MAX_CPM_TIMESTAMP,
    CONF_MAX_CPP_TIMESTAMP,
    CONF_TOTAL_CPP,
    CONF_UPTIME,
    CONF_ACCUMULATED_DOSE_UR,
    CONF_ACCUMULATED_DOSE_MSV,
    CONF_FIRMWARE_VERSION
)

from . import CONF_RADSENS_ID, RadSensComponent

DEPENDENCIES = ["radsens"]

intensity_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_MICROROENTGEN_PER_HOUR,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=1,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_EMPTY
)
counts_per_minute_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_COUNT_PER_MINUTE,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=1,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_EMPTY
)
count_integer_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_EMPTY
)
counts_per_polling_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_EMPTY
)
uptime_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_SECOND,
    icon="mdi:timer-outline",
    accuracy_decimals=0,
    state_class=STATE_CLASS_TOTAL_INCREASING,
    device_class=DEVICE_CLASS_EMPTY,
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
)
dose_ur_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_MICROROENTGEN,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=3,
    state_class=STATE_CLASS_TOTAL_INCREASING,
    device_class=DEVICE_CLASS_EMPTY,
)
dose_msv_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_MILLISIEVERT,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=6,
    state_class=STATE_CLASS_TOTAL_INCREASING,
    device_class=DEVICE_CLASS_EMPTY,
)
firmware_version_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_CHIP,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_EMPTY,
    entity_category=ENTITY_CATEGORY_DIAGNOSTIC
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_RADSENS_ID): cv.use_id(RadSensComponent),
            cv.Optional(CONF_DYNAMIC_INTENSITY): intensity_schema,
            cv.Optional(CONF_STATIC_INTENSITY): intensity_schema,
            cv.Optional(CONF_COUNTS_PER_POLLING): counts_per_polling_schema,
            cv.Optional(CONF_COUNTS_PER_MINUTE): counts_per_minute_schema,
            cv.Optional(CONF_RADSENS_MAX_CPM): counts_per_minute_schema,
            cv.Optional(CONF_RADSENS_MAX_CPP): count_integer_schema,
            cv.Optional(CONF_MAX_CPM_TIMESTAMP): uptime_schema,
            cv.Optional(CONF_MAX_CPP_TIMESTAMP): uptime_schema,
            cv.Optional(CONF_TOTAL_CPP): count_integer_schema,
            cv.Optional(CONF_UPTIME): uptime_schema,
            cv.Optional(CONF_ACCUMULATED_DOSE_UR): dose_ur_schema,
            cv.Optional(CONF_ACCUMULATED_DOSE_MSV): dose_msv_schema,
            cv.Optional(CONF_FIRMWARE_VERSION): firmware_version_schema,
        }
    )
)

async def to_code(config):
    var = await cg.get_variable(config[CONF_RADSENS_ID])
    if CONF_DYNAMIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_DYNAMIC_INTENSITY])
        cg.add(var.set_dynamic_intensity_sensor(sens))
    if CONF_STATIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_STATIC_INTENSITY])
        cg.add(var.set_static_intensity_sensor(sens))
    if CONF_COUNTS_PER_POLLING in config:
        sens = await sensor.new_sensor(config[CONF_COUNTS_PER_POLLING])
        cg.add(var.set_counts_per_polling_sensor(sens))
    if CONF_COUNTS_PER_MINUTE in config:
        sens = await sensor.new_sensor(config[CONF_COUNTS_PER_MINUTE])
        cg.add(var.set_counts_per_minute_sensor(sens))
    if CONF_RADSENS_MAX_CPM in config:
        sens = await sensor.new_sensor(config[CONF_RADSENS_MAX_CPM])
        cg.add(var.set_radsens_max_cpm_sensor(sens))
    if CONF_RADSENS_MAX_CPP in config:
        sens = await sensor.new_sensor(config[CONF_RADSENS_MAX_CPP])
        cg.add(var.set_radsens_max_cpp_sensor(sens))
    if CONF_MAX_CPM_TIMESTAMP in config:
        sens = await sensor.new_sensor(config[CONF_MAX_CPM_TIMESTAMP])
        cg.add(var.set_max_cpm_timestamp_sensor(sens))
    if CONF_MAX_CPP_TIMESTAMP in config:
        sens = await sensor.new_sensor(config[CONF_MAX_CPP_TIMESTAMP])
        cg.add(var.set_max_cpp_timestamp_sensor(sens))
    if CONF_TOTAL_CPP in config:
        sens = await sensor.new_sensor(config[CONF_TOTAL_CPP])
        cg.add(var.set_total_cpp_sensor(sens))
    if CONF_UPTIME in config:
        sens = await sensor.new_sensor(config[CONF_UPTIME])
        cg.add(var.set_uptime_sensor(sens))
    if CONF_ACCUMULATED_DOSE_UR in config:
        sens = await sensor.new_sensor(config[CONF_ACCUMULATED_DOSE_UR])
        cg.add(var.set_accumulated_dose_ur_sensor(sens))
    if CONF_ACCUMULATED_DOSE_MSV in config:
        sens = await sensor.new_sensor(config[CONF_ACCUMULATED_DOSE_MSV])
        cg.add(var.set_accumulated_dose_msv_sensor(sens))
    if CONF_FIRMWARE_VERSION in config:
        sens = await sensor.new_sensor(config[CONF_FIRMWARE_VERSION])
        cg.add(var.set_firmware_version_sensor(sens))
