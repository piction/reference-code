#ifndef H_WINGINPUTTRANSLATOR
#define H_WINGINPUTTRANSLATOR

#include "wingData.h"
#include "motorData.h"
#include "mqttData.h"   
#include "pch.h"


// This class will do a translation of the raw input to a real 
// wing command. The translation is based on the current state of the wing
// and the requested input

// the purpose of injecting this in the wingsHandler is to be able to test this 

class IWingInputTranslator {
    public:
        virtual ~IWingInputTranslator(){}
        virtual void translateInputToAction(const MotorStatusData & currStatus, 
                                    int currentPos, 
                                    bool waslastMovementOpening,
                                    WingComandType cmdType,
                                    std::function<void(void)>openAction,
                                    std::function<void(void)>stopAction,
                                    std::function<void(void)>closeAction,                                    
                                    std::function<void(void)>calibrateAction,
                                    std::function<void(void)>clearCalibrationAction,
                                    std::function<void(void)>lockAction,
                                    std::function<void(void)>setPositionAction,
                                    std::function<void(void)>commandNotFoundAction) const =0;
};
class WingInputTranslator : public IWingInputTranslator{
    public:
        WingInputTranslator(){}
        void translateInputToAction(const MotorStatusData & currStatus, 
                                    int currentPos, 
                                    bool waslastMovementOpening,
                                    WingComandType cmdType,
                                    std::function<void(void)>openAction,
                                    std::function<void(void)>stopAction,
                                    std::function<void(void)>closeAction,   
                                    std::function<void(void)>calibrateAction,
                                    std::function<void(void)>clearCalibrationAction,
                                    std::function<void(void)>lockAction,
                                    std::function<void(void)>setPositionAction,
                                    std::function<void(void)>commandNotFoundAction) const override ;
};



#endif