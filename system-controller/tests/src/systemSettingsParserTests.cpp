#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <functional>
#include <sstream>
#include "log.h"
#include "utils.h"

#include "systemSettingsParser.h"
using namespace std;

TEST(systemSettingsParser,canParseNoSettings ){
    Log::Init();
    ifstream f(utils::getApplicationDirectory() + "/testData/QOX-XXQ_test.json"); //taking file as inputstream
        
    EXPECT_TRUE(f) << "failed to get the file stream";
    ostringstream ss;
    ss << f.rdbuf(); // reading data
    
    int refChicanoverlap = SystemSettings::getInstance().getChicanOverlap();
    int refChicanzone = SystemSettings::getInstance().getChicanZone();
    int refSlowdowndist = SystemSettings::getInstance().getSlowdownDist();
    int refOppositeZone = SystemSettings::getInstance().getOppositeZone();
    int refCornerZone = SystemSettings::getInstance().getCornerZone();
    int refTriggerPushWingDistance = SystemSettings::getInstance().getTriggerPushWingDistance();
    
    auto json = ss.str();
    SystemSettingsParser::parseJsonToSettings(json);

    EXPECT_EQ(refChicanoverlap, SystemSettings::getInstance().getChicanOverlap()) << "No setting should be alterd, because nothing should be read from the file" ;
    EXPECT_EQ(refChicanzone, SystemSettings::getInstance().getChicanZone()) << "No setting should be alterd, because nothing should be read from the file" ;
    EXPECT_EQ(refSlowdowndist, SystemSettings::getInstance().getSlowdownDist()) << "No setting should be alterd, because nothing should be read from the file" ;
    EXPECT_EQ(refOppositeZone, SystemSettings::getInstance().getOppositeZone()) << "No setting should be alterd, because nothing should be read from the file" ;
    EXPECT_EQ(refCornerZone, SystemSettings::getInstance().getCornerZone()) << "No setting should be alterd, because nothing should be read from the file" ;
    EXPECT_EQ(refTriggerPushWingDistance, SystemSettings::getInstance().getTriggerPushWingDistance()) << "No setting should be alterd, because nothing should be read from the file" ;

}



TEST(systemSettingsParser,canParseAllSettings ){
    Log::Init();
    ifstream f(utils::getApplicationDirectory() + "/testData/settings_test.json"); //taking file as inputstream
        
    EXPECT_TRUE(f) << "failed to get the file stream";
    ostringstream ss;
    ss << f.rdbuf(); // reading data
    
    int refChicanoverlap = SystemSettings::getInstance().getChicanOverlap();
    int refChicanzone = SystemSettings::getInstance().getChicanZone();
    int refSlowdowndist = SystemSettings::getInstance().getSlowdownDist();
    int refOppositeZone = SystemSettings::getInstance().getOppositeZone();
    int refCornerZone = SystemSettings::getInstance().getCornerZone();
    int refTriggerPushWingDistance = SystemSettings::getInstance().getTriggerPushWingDistance();
    
    auto json = ss.str();
    SystemSettingsParser::parseJsonToSettings(json);

    EXPECT_NE(refChicanoverlap, SystemSettings::getInstance().getChicanOverlap()) << "Setting should be alterd from file" ;
    EXPECT_NE(refChicanzone, SystemSettings::getInstance().getChicanZone()) << "Setting should be alterd from file" ;
    EXPECT_NE(refSlowdowndist, SystemSettings::getInstance().getSlowdownDist()) << "Setting should be alterd from file" ;
    EXPECT_NE(refOppositeZone, SystemSettings::getInstance().getOppositeZone()) << "Setting should be alterd from file" ;
    EXPECT_NE(refCornerZone, SystemSettings::getInstance().getCornerZone()) << "Setting should be alterd from file" ;
    EXPECT_NE(refTriggerPushWingDistance, SystemSettings::getInstance().getTriggerPushWingDistance()) << "Setting should be alterd from file" ;


    int zzzrefChicanoverlap = SystemSettings::getInstance().getChicanOverlap();
    int zzzrefChicanzone = SystemSettings::getInstance().getChicanZone();
    int zzzrefSlowdowndist = SystemSettings::getInstance().getSlowdownDist();
    int zzzrefOppositeZone = SystemSettings::getInstance().getOppositeZone();
    int zzzrefCornerZone = SystemSettings::getInstance().getCornerZone();
    int zzzrefTriggerPushWingDistance = SystemSettings::getInstance().getTriggerPushWingDistance();


}