#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "log.h"
#include "wingInputTranslator.h"

TEST(WingInputTranslator,basics ){
    Log::Init();

    WingInputTranslator sut;

    int openActionCntr = 0, closeActionCntr=0, stopActionCntr=0, calibrateActionCntr=0, clearActionCntr = 0, lockActionCntr=0, positionActionCntr=0, notFoundActionCntr=0;

    auto runInputTranslation = [&](const MotorStatusData & currStatus, 
                                    int currentPos,
                                    bool waslastMovementOpening,
                                    WingComandType cmdType){
                                    // reset counters;
                                    openActionCntr = 0; closeActionCntr=0; stopActionCntr=0; calibrateActionCntr=0; clearActionCntr = 0; lockActionCntr=0; positionActionCntr=0; notFoundActionCntr=0;
                                    sut.translateInputToAction(currStatus, currentPos,waslastMovementOpening, cmdType,
                                                                    [&](){openActionCntr++;},
                                                                    [&](){stopActionCntr++;},
                                                                    [&](){closeActionCntr++;},
                                                                    [&](){calibrateActionCntr++;},
                                                                    [&](){clearActionCntr++;},
                                                                    [&](){lockActionCntr++;},
                                                                    [&](){positionActionCntr++;},
                                                                    [&](){notFoundActionCntr++;});
                                    };

    MotorStatusData data;
    auto actFromStandstill = [&](bool lastMovementWasOpen) {
         runInputTranslation(data,0,lastMovementWasOpen,WingComandType::Close);
        ASSERT_EQ(1,closeActionCntr) << "close should be called once";

        runInputTranslation(data,0,lastMovementWasOpen,WingComandType::Open);
        ASSERT_EQ(1,openActionCntr) << "open should be called once";

        runInputTranslation(data,0,lastMovementWasOpen,WingComandType::Stop);
        ASSERT_EQ(1,stopActionCntr) << "stop should be called once";

        runInputTranslation(data,0,lastMovementWasOpen,WingComandType::OpenOrStop);
        ASSERT_EQ(1,openActionCntr) << "open should be called once because not moving";

        runInputTranslation(data,0,lastMovementWasOpen,WingComandType::CloseOrStop);
        ASSERT_EQ(1,closeActionCntr) << "close should be called once because not moving";
    };
    actFromStandstill(true);
    actFromStandstill(false);
   


    data.speedMm = 20;
    data.isOpen = false;
    data.isClosed = false;
    data.isLocked = false;
    
    runInputTranslation(data,0,true,WingComandType::OpenOrStop);
    ASSERT_EQ(1,stopActionCntr) << "stop should be called once for OpenOrStop because moving";

    runInputTranslation(data,0,false,WingComandType::CloseOrStop);
    ASSERT_EQ(1,stopActionCntr) << "stop should be called once for CloseOrStop because moving";

    runInputTranslation(data,0,true,WingComandType::SetPosition);
    ASSERT_EQ(1,positionActionCntr) << "positionAction should be called once";

    runInputTranslation(data,0,false,WingComandType::SetPosition);
    ASSERT_EQ(1,positionActionCntr) << "positionAction should be called once";


    data.speedMm = 0;
    data.isOpen = true; data.isClosed=false;
    runInputTranslation(data,0,true,WingComandType::Pulse);
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on pulse when IsOpen";
    runInputTranslation(data,0,false,WingComandType::Pulse);
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on pulse when IsOpen";

    data.isOpen = false; data.isClosed=true;
    runInputTranslation(data,0,false,WingComandType::Pulse);
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on pulse when IsClosed";
    runInputTranslation(data,0,true,WingComandType::Pulse);
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on pulse when IsClosed";

    data.isOpen = false; data.isClosed=false;
    runInputTranslation(data,20,false,WingComandType::Pulse); // currently the wing is closing from pos : 20 ->  target : 10 
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on pulse when is closing";

    runInputTranslation(data,10,true,WingComandType::Pulse); // currently the wing is opening from pos : 10 ->  target : 30 
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on pulse when is closing";

    
    // check pulseOrStop on moving window
    data.isOpen = false; data.isClosed=false;  data.speedMm = 10;
    runInputTranslation(data,20,true,WingComandType::PulseOrStop); // currently the wing is closing from pos : 20 ->  target : 10 
    ASSERT_EQ(1,stopActionCntr) << "stop action should be called once on pulseOrStop when is closing";
    runInputTranslation(data,20,false,WingComandType::PulseOrStop); // currently the wing is closing from pos : 20 ->  target : 10 
    ASSERT_EQ(1,stopActionCntr) << "stop action should be called once on pulseOrStop when is closing";

    // check pulseOrStop on not moving window
    data.speedMm = 0;
    data.isOpen = true; data.isClosed=false;
    runInputTranslation(data,0,true,WingComandType::PulseOrStop);
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on PulseOrStop when IsOpen";
    runInputTranslation(data,0,false,WingComandType::PulseOrStop);
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on PulseOrStop when IsOpen";

    data.isOpen = false; data.isClosed=true;
    runInputTranslation(data,0,true,WingComandType::PulseOrStop);
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on PulseOrStop when IsClosed";
    runInputTranslation(data,0,false,WingComandType::PulseOrStop);
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on PulseOrStop when IsClosed";

    data.isOpen = false; data.isClosed=false;
    runInputTranslation(data,20,false,WingComandType::PulseOrStop); // currently the wing is closing from pos : 20 ->  target : 10 
    ASSERT_EQ(1,openActionCntr) << "open action should be called once on PulseOrStop when is closing";

    runInputTranslation(data,10,true,WingComandType::PulseOrStop); // currently the wing is opening from pos : 10 ->  target : 30 
    ASSERT_EQ(1,closeActionCntr) << "close action should be called once on PulseOrStop when is closing";
    
};