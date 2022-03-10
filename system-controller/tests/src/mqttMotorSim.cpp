#include "mqttMotorSim.h"


MqttMotorSim::MqttMotorSim (std::string ip, const int port, const std::string &mqttId,std::shared_ptr<IMqttMotor> motorRef )     
    : mosquittopp(mqttId.data())
    , _baseTopic("rbus/"+ motorRef->getId() +"/")
    , _id (motorRef->getId())
    , m_ip(std::move(ip))
    , m_port(port)
{
    LOG_INFO("Setup connection simulated motor at ip: " + m_ip );
        
    const int connRet = this->connect(m_ip.data(), m_port);
    if (connRet != MOSQ_ERR_SUCCESS) {
       LOG_ERROR("failed to connect to MQTT server: " +  std::string(mosqpp::strerror(connRet)));
    }
}  

void MqttMotorSim::on_disconnect(int rc) {return;}

void MqttMotorSim::on_message (const struct mosquitto_message *msg){
    return; // do nothing
}
void MqttMotorSim::on_connect(int rc){
    _isConnected=true;
    return; // do nothing
}

void MqttMotorSim::stop() {
    _isConnected=false;
    
    return;}
void MqttMotorSim::start(){return;}

bool MqttMotorSim::waitForConnection(const std::chrono::milliseconds timeout){

    typedef std::chrono::high_resolution_clock Clock;
    Clock::time_point t0 = Clock::now();

    for ( ;; ) {
        if (_isConnected) {return true;}
        Clock::time_point t1 = Clock::now();
        if ((t1 - t0) > timeout) {return false;}
        loop();
    }
}

void MqttMotorSim::commandPositionPerc(int percentage) {
    LOG_DEBUG("Set sim-motor [" + _id  + "] at " + std::to_string(percentage)+ "%");
    MqttData data(_baseTopic + "sim.jump.position.perc/trigger",percentage*100,"todo_id" );
    SendCommand(data);
}
void MqttMotorSim::jumpToRandomPos() {
    commandPositionPerc(rand() % 100 + 1);
}


void MqttMotorSim::SendCommand(MqttData & data){
    const std::string& topic = data.getTopic();
    const std::string& payload = data.getPayload();

    const auto publishRet = publish(0, topic.data(), payload.size(), payload.data(), 0, false);
}