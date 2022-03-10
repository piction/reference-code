#include "wingData.h"
#include "log.h"

using namespace rapidjson;

WingData::WingData(const MqttData &mqttData) {
    // command parser
    _mqttData = mqttData;
    auto getCommand = [this](const std::string& cmd) -> WingComandType {    
        if(cmd=="open" || cmd=="Open"  ) {
            return WingComandType::Open;
        }
        if(cmd=="openOrStop" || cmd=="OpenOrStop" || cmd=="openorstop"  ) {
            return WingComandType::OpenOrStop;
        }
        if(cmd=="calibrate" || cmd=="Calibrate"  ) {
            return WingComandType::Calibrate;
        }
        if(cmd=="cancel" || cmd=="Cancel"  ) {
            return WingComandType::Cancel;
        }
        if(cmd=="stop" || cmd=="Stop"  ) {
            return WingComandType::Stop;
        }
        if(cmd=="close" || cmd=="Close"  ) {
            return WingComandType::Close;
        }
        if(cmd=="closeOrStop" || cmd=="CloseOrStop" || cmd=="closeorstop"  ) {
            return WingComandType::CloseOrStop;
        }
        if(cmd=="pulse" || cmd=="Pulse"  ) {
            return WingComandType::Pulse;
        }
        if(cmd=="pulseOrStop" || cmd=="PulseOrStop" || cmd=="pulseorstop"  ) {
            return WingComandType::PulseOrStop;
        }
        if(cmd=="lock" || cmd=="Lock"  ) {
            return WingComandType::Lock;
        }
        if(cmd=="setposition" || cmd=="setPosition"  || cmd.compare("SetPosition")==0  ) {
            return WingComandType::SetPosition;
        }        
        return WingComandType::Ignore;
    };
    // read the topic's info to get the id and the command
    std::regex wing_regex("\\/wing\\/([^\\/]+)\\/([^\\/]+$)",std::regex_constants::ECMAScript | std::regex_constants::icase);
    std::string topicStr = mqttData.getTopic();
    std::smatch matches;
    if(std::regex_search( topicStr, matches, wing_regex)) {
        if ( matches.size() != 3) {
            LOG_CRITICAL_THROW("Failed to parse MqttData to wingData!");    
        }        
        _id =  matches[1].str();
        _wingCommand = getCommand(matches[2].str());
    } else {
        LOG_CRITICAL_THROW("Failed to parse MqttData to wingData!");
    }
    
}

void WingData::parsePayload(const std::string& payload) {
    std::string positionType;
    Document d = _mqttData.getParsedJsonDoc();
    Value& t = d["parameters"]["positionType"];
    if ( t.IsString()) {
        positionType = t.GetString();
    }
    Value::MemberIterator it = d.FindMember("parameters");
  
    if(it != d.MemberEnd() && it->value.IsString()) {
        std::string posType = it->value.GetString();
    } else {
        LOG_CRITICAL_THROW("Failed to find the positionType for the wing data");
    }
    (void)it;
}
std::string WingData::getId() const {
    return _id;
}
WingComandType WingData::getWingCommand()const  {
    return _wingCommand;
}
// collect extra info when a setPosition is in the topic
void WingData::getPosition(double & perc, int & mm ){
    if ( !_positionSet && _wingCommand == WingComandType::SetPosition) {
        
        Document d = _mqttData.getParsedJsonDoc();
        Value& t = d["parameters"]["positionType"];
        if ( !t.IsString()) {
            LOG_CRITICAL_THROW("failed to parse the position type of the wing data parameters");
        }
        Value& v= d["parameters"]["value"];
        if(!v.IsNumber()) {
            LOG_CRITICAL_THROW("failed to parse the position value of the wing data parameters");
        }

        std::string positionType = t.GetString();
        if (positionType.compare("perc")==0) {
            _positionPerc = v.GetDouble();
        } else {
            _positionMm = (int) (v.GetDouble());
        }

        _positionSet=true;
    }
    perc = _positionPerc;
    mm = _positionMm;
}

MqttData WingData::getAck() {
    return MqttData(_mqttData.getPayload()+ "_ack");
}
