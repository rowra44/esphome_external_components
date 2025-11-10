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
   {GATEPRO_CMD_WRITE_PARAMS, "WP,1:"}, // this is only the start of the cmd! followed by values
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
   GATEPRO_MSG_ACK_FULL_CLOSE,
   GATEPRO_MSG_ACK_FULL_OPEN,
   GATEPRO_MSG_ACK_STOP,
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
   // ACK RS:00,80,C4,C6,3E,16,FF,FF,FF\r\n
   {GATEPRO_MSG_ACK_RS, {0, 6, "ACK RS"}},
   // ACK RP,1:1,0,0,1,2,2,0,0,0,3,0,0,3,0,0,0,0\r\n"
   {GATEPRO_MSG_ACK_RP, {0, 6, "ACK RP"}},
   // ACK WP,1\r\n
   {GATEPRO_MSG_ACK_RP, {0, 6, "ACK WP"}},
   // $V1PKF0,17,Closed;src=0001\r\n
   {GATEPRO_MSG_MOTOR_EVENT, {0, 7, "$V1PKF0"}},
   // ACK READ DEVINFO:P500BU,PS21053C,V01\r\n
   {GATEPRO_MSG_ACK_READ_DEVINFO, {0, 16, "ACK READ DEVINFO"}},
   // ACK LEARN STATUS:SYSTEM LEARN COMPLETE,0\r\n
   {GATEPRO_MSG_ACK_LEARN_STATUS, {0, 16, "ACK LEARN STATUS"}},
   // ACK FULL CLOSE\r\n
   {GATEPRO_MSG_ACK_FULL_CLOSE, {0, 14, "ACK FULL CLOSE"}},
   // ACK FULL OPEN\r\n
   {GATEPRO_MSG_ACK_FULL_OPEN, {0, 13, "ACK FULL OPEN"}},
   // ACK STOP\r\n
   {GATEPRO_MSG_ACK_STOP, {0, 8, "ACK STOP"}},
};

const std::map<GateProMsgType, const GateProMsgConstant> MotorEvents = {
   // $V1PKF0,??,Opening;src=0001\r\n
   {MOTOR_EVENT_OPENING, {11, 7, "Opening"}},
   // $V1PKF0,??,Opened;src=0001\r\n
   {MOTOR_EVENT_OPENED, {11, 6, "Opened"}},
   // $V1PKF0,??,Closing;src=0001\r\n
   {MOTOR_EVENT_CLOSING, {11, 7, "Closing"}},
   // $V1PKF0,??,AutoClosing;src=0001\r\n
   {MOTOR_EVENT_CLOSING, {11, 11, "AutoClosing"}}, // for all currently known scopes, autoclose === close
   // $V1PKF0,??,Closed;src=0001\r\n
   {MOTOR_EVENT_CLOSED, {11, 6, "Closed"}},
   // $V1PKF0,??,Stopped;src=0001\r\n
   {MOTOR_EVENT_STOPPED, {11, 7, "Stopped"}},
};

// necessary for comfortable processing & to avoid confusion
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

// maximum acceptable difference of target pos / current pos in %
const float ACCEPTABLE_DIFF = 0.05f;
// ticks to update after an operation
const int AFTER_TICK_MAX = 5;
// status percentage location
      // example: ACK RS:00,80,C4,C6,3E,16,FF,FF,FF\r\n
      //                          ^- percentage in hex
const GateProMsgConstant STATUS_PERCENTAGE = {16, 2, ""};
// status: if currently moving, 3rd token is C4
const GateProMsgConstant STATUS_OP_MOVING = {13, 2, "C4"};
// percentage is offset by +128 when opening, so e.g. 50 => 50+128=178
const int PERCENTAGE_OFFSET_WHILE_OPENING = 128;
// example: ACK RP,1:1,0,0,1,2,2,0,0,0,3,0,0,3,0,0,0,0\r\n"
//                   ^-9  
const GateProMsgConstant PARAMS = {9, 33, ""};
const std::string PARAMS_SEPARATOR = ",";

}}