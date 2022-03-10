#include <gtest/gtest.h>

#include <fstream>
#include <functional>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctime>
#include <memory>
#include <vector>

#include "motorData.h"
#include "motorizedWindow.h"
#include "passiveWindow.h"
#include "mqttManager.h"
#include "wingsHandler.h"
#include "motorsHandler.h"
#include "mqttMotorSim.h"
#include "configBuilder.h"
#include "utils.h"
#include "motorParser.h"
#include "log.h"
#include "wingInputTranslator.h"

// The fixture for testing class Foo.
class CornerScenarioTest : public ::testing::Test {
   protected:

    CornerScenarioTest() {  }

    ~CornerScenarioTest() override {  } 

    std::vector<std::shared_ptr<IWing>> _wings;
    std::vector<std::shared_ptr<MqttMotorSim>> _simMotors;
    std::shared_ptr<MqttManager> _mqtt;
    int maxSecondsForTest = 20;
    std::string ipAdress="localhost";
    int port=1883;
    int numberOfRandomStartPositions = 2;   

    void SendAllWingsToClose() {
        for ( auto &w: _wings ) {w->close();}
    }
    void SendAllWingsToOpen() {
        for ( auto &w: _wings ) {w->open();}
    }
    bool AreAllWingsClosed() {
        return std::all_of(_wings.begin(),_wings.end(),[](std::shared_ptr<IWing> w)->bool {return w->getMasterWindow()->getMotionManager()->getMotorStatusData().getStatus() == MotorStatus::Closed;});
    }
    bool AreAllWingsOpen() {
        return std::all_of(_wings.begin(),_wings.end(),[](std::shared_ptr<IWing> w)->bool {return w->getMasterWindow()->getMotionManager()->getMotorStatusData().getStatus() == MotorStatus::Open;});
    }
   

  void SetUp() override {
    Log::Init();
    bool testIsSuccess = false;    
    std::ifstream f(utils::getApplicationDirectory() + "/testData/QXvXQ_withStrokes.json"); //taking file as inputstream
    std::ostringstream ss;
    ss << f.rdbuf(); // reading data    
    std::string json = ss.str();
    

    // get motors for simulators
    std::vector<motorparser::MotorParseResult> motors;
    motorparser::parse(json,true,motors);
       
 
    std::string configId;
    ConfigBuilder::parseFromJson(json,_wings,configId);
    LOG_INFO("parsed json to wings, wings-count="  + std::to_string(_wings.size()));
    
    auto wingsHandler = std::make_shared<WingsHandler>("dummyId", std::make_shared<WingInputTranslator>());    
    auto motorsHandler = std::make_shared<MotorsHandler>(); 

    for(auto &  wing : _wings) {
        std::string wingGuid = wingsHandler->addWing(wing);        
        for ( auto & m : wing->getMotors())  {
            LOG_INFO("Adding motor" + m->getMotionManager()->getId());
            motorsHandler->addMotor(std::dynamic_pointer_cast<IMqttMotor>(m->getMotionManager()));            
            _simMotors.push_back(std::make_shared<MqttMotorSim>(ipAdress,port,"sim-motor-"  + m->getMotionManager()->getId(),std::dynamic_pointer_cast<IMqttMotor>(m->getMotionManager())));
            _simMotors.back()->waitForConnection(std::chrono::milliseconds(500));
        }
    }
    std::vector<std::shared_ptr<TopicHandler>> topicHandlers;
    topicHandlers.push_back(motorsHandler);
    topicHandlers.push_back(wingsHandler);
    
    LOG_DEBUG("Launching "+std::to_string( motors.size()) + " simulator(s) ");
    for ( auto & m : motors) {
        std::ostringstream motorstr;
        motorstr << "../../../mqtt-simulated-motor/simulator/debug/simulator";  // todo change this when docker inplace!
        motorstr << " " << m.pn  << " " << m.serial << " " << m.stroke << " &";            
        system(motorstr.str().c_str()); // launch simulator with linux system command
        LOG_DEBUG("Launched simulator :" + motorstr.str());
    }    
    _mqtt= std::make_shared<MqttManager>(ipAdress,port,"systemcontroller",topicHandlers);
    const bool connected = _mqtt->waitForConnection(std::chrono::milliseconds(500));
    if (connected == false) {
        LOG_WARNING("Did not connect to Mqtt broker");
        return;
    }        
    _mqtt->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // give time to start the mqtt handlers and the simulators
  }

