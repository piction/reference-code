#include "mqttData.h"

#include <utility>

MqttData::MqttData(const std::string & topic , const std::string & payload)  
    : _topic(topic)
    , _payload(payload) {
}
MqttData::MqttData(const std::string & topic) : MqttData ( topic, "")  {
}

MqttData::MqttData(const std::string & topic , const std::string & parameter, const std::string & id)
: MqttData ( topic, "{\"parameters\":\"" + parameter + "\",\"id\":\""+ id  +"\"}") {

}
MqttData::MqttData(const std::string & topic , int parameterValue, const std::string & id) 
: MqttData ( topic, "{\"parameters\":\"" + std::to_string(parameterValue) + "\",\"id\":\""+ id  +"\"}") {

}

rapidjson::Document MqttData::getParsedJsonDoc() const {
    rapidjson::Document doc;
    doc.Parse(_payload.data());
    if (doc.HasParseError()) {
        throw std::runtime_error("invalid JSON found :" + getPayload());
    }
    if (!doc.IsObject()) {
        throw std::runtime_error("unexpected JSON format, expected object as root: " + getPayload());
    }
    return doc;
}
