#ifndef MQTTMANAGER_h
#define MQTTMANAGER_h

#include "pch.h"
#include "topicHandler.h"
#include <mosquittoToMqttData.h>


// The MqttManager handles only incoming MQTT messages and outgoing messages
// To make this task of handling messages asynchroneous buffers are used.
// Subsequent, the mqttManager only interacts with those buffers. 
// The buffers are part of the topicHandlers. A topichandler will empty the 
// input buffer and will fill the output buffer to be sent.

// Sending Mqtt messages out 
// -> the outputbuffer of each topic is checked and all messages are pushed out

// Reading Mqtt messages
// -> On_message will be called when an MqttMessage is received. A filter function of the 
// topics is used to determine if the new message should go to the input bufer of the topic handler



class MqttManager : public mosqpp::mosquittopp {
    public:
        MqttManager(std::string ip, const int port, const std::string &mqttId , std::vector<std::shared_ptr <TopicHandler>> topicHandlers);
        ~MqttManager();
        void on_message (const struct mosquitto_message *msg) override;
        void on_connect(int rc) override;
        void on_disconnect(int rc) override;        
        void stop();
        void start();

        bool isConnected() const;

        bool waitForConnection(const std::chrono::milliseconds timeout);
        void waitForDisconnection();

        void addTopicHandler(std::shared_ptr<TopicHandler> topicHandler);        
    private:
        void setConnected(const bool connected);
        void setReadingRunning(const bool running);
        void setSendingRunning(const bool running);

        void mqttReading();
        void mqttSending();

        std::string getConnectionError(int rc );


    private:
        mutable std::mutex m_connectedMutex;
        mutable std::mutex m_runningMutex;

        std::vector<std::shared_ptr<TopicHandler>> _topicHandlers;
        bool m_sendingRunning;
        bool m_readingRunning;

        std::string m_ip;
        int m_port;

        std::thread m_sendingWorker;
        std::thread m_readingWorker;
};


#endif //MQTTMANAGER_h
