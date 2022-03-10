#ifndef MOS_To_MQTTDATA_H
#define MOS_To_MQTTDATA_H

#include "pch.h"
#include "mqttData.h"



class MosquittoToMqttDataConverter {
    public:
        static MqttData CreateMqttData( const struct mosquitto_message *msg) {
            
            std::string msgTopic = (char *) msg->topic;    
            std::string payload = (msg->payloadlen != 0) ? ((char *) msg->payload) : "";
            return MqttData(msgTopic,payload);
        }
};


#endif //MOS_To_MQTTDATA_H