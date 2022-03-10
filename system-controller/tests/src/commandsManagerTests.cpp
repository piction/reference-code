#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include "log.h"

#include "commandsManager.h"

TEST(commandsManager,basics ){
    Log::Init();
    
    CommandsManager sut;
    int sendCounter=0;
    sut.setSendHandler([& sendCounter](MqttData d) {
        sendCounter++;
    });
    sut.startEvaluating();
    MqttData data("rbus/0628252/0000000000001/rbus.set.speed/trigger",35,"id" );
    sut.pushCommandoToBeSend(data,CommandType::SetParam);
    EXPECT_EQ(sut.getNumberOfManagedCommands(),1) << "Should be pushed to the buffer";
    EXPECT_EQ(sendCounter,1) <<"One message should be send out";

    
    MqttData data2("rbus/0628252/0000000000001/rbus.set.position/trigger",99,"id" );
    sut.pushCommandoToBeSend(data2,CommandType::SetParam);
    EXPECT_EQ(sut.getNumberOfManagedCommands(),2) << "second data should be pushed to the buffer";
    EXPECT_EQ(sendCounter,2) <<"Two message should be send out";

    // only change parameter
    MqttData data3("rbus/0628252/0000000000001/rbus.set.speed/trigger",120,"id" );
    sut.pushCommandoToBeSend(data3,CommandType::SetParam);
    EXPECT_EQ(sut.getNumberOfManagedCommands(),2) << "Third data should replace and not add one to the buffer";
    EXPECT_EQ(sendCounter,3) <<"Three message should be send out";

    sut.stopEvaluating();
    
    EXPECT_TRUE(true);
}

TEST(commandsManager,evaluator ){
    Log::Init();
    CommandsManager sut;
    int sendCounter=0;
    sut.setSendHandler([& sendCounter](MqttData d) {
        sendCounter++;
    });
    sut.startEvaluating();
    MqttData data("rbus/0628252/0000000000001/rbus.set.speed/trigger",35,"id" );
    sut.pushCommandoToBeSend(data,CommandType::SetParam);
    MqttData dataMove("rbus/0628252/0000000000001/rbus.close/trigger","id" );
    sut.pushCommandoToBeSend(dataMove,CommandType::SetMovement);

    EXPECT_EQ(sut.getNumberOfManagedCommands(),1) << "Should be pushed to the buffer";
    EXPECT_EQ(sendCounter,2) <<"Two messages should be send out (data,dataMove)";
    std::this_thread::sleep_for(std::chrono::milliseconds(1300));
    EXPECT_GE(sendCounter,4) <<"The messages where never acked and should be resend";
    sut.stopEvaluating();    
    EXPECT_TRUE(true);
}

TEST(commandsManager,handleAck ){
    Log::Init();
    
    CommandsManager sut;
    int sendCounter=0;
    sut.setSendHandler([& sendCounter](MqttData d) {
        sendCounter++;
    });
    sut.startEvaluating();
     MqttData data("rbus/0628252/0000000000001/rbus.set.speed/trigger",35,"id" );
    sut.pushCommandoToBeSend(data,CommandType::SetParam);
    MqttData dataMove("rbus/0628252/0000000000001/rbus.close/trigger","id" );
    sut.pushCommandoToBeSend(dataMove,CommandType::SetMovement);

    
    EXPECT_EQ(sendCounter,2) <<"Two messages should be send out (data,dataMove)";
    sut.handleAck(data);
    sut.handleAck(dataMove);
    std::this_thread::sleep_for(std::chrono::milliseconds(1300));
    
    EXPECT_EQ(sendCounter,2) <<"The message should not be resend because it was acked";

    sut.stopEvaluating();    
    EXPECT_TRUE(true);
}

