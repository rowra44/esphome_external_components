#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"

namespace esphome {
namespace gatepro {

class GateProTextSensor : public Component, public text_sensor::TextSensor, public EntityBase {
   public:   
};

}  // namespace gatepro
}  // namespace esphome
