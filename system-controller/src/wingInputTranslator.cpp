#include "wingInputTranslator.h"
#include "log.h"


void WingInputTranslator::translateInputToAction(const MotorStatusData &currStatus,
                                                 int currentPos,
                                                bool waslastMovementOpening,
                                                 WingComandType cmdType,
                                                 std::function<void(void)> openAction,
                                                 std::function<void(void)> stopAction,
                                                 std::function<void(void)> closeAction,
                                                 std::function<void(void)> calibrateAction,
                                                 std::function<void(void)> clearCalibrationAction,
                                                 std::function<void(void)> lockAction,
                                                 std::function<void(void)> setPositionAction,
                                                 std::function<void(void)> commandNotFoundAction) const
{

    switch (cmdType) {
        case WingComandType::Stop : LOG_INFO("Input 'Stop' received"); break;
        case WingComandType::Open : LOG_INFO("Input 'Open' received"); break;
        case WingComandType::OpenOrStop : LOG_INFO("Input 'OpenOrStop' received"); break;
        case WingComandType::Close : LOG_INFO("Input 'Close' received"); break;
        case WingComandType::CloseOrStop : LOG_INFO("Input 'CloseOrStop' received"); break;    
        case WingComandType::Calibrate : LOG_INFO("Input 'Calibrate' received"); break;
        case WingComandType::Cancel : LOG_INFO("Input 'Cancel' received"); break;
        case WingComandType::Lock : LOG_INFO("Input 'Lock' received"); break;
        case WingComandType::SetPosition : LOG_INFO("Input 'SetPosition' received"); break;
        case WingComandType::Pulse : LOG_INFO("Input 'Pulse' received"); break;          
        case WingComandType::PulseOrStop : LOG_INFO("Input 'PulseOrStop' received"); break;
        case WingComandType::Ignore : LOG_INFO("Input 'Ignore' received"); break;    
    }

    switch (cmdType)
    {

    case WingComandType::Close:
    case WingComandType::CloseOrStop:
        if (cmdType == WingComandType::CloseOrStop && !currStatus.isMotorStopped()) {
            stopAction();
        } else {
            closeAction();
        }
        break;

    case WingComandType::Open:
    case WingComandType::OpenOrStop:
        if (cmdType == WingComandType::OpenOrStop && !currStatus.isMotorStopped()) {
            stopAction();
        } else {
            openAction();
        }
        break;

    case WingComandType::Stop:
        stopAction();
        break;

    case WingComandType::Pulse:
    case WingComandType::PulseOrStop:
        if (cmdType == WingComandType::PulseOrStop && !currStatus.isMotorStopped())        {
            stopAction();
        }
        else {
            if (currStatus.isClosed){
                openAction();
            } else if (currStatus.isOpen){
                closeAction();
            } else {
                if (waslastMovementOpening) { 
                    closeAction();
                }
                else { // so also open when spot on target !!
                    openAction();
                }
            }
        }
        break;

    case WingComandType::Calibrate:
        calibrateAction();
        break;
    case WingComandType::Cancel:
        clearCalibrationAction();
        break;

    case WingComandType::Lock:
        lockAction();
        break;
    case WingComandType::SetPosition:
        setPositionAction();
        break;

    default:
        commandNotFoundAction();
        break;
    }
};
