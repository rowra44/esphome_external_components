import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, cover, button, number, text_sensor, switch, select
from esphome.const import CONF_ID, ICON_EMPTY, UNIT_EMPTY, CONF_NAME

AUTO_LOAD = ["switch", "select", "button"]
DEPENDENCIES = ["uart", "cover"]

gatepro_ns = cg.esphome_ns.namespace("gatepro")
GatePro = gatepro_ns.class_(
    "GatePro", cover.Cover, cg.PollingComponent, uart.UARTDevice
)

# switch
GateProSwitch = gatepro_ns.class_(
    "GateProSwitch", switch.Switch, cg.Component
)
SWITCH_SCHEMA = switch.switch_schema(GateProSwitch).extend(cv.COMPONENT_SCHEMA).extend(
    {cv.GenerateID(): cv.declare_id(GateProSwitch)}
)

# select
GateProSelect = gatepro_ns.class_(
    "GateProSelect", select.Select, cg.Component
)
SELECT_SCHEMA = select.select_schema(GateProSelect).extend(
    {cv.GenerateID(CONF_ID): cv.declare_id(GateProSelect)}
)

# button
GateProButton = gatepro_ns.class_(
    "GateProButton", button.Button, cg.Component
)
BUTTON_SCHEMA = select.select_schema(GateProButton).extend(
    {cv.GenerateID(): cv.declare_id(GateProButton)}
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
CONF_READ_STATUS = "read_status"
# text sensors
CONF_DEVINFO = "devinfo"
CONF_LEARN_STATUS = "learn_status"

CONFIG_SCHEMA = cover.cover_schema(GatePro).extend(
    {
        # TEXT SENSORS
        cv.Optional(CONF_DEVINFO): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LEARN_STATUS): cv.use_id(text_sensor.TextSensor),
    }).extend(cv.COMPONENT_SCHEMA).extend(cv.polling_component_schema("60s")).extend(uart.UART_DEVICE_SCHEMA)

# BUTTON controllers mapping
# name - command name list
BUTTONS = {
   "remote_learn": "GATEPRO_CMD_REMOTE_LEARN",
   "ped_open": "GATEPRO_CMD_PED_OPEN",
   "status": "GATEPRO_CMD_READ_STATUS",
}
for k, v in BUTTONS.items():
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(k): BUTTON_SCHEMA
   })


# SWITCH controllers mapping
# name - parameter list index
SWITCHES = {
   "infra1": 13,
   "infra2": 14,
   "stop_terminal": 15
}
for k, v in SWITCHES.items():
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(k): SWITCH_SCHEMA
   })

# SELECT controllers mapping
# name - {parameter list index, available options' list}
SELECTS = {
   "auto_close": {
      "idx": 1,
      "options": ["off", "5s", "15s", "30s", "45s", "60s", "80s", "120s", "180s"],
      "values": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
   },
   "operational_speed": {
      "idx": 3,
      "options": ["50%", "70%", "85%", "100%"],
      "values": [0, 1, 2, 3]
   },
   "decel_dist": {
      "idx": 4,
      "options": ["75%", "80%", "85%", "90%", "95%"],
      "values": [0, 1, 2, 3, 4]
   },
   "decel_speed": {
      "idx": 5,
      "options": ["80%", "60%", "40%", "25%"],
      "values": [0, 1, 2, 3]
   },
   "max_amp": {
      "idx": 6,
      "options": ["2A", "3A", "4A", "5A", "6A", "7A", "8A", "9A"],
      "values": [0, 1, 2, 3, 4, 5, 6, 7]
   },
   "ped_dura": {
      "idx": 7,
      "options": ["3s", "6s", "9s", "12s", "15s", "18s"],
      "values": [0, 1, 2, 3, 4, 5]
   }   
}
for k, v in SELECTS.items():
   CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
      cv.Optional(k): SELECT_SCHEMA
   })


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    await uart.register_uart_device(var, config)
    # switches
    for k, v in SWITCHES.items():
      if k in config:
         conf = config[k]
         sw = cg.new_Pvariable(conf[CONF_ID])
         await cg.register_component(sw, conf)
         await switch.register_switch(sw, conf)
         cg.add(var.set_switch(v, sw))

   # buttons
    for k, v in BUTTONS.items():
      if k in config:
         conf = config[k]
         btn = cg.new_Pvariable(conf[CONF_ID])
         await cg.register_component(btn, conf)
         await button.register_button(btn, conf)
         cg.add(var.set_button(btn, v))

    # selects
    for k, v in SELECTS.items():
      if k in config:
         conf = config[k]
         options = v["options"]
         values = v["values"]
         sel = await select.new_select(conf, options=options)
         await cg.register_component(sel, conf)
         cg.add(var.set_select(sel, v["idx"], options, values))

    # text sensors
    if CONF_DEVINFO in config:
      txt = await cg.get_variable(config[CONF_DEVINFO])
      cg.add(var.set_txt_devinfo(txt))
    if CONF_LEARN_STATUS in config:
      txt = await cg.get_variable(config[CONF_LEARN_STATUS])
      cg.add(var.set_txt_learn_status(txt))