  void TearDown() override {
        _mqtt->stop();
        system("killall simulator");    
  }
 
};

// // start at a random position for the 2 simulated motors and try to go to the fully closed position
// TEST_F(CornerScenarioTest, randomStartPositionsToClose) {
//     Log::Init();
//     auto jumpToRandomPosition = [](std::vector<std::shared_ptr<MqttMotorSim>> & motors)  { 
//                for( auto &m: motors) {m->jumpToRandomPos();}
//         };

//     bool isSuccess =false;
//     int counter =0;
//     for (  ;counter< numberOfRandomStartPositions; counter++) {
//         LOG_DEBUG("***** TEST " + std::to_string(counter));

//         isSuccess =false;        
//         jumpToRandomPosition(_simMotors);
//         std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//         SendAllWingsToClose();        
//         auto start = std::chrono::system_clock::now();
//         while(true) {  
//             if( AreAllWingsClosed()) {
//                 isSuccess=true;
//                 LOG_DEBUG("****  SUCCESS  ****");
//                 break;
//             }           
//             std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//             auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//             if ( diff > maxSecondsForTest*1000) {
//                 LOG_DEBUG("****  TIME OUT  ****");
//                 break;
//             }
//         }                
//         EXPECT_TRUE(isSuccess) << "No failure";  
//     }   
//     EXPECT_EQ(counter,numberOfRandomStartPositions) << "No exceptions while closing should happen so all runs should be done";

// }

// // start at a random position for the 2 simulated motors and try to go to the fully open position
// TEST_F(CornerScenarioTest, randomStartPositionsToOpen) {
//     Log::Init();
    
//     auto jumpToRandomPosition = [](std::vector<std::shared_ptr<MqttMotorSim>> & motors)  { 
//                for( auto &m: motors) {m->jumpToRandomPos();}
//         };

//     bool isSuccess =false;
//     int counter =0;
//     for (  ;counter< numberOfRandomStartPositions; counter++) {
//         LOG_DEBUG("***** TEST " + std::to_string(counter));

//         isSuccess =false;        
//         jumpToRandomPosition(_simMotors);
//         std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//         SendAllWingsToOpen();        
//         auto start = std::chrono::system_clock::now();
//         while(true) {  
//             if( AreAllWingsOpen()) {
//                 isSuccess=true;
//                 LOG_DEBUG("****  SUCCESS  ****");
//                 break;
//             }            
//             std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//             auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//             if ( diff > maxSecondsForTest*1000) {
//                 LOG_DEBUG("****  TIME OUT  ****");
//                 break;
//             }
//         }                
//         EXPECT_TRUE(isSuccess) << "No failure";  
//     }   
//     EXPECT_EQ(counter,numberOfRandomStartPositions) << "No exceptions while opening should happen so all runs should be done";

// }

// // start at open position and try to close
// TEST_F(CornerScenarioTest, FullyOpenToClose) {
//     Log::Init();
//     bool isSuccess =false;

//     for( auto &m: _simMotors) {m->commandPositionPerc(100);}
//     std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//     SendAllWingsToClose();        
//     auto start = std::chrono::system_clock::now();
//     while(true) {  
//         if(  AreAllWingsClosed()) {
//             isSuccess=true;
//             LOG_DEBUG("****  SUCCESS  ****");
//             break;
//         }
        
//         std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//         auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//         if ( diff > maxSecondsForTest*1000) {
//             LOG_DEBUG("****  TIME OUT  ****");
//             break;
//         }
//     }                
//     EXPECT_TRUE(isSuccess) << "All windows should be closed eventually";  
// }

// // start at closed position and try to open
// TEST_F(CornerScenarioTest, FullyClosedToOpen) {
//     Log::Init();
//     bool isSuccess =false;

//     for( auto &m: _simMotors) {m->commandPositionPerc(100);}
//     std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//     SendAllWingsToOpen();        
//     auto start = std::chrono::system_clock::now();
//     while(true) {  
//         if( AreAllWingsOpen()) {
//             isSuccess=true;
//             LOG_DEBUG("****  SUCCESS  ****");
//             break;
//         }
        
