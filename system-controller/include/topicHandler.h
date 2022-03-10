#ifndef TOPICHANDLER_H
#define TOPICHANDLER_H

#include "pch.h"
#include "buffer.h"
#include "mqttData.h"

class TopicHandler {
    public:    
        virtual ~TopicHandler();
        TopicHandler(std::vector<std::string> subscribeStrs);
        TopicHandler(TopicHandler &&) = default; // move constructor
        virtual void start();
        virtual void stop();
        bool isRunning() const;
        const std::shared_ptr<Buffer<MqttData>> getInputBuffer();
        const std::shared_ptr<Buffer<MqttData>> getOutputBuffer();
        std::vector<std::string> getSubscribeStrs();
        bool isTopicValidForHandling(const std::string & topic);
        virtual std::string getType() const {return "TOPIC";}
    protected:
        TopicHandler();
        virtual void handleNewInput ( const MqttData & inputData) ;
        void run();
    protected:
        bool _running;
        std::vector<std::string> _subscribeStrs;        
        std::thread _workerThread;

        std::shared_ptr<Buffer<MqttData>> _pInTypeBuffer;
        std::shared_ptr<Buffer<MqttData>> _pOutTypeBuffer;
};

#endif //TOPICHANDLER_H

