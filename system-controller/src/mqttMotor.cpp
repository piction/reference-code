
#include "mqttMotor.h"
#include "log.h"

#define LOG_MOTOR_CRITICAL(...) LOG_CRITICAL("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_CRITICAL_THROW(...) LOG_CRITICAL_THROW("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_ERROR(...) LOG_ERROR("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_WARNING(...) LOG_WARNING("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_DEBUG(...) LOG_DEBUG("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_INFO(...)  LOG_INFO("motor " + this->getId() + ":" + __VA_ARGS__);
#define LOG_MOTOR_TRACE(...) LOG_TRACE("motor " + this->getId() + ":" + __VA_ARGS__);

MqttMotor::MqttMotor(const std::string& pn,const std::string& serial)
: _baseTopic("rbus/"+pn + "/" + serial +"/")
, _id(pn + "/" + serial)
, _commandsManager(std::make_shared<CommandsManager>()) {   
    
}
MqttMotor::~MqttMotor() {
    onMotorDisconnected();
    LOG_MOTOR_TRACE("Deconstruct MqttMotor");
}
void MqttMotor::onMotorConnected() {
    //start worker for configuration and on polling motor data
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _isMotorConfigured=false; 
        _pollingActive = true;
    }
    LOG_MOTOR_INFO("connected");
    
    _commandsManager->startEvaluating();
    _workerThread = std::thread(&MqttMotor::polStatus, this);
    if (!_motorDelegateOutputIsSet) {
         LOG_CRITICAL("no delegate was set for the motor output!");
    }
}
void MqttMotor::polStatus() {
    LOG_MOTOR_DEBUG("start retrieve max/min speed");
    // configuring
    do {
        _commandsManager->pushCommandoToBeSend(MqttData(_baseTopic + "rbus.get.maxspeed/trigger","","_x_" ),CommandType::Get);
        _commandsManager->pushCommandoToBeSend(MqttData(_baseTopic + "rbus.get.minspeed/trigger","","_x_" ),CommandType::Get);        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } while(!_isMotorConfigured);
    // polling status data
    LOG_MOTOR_DEBUG(" start polling");
    auto start = std::chrono::system_clock::now();
    while(_pollingActive) {           
        start = std::chrono::system_clock::now();

        // when the motor is calibrated and the stroke is not known sendout the get-stroke command
        if ( isCalibrated() && _stroke <= 0 ) {            
            _commandsManager->pushCommandoToBeSend(MqttData(_baseTopic + "rbus.get.stroke/trigger","","_x_" ),CommandType::Get);
        } 
        // poll for the status
        _commandsManager->pushCommandoToBeSend(MqttData(_baseTopic + "rbus.get.status/trigger","","_x_" ),CommandType::Get);
            
        // wait maximum for 250ms 
        std::mutex mutex;
        std::unique_lock<std::mutex> lk(mutex);
        _cv.wait_for(lk, std::chrono::milliseconds(250)) ;         
        auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        if ( diff < 100) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100-diff)); //sleep at least 100ms
        }
    }  
}
void MqttMotor::onMotorDisconnected() {
    LOG_MOTOR_INFO("disconnect");
    _isMotorConfigured = true; //stop the loop of motorconfiguration
    _pollingActive = false;
    cancelAsyncTasks();
    _commandsManager->stopEvaluating();
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
}
void MqttMotor::setDelegateMotorOutput(std::function<void(MqttData)> delegateMotorOutput) {
    _delegateMotorOutput = delegateMotorOutput;
    _commandsManager->setSendHandler(_delegateMotorOutput);
    _motorDelegateOutputIsSet = true;
}
void MqttMotor::setHighSpeed() {
    if ( _currentTargetSpeed == _highSpeed){return;}
     _currentTargetSpeed = _highSpeed;
    LOG_MOTOR_TRACE("set high speed");
    MqttData data(_baseTopic + "rbus.set.speed/trigger",_highSpeed,"_x_" );
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetParam);
}
void MqttMotor::setLowSpeed() {
    if ( _currentTargetSpeed == _lowSpeed){return;}
    _currentTargetSpeed = _lowSpeed;
    LOG_MOTOR_TRACE("set low speed");
    MqttData data(_baseTopic + "rbus.set.speed/trigger",_lowSpeed,"_x_" );
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetParam);
}
void MqttMotor::limitSpeedIfNeeded () {
 if ( _currentTargetSpeed == _lowSpeed) {
        MqttData data(_baseTopic + "rbus.set.speed/trigger",_lowSpeed,"_x_" );
        _commandsManager->pushCommandoToBeSend(data,CommandType::SetParam);
    }
}

void MqttMotor::stop() {
    if ( _isMotorStopped) return;
    LOG_MOTOR_TRACE("stop triggered");
    MqttData data(_baseTopic + "rbus.stop/trigger","","_x_");
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetMovement);
}

