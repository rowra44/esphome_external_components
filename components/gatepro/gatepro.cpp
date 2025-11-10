#include "esphome/core/log.h"
#include "gatepro.h"
#include <vector>

namespace esphome {
namespace gatepro {

////////////////////////////////////
static const char* TAG = "gatepro";

////////////////////////////////////////////
// Helper / misc functions
////////////////////////////////////////////
bool GatePro::read_msg() {
   this->read_uart();
   if (!this->rx_queue.size()) {
      return false;
   }
   this->current_msg = this->rx_queue.front();
   this->rx_queue.pop();
   return true;
}

GateProMsgType GatePro::identify_current_msg_type(
   std::map<GateProMsgType, const GateProMsgConstant> possibilities = GateProMsgTypeMapping) {
   for (const auto& [key, value] : possibilities) {
      if (this->current_msg.substr(value.pos, value.len) == value.match) {
         return key;
      }
   }
   return GATEPRO_MSG_UNKNOWN;
}

int GatePro::get_position_percentage() {
   const std::string percentage_string = this->current_msg.substr(STATUS_PERCENTAGE.pos,
                                                                  STATUS_PERCENTAGE.len);
   return stoi(percentage_string, 0, 16);
}

bool GatePro::is_moving() {
   return this->current_msg.substr(STATUS_OP_MOVING.pos,
                                   STATUS_OP_MOVING.len) ==
          STATUS_OP_MOVING.match;
}

std::string GatePro::convert(uint8_t* bytes, size_t len) {
	std::string res;
	char buf[5];
	for (size_t i = 0; i < len; i++) {
      auto cm = ConversionMap.find(bytes[i]);
      if (cm != ConversionMap.end()) {
         res += cm->second;
		} else if (bytes[i] < 32 || bytes[i] > 127) {
			sprintf(buf, "\\x%02X", bytes[i]);
			res += buf;
		} else {
			res += bytes[i];
		}
	}
	//ESP_LOGD(TAG, "%s", res.c_str());
	return res;
}

////////////////////////////////////////////
// Device logic
////////////////////////////////////////////
void GatePro::queue_gatepro_cmd(GateProCmd cmd) {
   ESP_LOGD(TAG, "Queuing cmd: %s", GateProCmdMapping.at(cmd));
   this->tx_queue.push(GateProCmdMapping.at(cmd));
}

void GatePro::control(const cover::CoverCall &call) {
   if (call.get_stop()) {
      this->start_direction_(cover::COVER_OPERATION_IDLE);
      return;
   }

   if (call.get_position().has_value()) {
      auto pos = *call.get_position();
      if (pos == this->position) {
         return;
      }
      auto op = pos < this->position ? cover::COVER_OPERATION_CLOSING : cover::COVER_OPERATION_OPENING;
      this->target_position_ = pos;
      this->start_direction_(op);
   }
}

void GatePro::start_direction_(cover::CoverOperation dir) {
   if (this->current_operation == dir ) {
      return;
   }

   switch (dir) {
      case cover::COVER_OPERATION_IDLE:
         if (this->operation_finished) {
            break;
         }
         this->queue_gatepro_cmd(GATEPRO_CMD_STOP);
         break;
      case cover::COVER_OPERATION_OPENING:
         this->queue_gatepro_cmd(GATEPRO_CMD_OPEN);
         break;
      case cover::COVER_OPERATION_CLOSING:
         this->queue_gatepro_cmd(GATEPRO_CMD_CLOSE);
         break;
      default:
         return;
   }
}

void GatePro::stop_at_target_position() {
   if (this->target_position_ &&
         this->target_position_ != cover::COVER_OPEN &&
         this->target_position_ != cover::COVER_CLOSED) {
      const float diff = abs(this->position - this->target_position_);
      if (diff < ACCEPTABLE_DIFF) {
         this->make_call().set_command_stop().perform();
      }
   }
}

/* The gate physically doesn't always move literally from 0% to 100%.
   It happens that it stops at around 0-3% / 97-100%, a small deviation,
   while physically / by naked eye it's completely normal looking, we
   have to correct these end position values, if necessary
*/
void GatePro::correction_after_operation() {
   if (this->operation_finished || this->startup) {
      if (this->current_operation == cover::COVER_OPERATION_IDLE &&
         this->last_operation_ == cover::COVER_OPERATION_CLOSING &&
         this->position != cover::COVER_CLOSED &&
         abs(this->position - cover::COVER_CLOSED) < ACCEPTABLE_DIFF / 2) {
         this->position = cover::COVER_CLOSED;
         return;
      }
 
      if (this->current_operation == cover::COVER_OPERATION_IDLE &&
         this->last_operation_ == cover::COVER_OPERATION_OPENING &&
         this->position != cover::COVER_OPEN &&
         abs(this->position - cover::COVER_OPEN) < ACCEPTABLE_DIFF / 2) {
         this->position = cover::COVER_OPEN;
      }
   }
}

void GatePro::process() {
   // try reading a message from uart
   if (!this->read_msg()) {
      return;
   }
   GateProMsgType current_msg_type = this->identify_current_msg_type();
   switch (current_msg_type) {
      case GATEPRO_MSG_UNKNOWN:
         ESP_LOGD(TAG, "Unkown message type");
         return;

      case GATEPRO_MSG_ACK_RS: {
         // status only matters when in motion (operation not finished) 
         if (this->operation_finished) {
            return;
         }

         int percentage = this->get_position_percentage();
         /* The following logic is only necessary for startup. We have to somehow be able
            to identify current state. The only known possible method is this logic:
            * if percentage is above 100, it's offset by the constant that's applied when opening;
                  => this means it's currently opening
            * if percentage is normal [0, 100], but the 3rd token "in movement";
                  => this means it's currently closing
            * otherwise just leave as is
         */
         if (percentage > 100) {
            percentage -= PERCENTAGE_OFFSET_WHILE_OPENING;
            this->current_operation = cover::COVER_OPERATION_OPENING;
            this->last_operation_ = cover::COVER_OPERATION_OPENING;
            this->operation_finished = false;

         } else if (this->is_moving()) {
            this->current_operation = cover::COVER_OPERATION_CLOSING;
            this->last_operation_ = cover::COVER_OPERATION_CLOSING;
            this->operation_finished = false;
         }
         /*
            End of startup movement identification logic
         */

         this->position = (float)percentage / 100;
         return;
      }

      case GATEPRO_MSG_ACK_RP:
         this->parse_params();
         return;

      case GATEPRO_MSG_MOTOR_EVENT: {
         GateProMsgType motor_event = this->identify_current_msg_type(MotorEvents);
         switch(motor_event) {
            case GATEPRO_MSG_UNKNOWN:
               ESP_LOGD(TAG, "Unkown motor event");
               return;

            case MOTOR_EVENT_OPENING:
               this->operation_finished = false;
               this->current_operation = cover::COVER_OPERATION_OPENING;
               this->last_operation_ = cover::COVER_OPERATION_OPENING;
               return;
            
            case MOTOR_EVENT_OPENED:
               this->operation_finished = true;
               this->target_position_ = 0.0f;
               this->current_operation = cover::COVER_OPERATION_IDLE;
               this->position = cover::COVER_OPEN;
               return;
            
            case MOTOR_EVENT_CLOSING:
               this->operation_finished = false;
               this->current_operation = cover::COVER_OPERATION_CLOSING;
               this->last_operation_ = cover::COVER_OPERATION_CLOSING;
               return;

            case MOTOR_EVENT_CLOSED:
               this->operation_finished = true;
               this->target_position_ = 0.0f;
               this->current_operation = cover::COVER_OPERATION_IDLE;
               this->position = cover::COVER_CLOSED;
               return;
               
            case MOTOR_EVENT_STOPPED:
               this->target_position_ = 0.0f;
               this->current_operation = cover::COVER_OPERATION_IDLE;
               return;            
         }
         return; // should never reach here.. but just to be safe..
      }

      case GATEPRO_MSG_ACK_READ_DEVINFO:
         if (!this->txt_devinfo)
            return;
         this->txt_devinfo->publish_state(this->current_msg.substr(17, this->current_msg.size() - (17 + 4)));
         return;

      case GATEPRO_MSG_ACK_LEARN_STATUS:
         if (!this->txt_learn_status)
            return;
         this->txt_learn_status->publish_state(this->current_msg.substr(17, this->current_msg.size() - (17 + 4)));
         return;
   } 
}

////////////////////////////////////////////
// Parameters
////////////////////////////////////////////
/* Working with params is really complicated:
   * there's no known & working method of changing a single param,
     => always have to write, and thus know, all other params back too
   * at first, we have no idea of params and whether they're changed or not
     so we start with reading them out
   * at the same time, we create a task in a task queue that
     "in the future" (after we will have read the current params):
         * overwrites the requested param index with requested value
         * initiates writing this modified param list back to the device
   * once the devices replies with the current params list, the process
     identifies this as a RP (read param) msg type, and executes parsing
     and thus filling up our internal (current) param list
   * and finally, executes the previously mentioned tasks from the queue
   * in the end, we simply build & queue the WP (write params) msg,
     that eventually gets sent to the device, and also update our sensors
*/
void GatePro::publish_params() {
   if (!this->param_no_pub) {
      // Numbers
      for (auto swi : this->sliders_with_indices) {
         swi.slider->publish_state(this->params[swi.idx]);
      }
      // Switches
      for (auto swi : this->switches_with_indices) {
         swi.switch_->publish_state(this->params[swi.idx]);
      }
   }
}

void GatePro::write_params() {
   this->params_cmd = GateProCmdMapping.at(GATEPRO_CMD_WRITE_PARAMS);
   for (size_t i = 0; i < this->params.size(); i++) {
      this->params_cmd += to_string(this->params[i]);
      if (i != this->params.size() - 1) {
         this->params_cmd += PARAMS_SEPARATOR;
      }
   }

   ESP_LOGD(TAG, "Built params: %s", this->params_cmd.c_str());
   this->tx_queue.push(this->params_cmd.c_str());

   // read params again just to update frontend and make sure :)
   this->queue_gatepro_cmd(GATEPRO_CMD_READ_PARAMS);
}

void GatePro::parse_params() {
   this->params.clear();
   std::string msg = this->current_msg.substr(PARAMS.pos, PARAMS.len);
   size_t start = 0;
   size_t end;

   // efficiently split on separator
   while((end = msg.find(PARAMS_SEPARATOR, start)) != std::string::npos) {
      this->params.push_back(stoi(msg.substr(start, end - start)));
      start = end + 1;
   }
   this->params.push_back(stoi(msg.substr(start)));

   this->publish_params();

   /* This is where magic happens  */
   while (!this->paramTaskQueue.empty()) {
      auto task = this->paramTaskQueue.front();
      this->paramTaskQueue.pop();
      task();
      this->param_no_pub = false;
   }
}

void GatePro::set_param(int idx, int val) {
   ESP_LOGD(TAG, "Initiating setting param %d to %d", idx, val);
   this->param_no_pub = true;
   this->queue_gatepro_cmd(GATEPRO_CMD_READ_PARAMS);

   this->paramTaskQueue.push(
      [this, idx, val](){
         this->params[idx] = val;
         this->write_params();
      });
}



////////////////////////////////////////////
// Sensor logic
////////////////////////////////////////////
void GatePro::publish() {
   // if position is unchanged
   if (this->position_ == this->position) {
      // ..and the after ticks are up, then don't update
      if (this->after_tick == 0) {
         return;
      // ..but there are some after tick counts left, then update and take away 1 after tick
      } else {
         this->after_tick--;
      }
   // otherwise update and reset after tick remaning count
   } else {
      this->position_ = this->position;
      this->after_tick = AFTER_TICK_MAX;
   }

   this->publish_state();
}

////////////////////////////////////////////
// UART
////////////////////////////////////////////
void GatePro::read_uart() {
   // check if anything on UART buffer
   int available = this->available();
   if (!available) {
      return;
   }
   
   // read the whole buffer into our own buffer
   // (if there's remainder from previous msgs, concatenate)
   uint8_t* bytes = new byte[available];
   this->read_array(bytes, available);
   this->msg_buff += this->convert(bytes, available);

   // find delimiter, thus a whole msg, send it to processor, then remove from buffer and keep remainder (if any)
   size_t pos = this->msg_buff.find(DELIMITER);
   if (pos != std::string::npos) {
      std::string sub = this->msg_buff.substr(0, pos + DELIMITER_LENGTH);
      this->rx_queue.push(sub);
      ESP_LOGD(TAG, "UART RX[%d]: %s", this->rx_queue.size(), sub.c_str());
      this->msg_buff = this->msg_buff.substr(pos + DELIMITER_LENGTH); //, this->msg_buff.length() - pos);
   }
}

void GatePro::write_uart() {
   if (this->tx_queue.size()) {
      std::string tmp = this->tx_queue.front();
      tmp += TX_DELIMITER;
      const char* out = tmp.c_str();
      this->write_str(out);
      ESP_LOGD(TAG, "UART TX[%d]: %s", this->tx_queue.size(), out);
      this->tx_queue.pop();
   }
}

////////////////////////////////////////////
// Component functions
////////////////////////////////////////////
cover::CoverTraits GatePro::get_traits() {
   auto traits = cover::CoverTraits();
   traits.set_is_assumed_state(false);
   traits.set_supports_position(true);
   traits.set_supports_tilt(false);
   traits.set_supports_toggle(true);
   traits.set_supports_stop(true);
   return traits;
}

void GatePro::setup() {
   ESP_LOGD(TAG, "Setting up GatePro component..");
   this->last_operation_ = cover::COVER_OPERATION_CLOSING;
   this->current_operation = cover::COVER_OPERATION_IDLE;
   this->operation_finished = false;
   this->startup = true;
   this->queue_gatepro_cmd(GATEPRO_CMD_READ_STATUS);
   this->target_position_ = 0.0f;
   this->queue_gatepro_cmd(GATEPRO_CMD_READ_PARAMS);
   this->queue_gatepro_cmd(GATEPRO_CMD_DEVINFO);
   this->queue_gatepro_cmd(GATEPRO_CMD_READ_LEARN_STATUS);

   // set up frontend controllers
   if (btn_learn) {
      this->btn_learn->add_on_press_callback([this](){
         this->queue_gatepro_cmd(GATEPRO_CMD_LEARN);
      });
   }

   if (btn_params_od) {
      this->btn_params_od->add_on_press_callback([this](){
         this->queue_gatepro_cmd(GATEPRO_CMD_READ_PARAMS);
      });
   }

   if (btn_remote_learn) {
      this->btn_remote_learn->add_on_press_callback([this](){
         this->queue_gatepro_cmd(GATEPRO_CMD_REMOTE_LEARN);
      });
   }

   if (btn_read_status) {
      this->btn_read_status->add_on_press_callback([this](){
         this->queue_gatepro_cmd(GATEPRO_CMD_READ_STATUS);
      });
   }
   
   // Sliders
   for (auto swi : this->sliders_with_indices) {
      swi.slider->add_on_state_callback(
         [this, swi](int value) {
            if (this->params[swi.idx] == value) {
               return;
            }
            this->set_param(swi.idx, value);
         }
      );
   }
   // Switches
   for (auto swi : this->switches_with_indices) {
      swi.switch_->add_on_state_callback(
         [this, swi](bool state) {
            if (this->params[swi.idx] == state) {
               return;
            }
            this->set_param(swi.idx, state ? 1 : 0);
         }
      );
   }
}

void GatePro::update() {
   this->publish();
   this->stop_at_target_position();

   this->write_uart();

   if (this->current_operation != cover::COVER_OPERATION_IDLE) {
      this->queue_gatepro_cmd(GATEPRO_CMD_READ_STATUS);
   }

   this->correction_after_operation();
}

void GatePro::loop() {
   this->process();
}

void GatePro::dump_config(){
   ESP_LOGCONFIG(TAG, "GatePro sensor dump config");
}

}  // namespace gatepro
}  // namespace esphome
