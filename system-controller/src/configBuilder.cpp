#include "configBuilder.h"
#include "log.h"



ConfigObject::ConfigObject(std::string type, int length, std::string pn, std::string serial)
: type(type)
, length(length)
, pn(pn)
, serial(serial) {
    _parsedType = parseType(type);
}
ConfigObject::ConfigObject(std::string type, int length)
: ConfigObject(type,length,"","") {
}
ConfigObject::ConfigObject(std::string type) 
: ConfigObject(type,0) {
}

ConfigObjectType ConfigObject::parseType(std::string type) {
    if ( type.compare("Q") ==0 || type.compare("q") ==0 ) {
        return ConfigObjectType::fixed;
    } else if ( type.compare("X") ==0 || type.compare("x") ==0 ) {
        return ConfigObjectType::motorized;
    } else if ( type.compare("O") ==0 || type.compare("o") ==0 ) {
        return ConfigObjectType::passive;
    } else if ( type.compare("V") ==0 || type.compare("v") ==0 ) {
        return ConfigObjectType::corner;
    } else if ( type.compare("-") ==0) {
       return ConfigObjectType::middle;
    } else if ( type.compare(")") ==0) {
        return ConfigObjectType::inversEnd;
    } else if ( type.compare("(") ==0) {
        return ConfigObjectType::inversStart;
    }else {
        
        LOG_CRITICAL_THROW("Failed to parse configObject type ->" + type);
    }
}



// Example JSON -> see project folder (example-config-json)


void ConfigBuilder::parseJsonToObjects(std::string json, std::vector<ConfigObject> & configObjects , std::string & id) {

    rapidjson::Document doc;
    doc.Parse(json.data());
    if (doc.HasParseError()) {
        LOG_CRITICAL_THROW("invalid JSON found in config");        
    }
    if (doc.IsObject() == false) {
        LOG_CRITICAL_THROW("unexpected JSON format, expected object as root: ");
    }
    try {    
        rapidjson::Value& foundId = doc["id"];
        id = foundId.GetString();
    if ( !foundId.IsString()) {
            LOG_CRITICAL_THROW("Failed to parse configuration id of JSON configuration");
    }
    }catch(...) {
         LOG_CRITICAL_THROW("Failed to parse configuration id of JSON configuration");
    }
    
    rapidjson::Value& c = doc["config"];
    if ( !c.IsArray()) {
        LOG_CRITICAL_THROW("Failed to parse a configuration due missing array");
    }
    std::vector<std::shared_ptr<ConfigObject>> config;
    for (int i =0; i<c.Size() ; i++ ){
        auto & e = c[i];
        rapidjson::Value& t = e["type"];
        if ( !t.IsString()) {
            LOG_CRITICAL_THROW("Failed to parse type of element in configuration");
        }
        std::string elType =  t.GetString();
        ConfigObjectType parsedType = ConfigObject::parseType(elType);
        if ( parsedType == ConfigObjectType::fixed) { 
            configObjects.push_back(ConfigObject("Q"));
        } else if ( parsedType == ConfigObjectType::passive ) { 
            rapidjson::Value &len = e["length"];
            if ( !len.IsNumber()) {
                LOG_CRITICAL_THROW("Failed to parse length of passive element");
            }
            configObjects.push_back(ConfigObject("O",len.GetInt()) );
        } else if ( parsedType == ConfigObjectType::motorized ) { 
            rapidjson::Value& len = e["length"];
            if ( !len.IsNumber()) {
                LOG_CRITICAL_THROW("Failed to parse length of passive element");
            }            
            rapidjson::Value& serial = e["serial"];
            if ( !serial.IsString()) {
                LOG_CRITICAL_THROW("Failed to parse serial of motor element");
            }
            rapidjson::Value& pn = e["pn"];
            if ( !pn.IsString()) {
                LOG_CRITICAL_THROW("Failed to parse productnumber (pn) of motor element");
            }
            configObjects.push_back(ConfigObject("X",len.GetInt(), pn.GetString(), serial.GetString()) );

        } else {
          configObjects.push_back(ConfigObject(elType));
        }
    }
}

