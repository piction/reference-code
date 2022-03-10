#ifndef SYSTEMSETTINGSPARSER_H
#define SYSTEMSETTINGSPARSER_H

#include "pch.h"
#include "systemSettings.h"

class SystemSettingsParser {
public:
static void parseJsonToSettings(std::string & json) {

    rapidjson::Document doc;
    doc.Parse(json.data());
    if (doc.HasParseError()) {
        LOG_CRITICAL_THROW("invalid JSON found in config");        
    }
    if (doc.IsObject() == false) {
        LOG_CRITICAL_THROW("unexpected JSON format, expected object as root: ");
    }

    if ( ! doc.HasMember("systemsettings")) {
        LOG_ERROR("Now systemsettings found to parse");
        return; // no system settings to change
    }
    try {
        rapidjson::Value& systemSettingsVal = doc["systemsettings"];

        if ( systemSettingsVal.HasMember("chicanoverlap") && systemSettingsVal["chicanoverlap"].IsInt()) {
            SystemSettings::getInstance().setChicanOverlap(systemSettingsVal["chicanoverlap"].GetInt());
        }
        if ( systemSettingsVal.HasMember("chicanzone") && systemSettingsVal["chicanzone"].IsInt()) {
            SystemSettings::getInstance().setChicanZone(systemSettingsVal["chicanzone"].GetInt());
        }
        if ( systemSettingsVal.HasMember("slowdowndist") && systemSettingsVal["slowdowndist"].IsInt()) {
            SystemSettings::getInstance().setSlowdownDist(systemSettingsVal["slowdowndist"].GetInt());
        }
        if ( systemSettingsVal.HasMember("cornerzone") && systemSettingsVal["cornerzone"].IsInt()) {
            SystemSettings::getInstance().setCornerZone(systemSettingsVal["cornerzone"].GetInt());
        }
        if ( systemSettingsVal.HasMember("oppositezone") && systemSettingsVal["oppositezone"].IsInt()) {
            SystemSettings::getInstance().setOppositeZone(systemSettingsVal["oppositezone"].GetInt());
        }
        if ( systemSettingsVal.HasMember("triggerpushwingdistance") && systemSettingsVal["triggerpushwingdistance"].IsInt()) {
            SystemSettings::getInstance().setTriggerPushWingDistance(systemSettingsVal["triggerpushwingdistance"].GetInt());        
        }

   
    }catch(...) {
         LOG_CRITICAL_THROW("Failed to parse system settings of JSON configuration");
    }

   
    }
};

#endif