#include <mutex>
#include <condition_variable>
#include <thread>
#include <utility>
#include "log.h"
#include "topicHandler.h"


TopicHandler::TopicHandler(std::vector<std::string>  subscribeStrs)
    : _running(false)
    , _subscribeStrs(std::move(subscribeStrs))
    , _pInTypeBuffer(std::make_shared<Buffer<MqttData>> ())
    , _pOutTypeBuffer(std::make_shared<Buffer<MqttData>> ())
{}

TopicHandler::TopicHandler():_running(false){}

TopicHandler::~TopicHandler() {
    if (_running) {
        stop();
    }
}
bool TopicHandler::isRunning() const {
    return _running;
}
void TopicHandler::start() {
    if (_running) {
        LOG_WARNING("Topichandler tried to be restarted");
        return;
    }
    LOG_DEBUG("Topichandler started");
    _running = true;
    _workerThread = std::thread(&TopicHandler::run, this);
}

void TopicHandler::stop() {
    if (!_running) {
        LOG_WARNING("Topichandler tried to be stoped again");
        return;
    }
    LOG_DEBUG("Topichandler stopped");
    _running = false;
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
}

// to be overwritten in derived classes.
// this just returns the same message to the outputbuffer to be send out
void TopicHandler::handleNewInput ( const MqttData & inputData) {
    LOG_WARNING("Nothing done with an input message!");
    _pOutTypeBuffer->QueueNewMessage(inputData);
}

// The inputbuffer is filled somewhere and with the run function 
// the buffer is unqueued and the handled
void TopicHandler::run() {
    LOG_DEBUG("Topichandler starts running ...");
    while (_running) {
        MqttData inputData;
        const int ret = _pInTypeBuffer->UnqueueMessage(inputData);

        if (ret == 0) { //false notify, no data available
            std::mutex mutex;
            std::unique_lock<std::mutex> lk(mutex);
            _pInTypeBuffer->conditionVariable().wait_for(lk, std::chrono::milliseconds(250));
            continue;
        }
        handleNewInput(inputData);     
    }
}

const std::shared_ptr<Buffer<MqttData>> TopicHandler::getInputBuffer() {
    return _pInTypeBuffer;
}
const std::shared_ptr<Buffer<MqttData>> TopicHandler::getOutputBuffer() {
    return _pOutTypeBuffer;
}

std::vector<std::string> TopicHandler::getSubscribeStrs() {
    return _subscribeStrs;
}

bool TopicHandler::isTopicValidForHandling(const std::string & topic) {
    for (auto & sub: _subscribeStrs) {
        // check topic subscription on current=> if match return true
        
        // subscription length should always be shorter
        if ( topic.size() < sub.size())
            continue; // no match
        
        bool isSearchingForNextMatch =false;
        std::string::size_type j = 0; // counter for sub Str
        std::string::size_type i = 0; // counter for topic Str
        for(; i < topic.size(); ++i) {
            if ( j >= sub.size()) {
                    break;
            } else {
                 if (isSearchingForNextMatch) {
                    if( topic[i] == '/' ) {
                        isSearchingForNextMatch = false;
                    } else {
                        continue; //don't update the j-counter
                    }
                }
                else if ( topic[i] != sub[j]) {
                    if ( sub[j]=='#') {
                        if (j==0 || topic[j-1]=='/') {
                            return true;
                        } else { 
                            break;
                        }
                    } else if (sub[j]=='+') {
                        isSearchingForNextMatch=true;  
                        j++;// point to '/'
                        continue;                 
                    } else {                   
                        break;
                    }
                }
                j++;
            }
        }
        if ( i >= topic.size() && !isSearchingForNextMatch)
            return true;

    }
    return false;

}