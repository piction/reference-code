#ifndef MQTTDATA_H
#define MQTTDATA_H

#include "pch.h"
#include "buffer.h"
#include <unordered_set>

class MqttData {
    public:
        MqttData(){};
        MqttData(const std::string & topic);
        MqttData(const std::string & topic , const std::string & payload);
        MqttData(const std::string & topic , const std::string & parameter, const std::string & id);
        MqttData(const std::string & topic , int parameterValue, const std::string & id);

        std::string getTopic() const { return _topic;}
        std::string getPayload() const { return _payload;}
        std::size_t getHash() const {
            std::size_t h1 = std::hash<std::string>{}(_topic);
            std::size_t h2 = std::hash<std::string>{}(_payload);
            return h1 ^ (h2 << 1); // combine hash
        }

        rapidjson::Document  getParsedJsonDoc() const ;     
        operator std::string() const { 
            if (_payload.find("\"parameters\":\"\"") != std::string::npos) {
                return _topic + " [ no params ]";
            } else  {
                return _topic + " [" + _payload + "]"; 
            }
        }

    private:
        std::string _topic;
        std::string _payload;
        
};




#endif //MQTTDATA_H