void MqttMotor::close() {
    if (getMotorStatusData().getStatus() == MotorStatus::Closed) return;
    {
         std::lock_guard<std::mutex> lock(_mutex);
         _isMotorStopped = false;
    }
    LOG_MOTOR_TRACE("close triggerd");
    MqttData data(_baseTopic + "rbus.close/trigger","","_x_");
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetMovement);
    limitSpeedIfNeeded();
}
void MqttMotor::open() {
    if (getMotorStatusData().getStatus() == MotorStatus::Open) return;
    {
         std::lock_guard<std::mutex> lock(_mutex);
         _isMotorStopped = false;
    }
    LOG_MOTOR_TRACE("open triggerd");
    MqttData data(_baseTopic + "rbus.open/trigger","","_x_");
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetMovement);
    limitSpeedIfNeeded();   
}
void MqttMotor::setPosition(int position) {
    {
         std::lock_guard<std::mutex> lock(_mutex);
         _isMotorStopped = false;
    }
    LOG_MOTOR_TRACE("set position triggerd");
    MqttData data(_baseTopic + "rbus.set.position.mm/trigger",position,"_x_");
    _commandsManager->pushCommandoToBeSend(data,CommandType::SetParam);
    limitSpeedIfNeeded();
}
bool MqttMotor::clearCalibrationWorker(std::future<void> cancelObj)  {
    LOG_MOTOR_TRACE("Clear calibration");
    _stroke = -1;
    int counter =0;
    auto genUserLevel = [](int level) {
        uint32_t boundMin = 1000000;
        uint32_t boundMax =4623215;
        uint32_t prime = 929;        		
        uint32_t x =  (rand() % ((boundMax + 1) - boundMin) + boundMin) * prime;
        if ( level == 0) { 
            return "2175268," + std::to_string(x+level);
        } else if ( level == 1) {
            return "2470658," + std::to_string(x+level);
        } else {
            return "2362243," + std::to_string(x+2); // level max is 2
        }
    };

    do 
    {        
        if(cancelObj.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
            LOG_MOTOR_TRACE("cancel clear calibration");
            break;
        }
        if ( !(counter % 5) ) { // only sendout at interval of 5 times sleeping
      
            MqttData data_userLevel(_baseTopic + "rbus.set.userlevel/trigger",genUserLevel(2),"_x_");
            _commandsManager->pushCommandoToBeSend(data_userLevel,CommandType::SetParam);
      
            MqttData data_clearCalib(_baseTopic + "config.calib.clear/trigger","","_x_");
            _commandsManager->pushCommandoToBeSend(data_clearCalib,CommandType::SetMovement);
        }
        if ( counter > 1000) {break;}
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } while(isCalibrated());
    LOG_MOTOR_DEBUG( (isCalibrated() ? "Failed to clear calibration" : "Calibration cleared") );
    return !isCalibrated();
}

std::future<bool> MqttMotor::clearCalibration() {
    _cancelWorkerSignal = std::promise<void>();
    std::future<void> cancelObj = _cancelWorkerSignal.get_future();
    LOG_INFO("calibration clear async worker started");
    return std::async(std::launch::async, &MqttMotor::clearCalibrationWorker,this,std::move(cancelObj));
}
void MqttMotor::cancelAsyncTasks() {
    _cancelWorkerSignal.set_value(); //set void value to flag a cancel    
    _cancelWorkerSignal = std::promise<void>();
}

void MqttMotor::onMotorInput(const MotorData & data){
    
    // if update motor position (only relevant when motor is configured):     
    _cv.notify_all(); 

    
    if (data.isAck()) {
        _commandsManager->handleAck(data.getMqttData());
    }
    if (! data.hasInfo()) return;
        
    if (_isMotorConfigured && data.getCommand() == "rbus.get.status") {        
        
        bool shouldNotUpdatePositionDueManualIntervention = false;        
        {
            std::lock_guard<std::mutex> lock(_mutex) ;
            data.parseStatusData(_currentMotorStatusData );

            // if motor was stopped but moved without giving a command -> manual intervention, ignore update positions            
            shouldNotUpdatePositionDueManualIntervention = (_isMotorStopped && !_currentMotorStatusData.isMotorStopped());
            _isMotorStopped = _currentMotorStatusData.isMotorStopped();                      
        }
        // the update can be skipped to not actuate the motors on a manual intervention
        // always update motion data when emergency run is detected!
        if(_currentMotorStatusData.isEmergencyRun || !shouldNotUpdatePositionDueManualIntervention) {
            MotorMotionManager::updateMotionData(_currentMotorStatusData);
        }
        
    } else if (data.getCommand() == "rbus.get.minspeed") {
        data.parseOneValue(_lowSpeed);
         {
            std::lock_guard<std::mutex> lock(_mutex);   
            _isMotorConfigured = (_lowSpeed!=0) && (_highSpeed!=0);
        }
    } else if (data.getCommand() == "rbus.get.maxspeed") {
        data.parseOneValue(_highSpeed);
         {
            std::lock_guard<std::mutex> lock(_mutex);   
            _isMotorConfigured = (_lowSpeed!=0) && (_highSpeed!=0);
        }
    } else if (data.getCommand() == "rbus.get.stroke") {
        LOG_DEBUG("Received stroke result");
        data.parseOneValue(_stroke);
        if ( !isCalibrated()) {
            LOG_DEBUG("Ignored stroke result due flag IsCalibrated=False");
            _stroke = -1;
        }
    } else if (data.getCommand() == "rbus.get.emergencyrun") {
        LOG_ERROR("Command '" + data.getCommand() + "' not HANDLED");
    } else {
       // LOG_TRACE("Command '" + data.getCommand() + "' not used");
        // do nothing at the moment
    }   
   
}




