#ifndef MQTTMOTORSIM_H
#define MQTTMOTORSIM_H

#include "pch.h"
#include <mosquittoToMqttData.h>
#include "mqttMotor.h"




class MqttMotorSim : public mosqpp::mosquittopp {   
    public:
        MqttMotorSim (std::string ip, const int port, const std::string &mqttId ,std::shared_ptr<IMqttMotor> motorRef) ;     
        ~MqttMotorSim(){_isConnected=false;};
  
        void on_message (const struct mosquitto_message *msg) override;
        void on_connect(int rc) override;
        void on_disconnect(int rc) override;        
        void stop();
        void start();

        bool waitForConnection(const std::chrono::milliseconds timeout);
        void waitForDisconnection();

        void commandPositionPerc(int percentage);
        void jumpToRandomPos();

    private:
        std::string getConnectionError(int rc );

    private:
        void SendCommand(MqttData & data);
        std::string m_ip;
        int m_port;
        bool _isConnected=false;
                std::string _baseTopic;
        std::string _id;
};




#endif // MQTTMOTORSIM_H    