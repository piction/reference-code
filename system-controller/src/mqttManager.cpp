#include "mqttManager.h"

#include <utility>
#include "log.h"

MqttManager::MqttManager(std::string ip, const int port, const std::string &mqttId, std::vector<std::shared_ptr <TopicHandler>> topicHandlers)
    : mosquittopp(mqttId.data())
    , m_sendingRunning(false)
    , m_readingRunning(false)
    , m_ip(std::move(ip))
    , m_port(port)
    , _topicHandlers(std::move(topicHandlers))
{    
    /* Connect to server*/
    LOG_INFO("Setup connection at " + m_ip );
        
    const int connRet = this->connect(m_ip.data(), m_port);
    if (connRet != MOSQ_ERR_SUCCESS) {
       LOG_ERROR("failed to connect to MQTT server: " +  std::string(mosqpp::strerror(connRet)));
    }
}

MqttManager::~MqttManager() {
    LOG_DEBUG("Destruct mqttManager" );
    stop();
}

void LogStatus(const std::string& successMsg, const std::string& failMsg, int rc) {
    if (rc != MOSQ_ERR_SUCCESS) {                
        LOG_WARNING (failMsg + ": " +std::string(mosqpp::strerror(rc)));
    }
    if ( !successMsg.empty())
        LOG_INFO(successMsg);
}

void MqttManager::addTopicHandler(std::shared_ptr<TopicHandler> topicHandler){    
    // for now throw when running to prevent strange behaviour
    if (m_sendingRunning || m_readingRunning) {
        LOG_CRITICAL_THROW("MqttManager can not add a topichandler while running!");
    }

    _topicHandlers.push_back(topicHandler);
}

void MqttManager::start() {
    LOG_DEBUG("mqttManager starting" );
    setSendingRunning(true);
    m_sendingWorker = std::thread(&MqttManager::mqttSending, this);

    setReadingRunning(true);
    m_readingWorker = std::thread(&MqttManager::mqttReading, this);

   
}

void MqttManager::stop() {
    LOG_DEBUG("mqttManager stopping" );
    setReadingRunning(false);
    setSendingRunning(false);
     for (auto &h : _topicHandlers) {
        h->stop();
    }

    if (m_sendingWorker.joinable()) {
        m_sendingWorker.join();
    }

    if (m_readingWorker.joinable()) {
        m_readingWorker.join();
    }
     for ( auto & t : _topicHandlers) {
         t->stop();
    }
    LOG_DEBUG("mqttManager stopped" );
}

void MqttManager::setReadingRunning(const bool running) {
    std::lock_guard<std::mutex> lock(m_runningMutex);
    m_readingRunning = running;
}

void MqttManager::setSendingRunning(const bool running) {
    std::lock_guard<std::mutex> lock(m_runningMutex);
    m_sendingRunning = running;
}

// On a new message fill the input buffer of the correct topicHandler
void MqttManager::on_message (const struct mosquitto_message *msg){
    std::string msgTopic = (char *) msg->topic;    
    const MqttData data = MosquittoToMqttDataConverter::CreateMqttData(msg);
 
    for ( auto & t : _topicHandlers) {
        if(t->isTopicValidForHandling(msgTopic)) {
            t->getInputBuffer()->QueueNewMessage(data);
        }
    }    
}
void MqttManager::on_connect(int rc){
    
    LogStatus("MQTT on connect","MQTT on connect failed", rc);
    setConnected(true);

    for ( auto & t : _topicHandlers) {
        for (auto & s : t->getSubscribeStrs()) {
            const int subRet = this->subscribe(0, s.data());
            LogStatus("","MQTT subscribe failed " + s, rc);
        }
    }    
}
void MqttManager::on_disconnect(int rc){
    setConnected(false);
    LogStatus("MQTT on disconnect","MQTT on disconnect failed", rc);
}

void MqttManager::setConnected(const bool connected)
{    
    if (connected)
        m_connectedMutex.try_lock();
    else
        m_connectedMutex.unlock();
}

bool MqttManager::isConnected() const
{
    if (m_connectedMutex.try_lock()) {
        m_connectedMutex.unlock();
        return false;
    }
    return true;
}
// this function allows the caller of this class to hold the program untill 
// the reading is finished 
bool MqttManager::waitForConnection(std::chrono::milliseconds timeout)
{
    typedef std::chrono::high_resolution_clock Clock;
    Clock::time_point t0 = Clock::now();

    for ( ;; ) {
        if (isConnected()) {return true;}
        Clock::time_point t1 = Clock::now();
        if ((t1 - t0) > timeout) {return false;}
        loop();
    }
}

void MqttManager::waitForDisconnection()
{
    LOG_DEBUG("MqttManager: waitForDisconnection");
    m_connectedMutex.lock();
    m_connectedMutex.unlock();
    LOG_DEBUG("MqttManager: wait on disconnect finished");
}
// keep the reading of Mqtt message alive so that the on_message will receive new messages
void MqttManager::mqttReading()
{
    LOG_DEBUG("Mqtt started reading worker");
    for ( auto & t : _topicHandlers) {
         t->start();
    }
    while (m_readingRunning) {
        loop();
    }
}
// Sending Mqtt messages out 
// -> the outputbuffer of each topic is checked and all messages are pushed out
void MqttManager::mqttSending()
{
    LOG_DEBUG("Mqtt started sending worker");
    while (m_sendingRunning) {
        for ( auto & t : _topicHandlers) {
            {
                std::mutex mutex;
                std::unique_lock<std::mutex> lk(mutex);
                // wait on other thread of receiving a notify from the condition variable, timeout at 200 ms
                const std::cv_status status
                    = t->getOutputBuffer()->conditionVariable().wait_for(lk, std::chrono::milliseconds(200));
                // don't handle timeout of the conditionVariable different! If the notify is missed once it should be picked up 
                // next run. If we would skip next block of code every time a timeout occurred the system should wait until a
                // next message is pushed on the buffer to get notified again. If a notify was missed we would never pop this message
                // if now new message is put on the queue (triggering a new notify)
            }
            MqttData data;
            std::size_t lastDataHash=0; // make sure that there is send at least one message by setting hash on zero
            while (t->getOutputBuffer()->UnqueueMessage(data)) {
                // *** No lock needed here, resource is locked on the buffer it self
                std::size_t dataHash = data.getHash();
                if ( lastDataHash != dataHash) {    // prevent sending in burst the same message multiple times
                    lastDataHash = dataHash;
                    const std::string& topic = data.getTopic();
                    const std::string& payload = data.getPayload();

                    const auto publishRet = publish(0, topic.data(), payload.size(), payload.data(), 0, false);
                    // validate response and log when failed
                    LogStatus("","MQTT publish failed", publishRet);
                    if (std::string::npos == topic.find("get.status")) { // only log non status messages
                        LOG_TRACE("Published : " + (std::string)(data));
                    } 
                } else {
                    //LOG_TRACE("Show not send command:" + (std::string)(data));
                }
            } 
        }
    }
    LOG_DEBUG("MqttManager: Finished sending");    
}
