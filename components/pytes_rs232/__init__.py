import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, text_sensor
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

pytes_ns = cg.esphome_ns.namespace("pytes_rs232")
PytesRS232 = pytes_ns.class_("PytesRS232", cg.PollingComponent, uart.UARTDevice)

CONF_UART_ID = "uart_id"
CONF_NUM_BATTERIES = "num_batteries"
CONF_CAPACITY_AH = "capacity_ah"
CONF_SUMMARY = "summary"
CONF_BATTERIES = "batteries"

# per-battery keys
CONF_VOLTAGE = "voltage"
CONF_SOC_VOLTAGE = "soc_voltage"   # opcional, puede NO estar
CONF_CURRENT = "current"
CONF_TEMPERATURE = "temperature"
CONF_COULOMB = "coulomb"
CONF_SOC_ALIAS = "soc"             # alias -> coulomb

CONF_BARCODE = "barcode"
CONF_DEVTYPE = "devtype"
CONF_FIRM_VERSION = "firm_version"
CONF_BASIC_STATUS = "basic_status"
CONF_VOLT_STATUS = "volt_status"

def _apply_soc_alias(cfg):
    # allow "soc:" as alias of "coulomb:" in summary and batteries
    if isinstance(cfg, dict):
        if CONF_SUMMARY in cfg and isinstance(cfg[CONF_SUMMARY], dict):
            s = cfg[CONF_SUMMARY]
            if CONF_SOC_ALIAS in s and CONF_COULOMB not in s:
                s[CONF_COULOMB] = s[CONF_SOC_ALIAS]
        if CONF_BATTERIES in cfg and isinstance(cfg[CONF_BATTERIES], list):
            for b in cfg[CONF_BATTERIES]:
                if isinstance(b, dict) and CONF_SOC_ALIAS in b and CONF_COULOMB not in b:
                    b[CONF_COULOMB] = b[CONF_SOC_ALIAS]
    return cfg

SUMMARY_SCHEMA = cv.Schema({
    cv.Optional(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_CURRENT): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_COULOMB): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_BASIC_STATUS): cv.use_id(text_sensor.TextSensor),

    # alias
    cv.Optional(CONF_SOC_ALIAS): cv.use_id(sensor.Sensor),
})

BATTERY_SCHEMA = cv.Schema({
    cv.Optional(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_SOC_VOLTAGE): cv.use_id(sensor.Sensor),   # puede faltar
    cv.Optional(CONF_CURRENT): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_COULOMB): cv.use_id(sensor.Sensor),

    # alias
    cv.Optional(CONF_SOC_ALIAS): cv.use_id(sensor.Sensor),

    cv.Optional(CONF_BARCODE): cv.use_id(text_sensor.TextSensor),
    cv.Optional(CONF_DEVTYPE): cv.use_id(text_sensor.TextSensor),
    cv.Optional(CONF_FIRM_VERSION): cv.use_id(text_sensor.TextSensor),
    cv.Optional(CONF_BASIC_STATUS): cv.use_id(text_sensor.TextSensor),
    cv.Optional(CONF_VOLT_STATUS): cv.use_id(text_sensor.TextSensor),
})

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(CONF_ID): cv.declare_id(PytesRS232),
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),

        cv.Optional(CONF_NUM_BATTERIES, default=6): cv.int_range(min=1, max=16),
        cv.Optional(CONF_UPDATE_INTERVAL, default="30s"): cv.update_interval,
        cv.Optional(CONF_CAPACITY_AH, default=100.0): cv.float_range(min=1.0, max=2000.0),

        cv.Optional(CONF_SUMMARY, default={}): SUMMARY_SCHEMA,
        cv.Optional(CONF_BATTERIES, default=[]): cv.ensure_list(BATTERY_SCHEMA),
    }).extend(cv.COMPONENT_SCHEMA),
    _apply_soc_alias
)

DEPENDENCIES = ["uart"]

async def to_code(config):
    parent = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_num_batteries(int(config[CONF_NUM_BATTERIES])))
    cg.add(var.set_capacity_ah(float(config[CONF_CAPACITY_AH])))

    # -------- Summary (usar nullptr cuando falte algo) --------
    s = config.get(CONF_SUMMARY, {})

    sum_v = cg.nullptr
    sum_c = cg.nullptr
    sum_t = cg.nullptr
    sum_soc = cg.nullptr
    sum_basic = cg.nullptr

    if CONF_VOLTAGE in s:
        sum_v = await cg.get_variable(s[CONF_VOLTAGE])
    if CONF_CURRENT in s:
        sum_c = await cg.get_variable(s[CONF_CURRENT])
    if CONF_TEMPERATURE in s:
        sum_t = await cg.get_variable(s[CONF_TEMPERATURE])
    if CONF_COULOMB in s:
        sum_soc = await cg.get_variable(s[CONF_COULOMB])
    if CONF_BASIC_STATUS in s:
        sum_basic = await cg.get_variable(s[CONF_BASIC_STATUS])

    cg.add(var.set_summary(sum_v, sum_c, sum_t, sum_soc, sum_basic))

    # -------- Batteries (nullptr para todo lo opcional) --------
    batteries = config.get(CONF_BATTERIES, [])
    for idx, b in enumerate(batteries, start=1):
        v   = cg.nullptr
        sv  = cg.nullptr   # <- este era el que se quedaba en None y explotaba
        cur = cg.nullptr
        tmp = cg.nullptr
        cou = cg.nullptr

        bc  = cg.nullptr
        dt  = cg.nullptr
        fv  = cg.nullptr
        bs  = cg.nullptr
        vs  = cg.nullptr

        if CONF_VOLTAGE in b:
            v = await cg.get_variable(b[CONF_VOLTAGE])
        if CONF_SOC_VOLTAGE in b:
            sv = await cg.get_variable(b[CONF_SOC_VOLTAGE])
        if CONF_CURRENT in b:
            cur = await cg.get_variable(b[CONF_CURRENT])
        if CONF_TEMPERATURE in b:
            tmp = await cg.get_variable(b[CONF_TEMPERATURE])
        if CONF_COULOMB in b:
            cou = await cg.get_variable(b[CONF_COULOMB])

        if CONF_BARCODE in b:
            bc = await cg.get_variable(b[CONF_BARCODE])
        if CONF_DEVTYPE in b:
            dt = await cg.get_variable(b[CONF_DEVTYPE])
        if CONF_FIRM_VERSION in b:
            fv = await cg.get_variable(b[CONF_FIRM_VERSION])
        if CONF_BASIC_STATUS in b:
            bs = await cg.get_variable(b[CONF_BASIC_STATUS])
        if CONF_VOLT_STATUS in b:
            vs = await cg.get_variable(b[CONF_VOLT_STATUS])

        cg.add(var.set_battery(idx, v, sv, cur, tmp, cou, bc, dt, fv, bs, vs))
