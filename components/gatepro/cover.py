import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, cover, button, number, text_sensor, switch
from esphome.const import CONF_ID, ICON_EMPTY, UNIT_EMPTY, CONF_NAME

DEPENDENCIES = ["uart", "cover", "button"]

gatepro_ns = cg.esphome_ns.namespace("gatepro")
GatePro = gatepro_ns.class_(
    "GatePro", cover.Cover, cg.PollingComponent, uart.UARTDevice
)

CONF_OPERATIONAL_SPEED = "operational_speed"

cover.COVER_OPERATIONS.update({
    "READ_STATUS": cover.CoverOperation.COVER_OPERATION_READ_STATUS,
})
validate_cover_operation = cv.enum(cover.COVER_OPERATIONS, upper=True)

# buttons
CONF_LEARN = "set_learn"
CONF_PARAMS_OD = "get_params"
CONF_REMOTE_LEARN = "remote_learn"
CONF_PED_OPEN = "get_ped_open"
# text sensors
CONF_DEVINFO = "txt_devinfo"
CONF_LEARN_STATUS = "txt_learn_status"
# switchs
CONF_PERMALOCK = "sw_permalock"
CONF_INFRA1 = "sw_infra1"
CONF_INFRA2 = "sw_infra2"

CONFIG_SCHEMA = cover.cover_schema(GatePro).extend(
    {
        # buttons
        cv.GenerateID(): cv.declare_id(GatePro),
        cv.Optional(CONF_LEARN): cv.use_id(button.Button),
        cv.Optional(CONF_PARAMS_OD): cv.use_id(button.Button),
        cv.Optional(CONF_REMOTE_LEARN): cv.use_id(button.Button),
        # text sensors
        cv.Optional(CONF_DEVINFO): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LEARN_STATUS): cv.use_id(text_sensor.TextSensor),
    }).extend(cv.COMPONENT_SCHEMA).extend(cv.polling_component_schema("60s")).extend(uart.UART_DEVICE_SCHEMA)

# SWITCH controllers
GP_SWITCH_SCHEMA = cv.Schema({
   cv.Required("switch"): cv.use_id(switch.Switch),
   cv.Required("param"): cv.int_,
})
SWITCHES = [
   "stop_terminal",
   "infra1",
   "infra2"
]
for i in SWITCHES:
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(i): GP_SWITCH_SCHEMA
   })

# NUMBER controllers
GP_NUMBER_SCHEMA = cv.Schema({
   cv.Required("number"): cv.use_id(number.Number),
   cv.Required("param"): cv.int_,
})
NUMBERS = [
   "operational_speed",
   "decel_dist",
   "decel_speed",
   "max_amp",
   "auto_close",
   "ped_dura",   
]

for i in NUMBERS:
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(i): GP_NUMBER_SCHEMA
   })

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    await uart.register_uart_device(var, config)
    # switches
    for i in SWITCHES:
      if i in config:
         cfg = config[i]
         sw = await cg.get_variable(cfg["switch"])
         cg.add(var.set_switch(cfg["param"], nuswm))
    # numbers
    for i in NUMBERS:
      if i in config:
         cfg = config[i]
         num = await cg.get_variable(cfg["number"])
         cg.add(var.set_number(cfg["param"], num))

    # text sensors
    if CONF_DEVINFO in config:
      txt = await cg.get_variable(config[CONF_DEVINFO])
      cg.add(var.set_txt_devinfo(txt))
    if CONF_LEARN_STATUS in config:
      txt = await cg.get_variable(config[CONF_LEARN_STATUS])
      cg.add(var.set_txt_learn_status(txt))
    # buttons
    if CONF_LEARN in config:
        btn = await cg.get_variable(config[CONF_LEARN])
        cg.add(var.set_btn_learn(btn))
    if CONF_PARAMS_OD in config:
        btn = await cg.get_variable(config[CONF_PARAMS_OD])
        cg.add(var.set_btn_params_od(btn))
    if CONF_REMOTE_LEARN in config:
        btn = await cg.get_variable(config[CONF_REMOTE_LEARN])
        cg.add(var.set_btn_remote_learn(btn))
