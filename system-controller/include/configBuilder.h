#ifndef CONFIGBUILDER_H
#define CONFIGBUILDER_H

#include "pch.h"

#include "movingWindow.h"
#include "passiveWindow.h"
#include "masterMotorizedWindow.h"
#include "motorizedWindow.h"
#include "wingStatusPublisher.h"
#include "mqttMotor.h"
#include "wing.h"

enum ConfigObjectType {
    fixed,
    motorized,
    passive,
    inversStart,
    inversEnd,
    corner,
    middle
};

class ConfigObject {
    public:
        ConfigObject(std::string type, int length, std::string pn, std::string serial);
        ConfigObject(std::string type);
        ConfigObject(std::string type, int length);
        std::string type;
        int length;
        std::string pn;
        std::string serial;
        ConfigObjectType getParseType() { return _parsedType;}
        static ConfigObjectType parseType(std::string type);
    private:
        ConfigObjectType _parsedType;
};


class ConfigBuilder {
    public:                
        static void parseFromJson(std::string json, std::vector<std::shared_ptr<IWing>> & wings, std::string & configId);
    protected:
        static void parseJsonToObjects(std::string json, std::vector<ConfigObject> & configObjects, std::string & configId);
        static std::shared_ptr<IWing> parseWing(std::vector<ConfigObject>::iterator & startOfWing,std::vector<ConfigObject>::iterator & endOfWing, bool isLefOpening,const std::string &configId) ;
        static void listWings(std::vector<ConfigObject> & configObjects, std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> & wingInfos);
        static void connectWingsWithRelationsAndList(std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> & wingInfos, std::vector<std::shared_ptr<IWing>> & wings ,const std::string &configId) ;
    
    private:
        static void parseWing();
};

#endif //CONFIGBUILDER_H