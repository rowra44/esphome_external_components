#pragma once

#include "esphome/components/button/button.h"
#include "esphome/core/component.h"

namespace esphome {
namespace gatepro {

class GateProButton : public button::Button, public Component {
   protected:
      void press_action() override { }
};

}  // namespace gatepro
}  // namespace esphome
