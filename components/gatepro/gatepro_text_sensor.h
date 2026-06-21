#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace gatepro {

class GateProTextSensor : public Component, public text_sensor::TextSensor {
   public:   
};

}  // namespace gatepro
}  // namespace esphome