// lists wings as a tuple of startitterator, enditterator and isLeftOpening
void ConfigBuilder::listWings(std::vector<ConfigObject> & configObjects, std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> & wingInfos){
    bool lastWingHasOpeningLeft = true;
    bool isFirstWing = true;
    auto getIsLeftOpeningForCurrentWingConfig = [& lastWingHasOpeningLeft, & isFirstWing](ConfigObject & firstElementOfCurrentConfig) ->bool{
        if( isFirstWing) {
            isFirstWing = false;
            // checking the FIRST wing ...
            // when there is no Q the first wing is has the opening always on the left side beceause this implies that
            // there is at least ont wing inverted ( ex: (X)X-XQ, X(X), ...)
            // when there is a Q, it should be the first letter because at the end it is also the opening at the left side
            lastWingHasOpeningLeft = firstElementOfCurrentConfig.getParseType() != ConfigObjectType::fixed;
        } else {
            lastWingHasOpeningLeft = !lastWingHasOpeningLeft;
        }
        return lastWingHasOpeningLeft;
    };
        
    std::vector<ConfigObject>::iterator startOfCurrentWing =  configObjects.begin();    
    for (auto i = configObjects.begin(); i != configObjects.end(); ++i) 
    { 
        if ( startOfCurrentWing >= configObjects.end()) {
            LOG_CRITICAL_THROW("Programming error ! => start pointer can not be over or same as end of source");
        }

        if( i->getParseType() == ConfigObjectType::inversStart) {
            if ( std::distance(startOfCurrentWing, i) > 0) {
                // parse wing before inverse started
                wingInfos.push_back(std::make_tuple(startOfCurrentWing,i,getIsLeftOpeningForCurrentWingConfig(*startOfCurrentWing))); 
            } 
            startOfCurrentWing = i ; 
        } else if (i->getParseType() == ConfigObjectType::inversEnd || 
                    i->getParseType() == ConfigObjectType::corner || 
                    i->getParseType() == ConfigObjectType::middle) {
            if ( std::distance(startOfCurrentWing,i+1)==0) {
                LOG_CRITICAL_THROW("Programming error ! (or nothing between brackets of config)");
            }   
            auto endOfWing = i->getParseType() == ConfigObjectType::inversEnd ? i+1 : i;
            wingInfos.push_back(std::make_tuple(startOfCurrentWing,endOfWing,getIsLeftOpeningForCurrentWingConfig(*startOfCurrentWing)));
            startOfCurrentWing  = i->getParseType() == ConfigObjectType::inversEnd ? endOfWing : endOfWing+1; 
            // when inverse closing is followed by a corner or middle we need to skip one extra 
            if ( i != configObjects.end() && 
                    ((i+1)->getParseType() == ConfigObjectType::corner || (i+1)->getParseType() == ConfigObjectType::middle)) {
                startOfCurrentWing++; i++;
            }
        }   
    } 
 
    if ( std::distance(startOfCurrentWing,  configObjects.end()) > 0 ) {
        wingInfos.push_back(std::make_tuple(startOfCurrentWing, configObjects.end(),getIsLeftOpeningForCurrentWingConfig(*startOfCurrentWing)));
    }
}