//         std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//         auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//         if ( diff > maxSecondsForTest*1000) {
//             LOG_DEBUG("****  TIME OUT  ****");
//             break;
//         }
//     }                
//     EXPECT_TRUE(isSuccess) << "All windows should be open eventually";  
// }


// // start at ione wing closed ( female) and the other wing open and try to close them all
// TEST_F(CornerScenarioTest, FemaleClosedMaleOpenToAllClosed) {
//     Log::Init();
//     bool isSuccess =false;

//     _simMotors[0]->commandPositionPerc(100);
//     _simMotors[1]->commandPositionPerc(0);

//     std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//     SendAllWingsToClose();        
//     auto start = std::chrono::system_clock::now();
//     while(true) {  
//         if( AreAllWingsClosed()) {
//             isSuccess=true;
//             LOG_DEBUG("****  SUCCESS  ****");
//             break;
//         }
        
//         std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//         auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//         if ( diff > maxSecondsForTest*1000) {
//             LOG_DEBUG("****  TIME OUT  ****");
//             break;
//         }
//     }                
//     EXPECT_TRUE(isSuccess) << "All windows should be closed eventually";  
// }


// // start at ione wing closed ( male) and the other wing open and try to close them all
// TEST_F(CornerScenarioTest, MaleClosedFemaleOpenToAllClosed) {
//     Log::Init();
//     bool isSuccess =false;

//     _simMotors[0]->commandPositionPerc(0);
//     _simMotors[1]->commandPositionPerc(100);

//     std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//     SendAllWingsToClose();        
//     auto start = std::chrono::system_clock::now();
//     while(true) {  
//         if( AreAllWingsClosed()) {
//             isSuccess=true;
//             LOG_DEBUG("****  SUCCESS  ****");
//             break;
//         }
        
//         std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//         auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//         if ( diff > maxSecondsForTest*1000) {
//             LOG_DEBUG("****  TIME OUT  ****");
//             break;
//         }
//     }                
//     EXPECT_TRUE(isSuccess) << "All windows should be closed eventually";  
// }

// !! todo !! at feedback of position

// // start at ione wing closed ( male) and the other wing open and try to close them all
// TEST_F(CornerScenarioTest, goToRandomPosition) {
//     Log::Init();
    
//     int randomPosWing0 = 0;
//     int randomPosWing1 = 0;
    
//     auto jumpToRandomPosition = [](std::vector<std::shared_ptr<MqttMotorSim>> & motors)  { 
//                for( auto &m: motors) {m->jumpToRandomPos();}
//         };
//     auto setRandomTargetPositions = [& randomPosWing0, & randomPosWing1 ](std::vector<std::shared_ptr<IWing>> & wings) {
//         wings[0]->setPositionPerc(randomPosWing0);
//         wings[1]->setPositionPerc(randomPosWing1);
//     };
//     auto areWingsAtPosition = [& randomPosWing0, & randomPosWing1 ](std::vector<std::shared_ptr<IWing>> & wings) {
//         // todo check how to get the actual position for comparisment
//         wings[0]->getPosition();
//         wings[1]->getPosition();
//     };

//     bool isSuccess =false;
//     int counter =0;
//     for (  ;counter< numberOfRandomStartPositions; counter++) {
//         LOG_DEBUG("***** TEST " + std::to_string(counter));

//         isSuccess =false;        
//         jumpToRandomPosition(_simMotors);
//         std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // give time to sim motors to jump  
//         SendAllWingsToOpen();        
//         auto start = std::chrono::system_clock::now();
//         while(true) {  
//             if( AreAllWingsOpen()) {
//                 isSuccess=true;
//                 LOG_DEBUG("****  SUCCESS  ****");
//                 break;
//             }            
//             std::this_thread::sleep_for(std::chrono::milliseconds(200));  
//             auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
//             if ( diff > maxSecondsForTest*1000) {
//                 LOG_DEBUG("****  TIME OUT  ****");
//                 break;
//             }
//         }                
//         EXPECT_TRUE(isSuccess) << "No failure";  
//     }   
//     EXPECT_EQ(counter,numberOfRandomStartPositions) << "No exceptions while opening should happen so all runs should be done";
// }