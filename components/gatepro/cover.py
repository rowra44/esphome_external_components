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
CONF_LEARN = "learn"
CONF_PARAMS_OD = "params"
CONF_REMOTE_LEARN = "remote_learn"
CONF_PED_OPEN = "ped_open"
# text sensors
CONF_DEVINFO = "devinfo"
CONF_LEARN_STATUS = "learn_status"

CONFIG_SCHEMA = cover.cover_schema(GatePro).extend(
    {
        # BUTTON controllers
        cv.GenerateID(): cv.declare_id(GatePro),
        cv.Optional(CONF_LEARN): cv.use_id(button.Button),
        cv.Optional(CONF_PARAMS_OD): cv.use_id(button.Button),
        cv.Optional(CONF_REMOTE_LEARN): cv.use_id(button.Button),
        # TEXT SENSORS
        cv.Optional(CONF_DEVINFO): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LEARN_STATUS): cv.use_id(text_sensor.TextSensor),
    }).extend(cv.COMPONENT_SCHEMA).extend(cv.polling_component_schema("60s")).extend(uart.UART_DEVICE_SCHEMA)

# SWITCH controllers mapping
# name - parameter list index
SWITCHES = {
   "infra1": 13,
   "infra2": 14,
   "stop_terminal": 15
}
for k, v in SWITCHES.items():
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(k): cv.use_id(switch.Switch)
   })

# NUMBER controllers mapping
# name - parameter list index
NUMBERS = {
   "auto_close": 1,
   "operational_speed": 3,
   "decel_dist": 4,
   "decel_speed": 5,
   "max_amp": 6,
   "ped_dura": 7   
}
for k, v in NUMBERS.items():
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(k): cv.use_id(number.Number)
   })

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    await uart.register_uart_device(var, config)
    # switches
    for k, v in SWITCHES.items():
      if k in config:
         sw = await cg.get_variable(config[k])
         cg.add(var.set_switch(v, sw))
    # numbers
    for k, v in NUMBERS.items():
      if k in config:
         num = await cg.get_variable(config[k])
         cg.add(var.set_number(v, num))

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
