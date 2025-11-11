#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace gatepro {

class GateProTextSensor : public text_sensor::TextSensor, public Component {
   protected:
      void press_action() override { }
};

}  // namespace gatepro
}  // namespace esphome
