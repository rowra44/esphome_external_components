#pragma once

#include "esphome/components/button/button.h"
#include "esphome/core/component.h"

namespace esphome {
namespace gatepro {

class GateProButton : public switch_::Switch, public Component {
   protected:
      void press_action() override { ESP_LOGD("nyeh", "pressed"); }
};

}  // namespace gatepro
}  // namespace esphome
