#pragma once

#include <map>
#include <vector>
#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"
#include "constants.h"

namespace esphome {
namespace gatepro {

class GatePro : public cover::Cover, public PollingComponent, public uart::UARTDevice {
   public:
      void set_btn_learn(esphome::button::Button *btn) { btn_learn = btn; }
      void set_btn_params_od(esphome::button::Button *btn) { btn_params_od = btn; }
      void set_btn_remote_learn(esphome::button::Button *btn) { btn_remote_learn = btn; }
      void set_btn_read_status(esphome::button::Button *btn) { btn_read_status = btn; }
      void set_txt_devinfo(esphome::text_sensor::TextSensor *txt) { txt_devinfo = txt; }
      void set_txt_learn_status(esphome::text_sensor::TextSensor *txt) { txt_learn_status = txt; }
      void set_number(u_int param_idx, number::Number *slider) {
         this->sliders_with_indices.push_back(NumberWithIdx(param_idx, slider));
      }
      void set_switch(u_int param_idx, switch_::Switch *switch_) {
         this->switches_with_indices.push_back(SwitchWithIdx(param_idx, switch_));
      }

      void set_select(select::Select *sel, std::vector<std::string> *options, std::vector<int> *values) {
         select_with_data.push_back(sel, options, values);
      }

      void setup() override;
      void update() override;
      void loop() override;
      void dump_config() override;
      cover::CoverTraits get_traits() override;

   protected:
      // helpers
      std::string current_msg;
      bool read_msg();
      GateProMsgType identify_current_msg_type(std::map<GateProMsgType, const GateProMsgConstant>);
      int get_position_percentage();
      bool is_moving();
      std::string convert(uint8_t*, size_t);

      // device logic
      int after_tick = AFTER_TICK_MAX;
      float target_position_;
      float position_;
      bool operation_finished;
      bool startup;
      cover::CoverCall* last_call_;
      cover::CoverOperation last_operation_{cover::COVER_OPERATION_OPENING};
      void queue_gatepro_cmd(GateProCmd cmd);
      void control(const cover::CoverCall &call) override;
      void start_direction_(cover::CoverOperation dir);
      void stop_at_target_position();
      void correction_after_operation();
      void process();

      // param logic
      std::vector<int> params;
      std::string params_cmd;
      bool param_no_pub = false;
      std::queue<std::function<void()>> paramTaskQueue;
      void publish_params();
      void write_params();
      void parse_params();
      void set_param(int idx, int val);


      // sensor logic
      void publish();

      // UART
      std::string msg_buff;
      std::queue<const char*> tx_queue;
      std::queue<std::string> rx_queue;
      void read_uart();
      void write_uart();

      // UI
      esphome::button::Button *btn_learn;
      esphome::button::Button *btn_params_od;
      esphome::button::Button *btn_remote_learn;
      esphome::button::Button *btn_read_status;
      text_sensor::TextSensor *txt_devinfo{nullptr};
      text_sensor::TextSensor *txt_learn_status{nullptr};
      struct NumberWithIdx{
         u_int idx;
         number::Number *slider;
         NumberWithIdx(u_int idx, number::Number *slider) : idx(idx), slider(slider) {};
      };
      std::vector<NumberWithIdx> sliders_with_indices;
      struct SwitchWithIdx{
         u_int idx;
         switch_::Switch *switch_;
         SwitchWithIdx(u_int idx, switch_::Switch *switch_) : idx(idx), switch_(switch_) {};
      };
      std::vector<SwitchWithIdx> switches_with_indices;

      struct SelectWithIdxOpts{
         switch_::Switch *switch_;
         u_int idx;
         std::vector<std::string> options;
         std::vector<int> values;
      };
      std::vector<SelectWithIdxOpts> select_with_data;
};

}  // namespace gatepro
}  // namespace esphome
