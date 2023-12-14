import hashlib

from esphome import config_validation as cv, automation
from esphome import codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_INITIAL_VALUE,
    CONF_RESTORE_VALUE,
    CONF_TYPE,
    CONF_VALUE,
)
from esphome.core import coroutine_with_priority

CODEOWNERS = ["@rfdarter/coopSettings"]
coopSettings_ns = cg.esphome_ns.namespace("coopSettings")
RestoringCoopSettingComponent = coopSettings_ns.class_("RestoringCoopSettingsComponent", cg.Component)

CONF_MAX_RESTORE_DATA_LENGTH = "max_restore_data_length"


MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.declare_id(RestoringCoopSettingComponent),
        cv.Optional(CONF_INITIAL_VALUE): cv.string_strict,
        cv.Optional(CONF_RESTORE_VALUE, default=False): cv.boolean,
        cv.Optional(CONF_MAX_RESTORE_DATA_LENGTH): cv.int_range(0, 254),
    }
).extend(cv.COMPONENT_SCHEMA)


# Run with low priority so that namespaces are registered first
@coroutine_with_priority(-100.0)
async def to_code(config):
    type_ = 'CoopSettings'
    restore = config[CONF_RESTORE_VALUE]

    template_args = cg.TemplateArguments(type_)
    type = RestoringCoopSettingComponent

    res_type = type.template(template_args)
    initial_value = None
    if CONF_INITIAL_VALUE in config:
        initial_value = cg.RawExpression(config[CONF_INITIAL_VALUE])

    rhs = type.new(template_args, initial_value)
    coopSet = cg.Pvariable(config[CONF_ID], rhs, res_type)
    await cg.register_component(coopSet, config)

    if restore:
        value = config[CONF_ID].id
        if isinstance(value, str):
            value = value.encode()
        hash_ = int(hashlib.md5(value).hexdigest()[:8], 16)
        cg.add(coopSet.set_name_hash(hash_))