std::shared_ptr<IWing> ConfigBuilder::parseWing(std::vector<ConfigObject>::iterator & startOfWing,std::vector<ConfigObject>::iterator & endOfWing, bool hasOpeningLeft , const std::string &configId)  {

    std::shared_ptr<IMovingWindow> lastElement;
    std::shared_ptr<IMovingWindow> currElement;
    std::shared_ptr<MasterMotorizedWindow> master;
    std::shared_ptr<IWing> wing;
   
    ConfigObjectType lastElementType;
    // check if wing contains Q
    if( startOfWing->getParseType() == ConfigObjectType::fixed || (endOfWing-1)->getParseType() == ConfigObjectType::fixed ) {

        //make sure that there is no passive element on the end or beginning of the wing
        if( startOfWing->getParseType() == ConfigObjectType::passive || (endOfWing-1)->getParseType() == ConfigObjectType::passive ) {
            LOG_CRITICAL_THROW("A passive element can not sit at the edge of a wing!");
        }
    }
    bool isInversedWing = startOfWing->getParseType()== ConfigObjectType::inversStart;
    auto start = isInversedWing ? startOfWing+1 : startOfWing;
    auto end = isInversedWing ? endOfWing-1 : endOfWing;
    for (auto i = start; i != end; ++i) {    

        if ( i->getParseType() == ConfigObjectType::fixed) {
            // fixed windows are not important, skip this and don't update lastElement
            continue;
        }
        if (i->getParseType() == ConfigObjectType::passive) {
            currElement = std::make_shared<PassiveWindow>(i->length);
        }
        if ( i->getParseType() == ConfigObjectType::motorized) {
            if ( hasOpeningLeft && i==start ||  !hasOpeningLeft && i==(end-1) ) { 
                currElement = std::make_shared<MasterMotorizedWindow>(i->length,std::make_shared<MqttMotor>(i->pn,i->serial));
                master = std::dynamic_pointer_cast<MasterMotorizedWindow>(currElement);               
                wing =std::make_shared<Wing>(master, std::make_shared<WingRelationManager>(),std::make_shared<WingStatusPublisher>(configId), hasOpeningLeft);  
                        
            } else {
                currElement = std::make_shared<MotorizedWindow>(i->length,std::make_shared<MqttMotor>(i->pn,i->serial));
            }
        }        
        // handle slave topology
        if( lastElement) {
            if ( !hasOpeningLeft) {
                currElement->addSlave(lastElement, lastElementType==ConfigObjectType::motorized ? SlaveType::Motor : SlaveType::Passive );
            } else {
                lastElement->addSlave(currElement, i->getParseType()==ConfigObjectType::motorized ? SlaveType::Motor : SlaveType::Passive );
            }
        }
        lastElement = currElement;
        lastElementType = i->getParseType();
    }   
    if ( wing ) {
        return wing;
    } else {
        LOG_CRITICAL_THROW("No wing is created! Failed to parse and return a wing");
    }
}



void ConfigBuilder::connectWingsWithRelationsAndList(std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> & wingInfos, std::vector<std::shared_ptr<IWing>> & wings, const std::string &configId) {

    for ( int j = 0; j< wingInfos.size() ; j++) {
        wings.push_back(parseWing(std::get<0>(wingInfos[j]),std::get<1>(wingInfos[j]),std::get<2>(wingInfos[j]), configId));
        // when more that one wing there is a relation
        if ( j >= 1 ) {
            if ((std::get<0>(wingInfos[j]) - 1)-> getParseType() == ConfigObjectType::corner) {
                wings[wings.size()-2]->addSibling(wings[wings.size() - 1], WingSiblingType::CornerFemale); // first wing in the configuration will be the MALE and having a female sibling
                wings[wings.size()-1]->addSibling(wings[wings.size() - 2], WingSiblingType::CornerMale);    // second wing in the configuration will be the FEMALE wing and having a male sibling
            }
            if ((std::get<0>(wingInfos[j]) - 1)-> getParseType() == ConfigObjectType::middle) {
                wings[wings.size()-2]->addSibling(wings[wings.size() - 1], WingSiblingType::MiddleFemale); // first wing in the configuration will be the MALE and having a female sibling
                wings[wings.size()-1]->addSibling(wings[wings.size() - 2], WingSiblingType::MiddleMale); // second wing in the configuration will be the FEMALE wing and having a male sibling
            }
            if ((std::get<0>(wingInfos[j]) - 1)-> getParseType() == ConfigObjectType::inversEnd ||
                (std::get<0>(wingInfos[j]) )-> getParseType() == ConfigObjectType::inversStart) {
                wings[wings.size()-2]->addSibling(wings[wings.size() - 1], WingSiblingType::Opposite);
                wings[wings.size()-1]->addSibling(wings[wings.size() - 2], WingSiblingType::Opposite);
            }
        }
    }
}


void ConfigBuilder::parseFromJson(std::string json, std::vector<std::shared_ptr<IWing>> & wings, std::string & configId) {
    std::vector<ConfigObject> configObjects;
    parseJsonToObjects(json,configObjects, configId);
    std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> wingInfos;
    listWings(configObjects,wingInfos);
    connectWingsWithRelationsAndList(wingInfos,wings, configId);
}