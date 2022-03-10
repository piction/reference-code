#include "motorData.h"
#include "log.h"

using namespace rapidjson;

MotorData::MotorData(const MqttData &mqttData) {
    // command parser
    _mqttData = mqttData;

    // Example : rbus/0628252/0000000000001/rbus.get.status/result
    // read the topic's info to get the id and the command
    std::regex motor_regex("\\/(\\d{7})\\/(\\d{13})\\/([^\\/]+)\\/([^\\/]+$)",std::regex_constants::ECMAScript | std::regex_constants::icase);
    std::string topicStr = mqttData.getTopic();
    std::smatch matches;
     if(std::regex_search( topicStr, matches, motor_regex)) {
         auto s = matches.size();
         if ( matches.size() != 5) {
             LOG_ERROR("Failed to parse and match MqttData to motorData!");    
             return;
         }        
         _id =  matches[1].str() + "/" + matches[2].str() ;
         _command = matches[3].str(); // for example rbus.get.status
         std::string action = matches[4].str();// for example 'result' or 'trigger'
         _hasInfo= (action=="result") && !mqttData.getPayload().empty();
         _isAck =_hasInfo || (topicStr.find("ack")!=std::string::npos) || (_mqttData.getPayload().find("ack")!=std::string::npos);
         
     } else {
         LOG_ERROR("Failed to parse MqttData to motorData!");
         return;
     }
}

void MotorData::parseOneValue(int & value) const {
   if(!_hasInfo) {
       LOG_CRITICAL_THROW("Can not parse value when no info present when parseOneValue!");
   }
    Document d = _mqttData.getParsedJsonDoc();
    Value& t = d["results"];
    if ( t.IsNumber()) {
        value = (int) (t.GetDouble());
    } else if ( t.IsString()) {
        std::string  str = t.GetString();
        value = std::stoi(str);
    } else {
        LOG_CRITICAL_THROW("Failed to parse a value");
    }
}

void MotorData::parseStatusData(MotorStatusData & motorStatus) const {
    // posPerc, posMM, VelMM, LockState, IsOpen,IsClosed, Temp, Current, button...
   if(!_hasInfo) {
       LOG_CRITICAL_THROW("Can not parse value when no info present when parseStatus!");
   }
    if ( _command!= "rbus.get.status") {
        LOG_CRITICAL_THROW("can not get the status of a non status mqttdata object!");
    }
    Document d = _mqttData.getParsedJsonDoc();
    //{"results":"0,0,0,false,false,true,34,20,false,false,false,false,true,false"}
    Value& t = d["results"];
    if ( t.IsString()) {
        std::string result = t.GetString();
        size_t pos = 0;
        std::string token;
        std::string delimitor = ",";
        int count = 0;
        while ((pos = result.find(delimitor)) != std::string::npos) {
            token = result.substr(0, pos);
            if ( count == 0) {
                 motorStatus.posMm = std::stoi(token);
            }
            else if ( count == 1) {
                // don't use Posperc
            } else if ( count == 2) {
                motorStatus.speedMm = std::stoi(token);
            } else if ( count == 3) {
                motorStatus.isLocked =  (token == "true"); // expect exactly "true" or "false" -> Not case sensitive -> fixed api!
            } else if ( count == 4) {
                motorStatus.isOpen =  (token == "true"); // expect exactly "true" or "false" -> Not case sensitive -> fixed api!
            } else if ( count == 5) {
                motorStatus.isClosed =  (token == "true"); // expect exactly "true" or "false" -> Not case sensitive -> fixed api!
            // 6-> temperature
            // 7-> current
            // 8-> button 0
            // 9-> button 1
            // 10-> button 2
            // 11-> bool-button 0
            // 12-> bool-button 1
            } else if ( count == 13) {
                motorStatus.isCalibrated =  (token == "true"); // expect exactly "true" or "false" -> Not case sensitive -> fixed api!                
            } else if ( count == 14) {
                motorStatus.isEmergencyRun =  (token == "true"); // expect exactly "true" or "false" -> Not case sensitive -> fixed api!
            }               
            result.erase(0, pos + delimitor.length());
            count ++;
        }
    } else {
        LOG_CRITICAL("Failed to parse result of " + _mqttData.getPayload());
    }
}
std::string MotorData::getId() const {
    return _id;
}


