#pragma once

namespace esphome {
namespace gatepro {

// Mappings
enum GateProCmd : uint8_t {
   GATEPRO_CMD_NONE,
   GATEPRO_CMD_OPEN,
   GATEPRO_CMD_CLOSE,
   GATEPRO_CMD_STOP,
   GATEPRO_CMD_READ_STATUS,
   GATEPRO_CMD_READ_PARAMS,
   GATEPRO_CMD_WRITE_PARAMS,
   GATEPRO_CMD_LEARN,
   GATEPRO_CMD_DEVINFO,
   GATEPRO_CMD_READ_LEARN_STATUS,
   GATEPRO_CMD_REMOTE_LEARN,
   GATEPRO_CMD_CLEAR_REMOTE_LEARN, // untested
   GATEPRO_CMD_RESTORE, // untested
   GATEPRO_CMD_PED_OPEN, // untested
   GATEPRO_CMD_READ_FUNCTION, // untested
};  

const std::map<GateProCmd, const char*> GateProCmdMapping = {
   {GATEPRO_CMD_NONE, "NAK"},
   {GATEPRO_CMD_OPEN, "FULL OPEN;src=P00287D7"},
   {GATEPRO_CMD_CLOSE, "FULL CLOSE;src=P00287D7"},
   {GATEPRO_CMD_STOP, "STOP;src=P00287D7"},
   {GATEPRO_CMD_READ_STATUS, "RS;src=P00287D7"},
   {GATEPRO_CMD_READ_PARAMS, "RP,1:;src=P00287D7"},
   {GATEPRO_CMD_WRITE_PARAMS, "WP,1:"},
   {GATEPRO_CMD_LEARN, "AUTO LEARN;src=P00287D7"},
   {GATEPRO_CMD_DEVINFO, "READ DEVINFO;src=P00287D7"},
   {GATEPRO_CMD_READ_LEARN_STATUS, "READ LEARN STATUS;src=P00287D7"},
   {GATEPRO_CMD_REMOTE_LEARN, "REMOTE LEARN;src=P00287D7"},
   {GATEPRO_CMD_CLEAR_REMOTE_LEARN, "CLEAR REMOTE LEARN;src=P00287D7"},
   {GATEPRO_CMD_RESTORE, "RESTORE;src=P00287D7"},
   {GATEPRO_CMD_PED_OPEN, "PED OPEN;src=P00287D7"},
   {GATEPRO_CMD_READ_FUNCTION, "READ FUNCTION;src=P00287D7"},
};

enum GateProMsgType : uint8_t {
   GATEPRO_MSG_UNKNOWN,
   GATEPRO_MSG_ACK_RS,
   GATEPRO_MSG_ACK_RP,
   GATEPRO_MSG_ACK_WP,
   GATEPRO_MSG_MOTOR_EVENT,
   GATEPRO_MSG_ACK_READ_DEVINFO,
   GATEPRO_MSG_ACK_LEARN_STATUS,
   MOTOR_EVENT_OPENING,
   MOTOR_EVENT_OPENED,
   MOTOR_EVENT_CLOSING,
   MOTOR_EVENT_AUTOCLOSING, // defined for possible future usage, but currently unused
   MOTOR_EVENT_CLOSED,
   MOTOR_EVENT_STOPPED,
};

struct GateProMsgConstant {
   int pos;
   int len;
   std::string match;
};

const std::map<GateProMsgType, const GateProMsgConstant> GateProMsgTypeMapping = {
   {GATEPRO_MSG_ACK_RS, {0, 6, "ACK RS"}},
   {GATEPRO_MSG_ACK_RP, {0, 6, "ACK RP"}},
   {GATEPRO_MSG_ACK_RP, {0, 6, "ACK WP"}},
   {GATEPRO_MSG_MOTOR_EVENT, {0, 7, "$V1PKF0"}},
   {GATEPRO_MSG_ACK_READ_DEVINFO, {0, 16, "ACK READ DEVINFO"}},
   {GATEPRO_MSG_ACK_LEARN_STATUS, {0, 16, "ACK LEARN STATUS"}},
};

const std::map<GateProMsgType, const GateProMsgConstant> MotorEvents = {
   {MOTOR_EVENT_OPENING, {11, 7, "Opening"}},
   {MOTOR_EVENT_OPENED, {11, 6, "Opened"}},
   {MOTOR_EVENT_CLOSING, {11, 7, "Closing"}},
   {MOTOR_EVENT_CLOSING, {11, 11, "AutoClosing"}}, // for all currently known scopes, autoclose === close
   {MOTOR_EVENT_CLOSED, {11, 6, "Closed"}},
   {MOTOR_EVENT_STOPPED, {11, 7, "Stopped"}},
};

const std::map<int, const std::string> ConversionMap {
   {7, "\\a"},
   {8, "\\b"},
   {9, "\\t"},
   {10, "\\n"},
   {11, "\\v"},
   {12, "\\f"},
   {13, "\\r"},
   {27, "\\e"},
   {34, "\\\""},
   {39, "\\'"},
   {92, "\\\\"},
};

/* Misc constants
*/
const std::string DELIMITER = "\\r\\n";
const uint8_t DELIMITER_LENGTH = DELIMITER.length();
const std::string TX_DELIMITER = "\r\n";

// black magic shit. Somehow the percentage of position is offset by this number
const int KNOWN_PERCENTAGE_OFFSET = 128;
// maximum acceptable difference of target pos / current pos in %
const float ACCEPTABLE_DIFF = 0.05f;
// status percentage location
const GateProMsgConstant STATUS_PERCENTAGE = {16, 2, ""};
const GateProMsgConstant PARAMS = {9, 33, ""};
const std::string WRITE_PARAMS_START = "WP,1:";


}}