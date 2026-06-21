#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace gatepro {

class GateProTextSensor : public Component, public text_sensor::TextSensor {
   public:
    using text_sensor::TextSensor::set_entity_category;
    using text_sensor::TextSensor::get_entity_category;
   
};

}  // namespace gatepro
}  // namespace esphome
