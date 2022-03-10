#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include "log.h"
#include <algorithm>

class SystemSettings {
public:
    static SystemSettings& getInstance()
    {
        static SystemSettings instance;
        return instance;
    }

    int getChicanOverlap() { return _chicanOverlap; }
    int getChicanZone() { return _chicanZone; }
    int getSlowdownDist() { return _slowdownDist; }
    int getCornerZone() { return _cornerZone; }
    int getOppositeZone() { return _oppositeZone; }
    int getTriggerPushWingDistance() { return _triggerPushWingDistance; }

    void setChicanOverlap(int dist)
    {
        _chicanOverlap = std::min(std::max(dist, 100),600);
        if (_chicanOverlap != dist) {
            LOG_WARNING("chican overlap requested out of boundries [100-600]: " + std::to_string(dist) + " set to " + std::to_string(_chicanOverlap));
        }
        LOG_INFO("chican overlap set: " + std::to_string(_chicanOverlap));
        
    }
    void setChicanZone(int dist)
    {
        _chicanZone =std::min(std::max(dist, 300),1200);
        if (_chicanZone != dist) {
            LOG_WARNING("chicanZone overlap requested out of boundries [300-1200]: " + std::to_string(dist) + " set to " + std::to_string(_chicanZone));
        }
        LOG_INFO("chicanZone overlap set: " + std::to_string(_chicanZone));
    }
    void setSlowdownDist(int dist)
    {
        _slowdownDist = std::min(std::max(dist, 150),800);
        if (_slowdownDist != dist) {
            LOG_WARNING("Slowdowndist overlap requested out of boundries [150-800]: " + std::to_string(dist) + " set to " + std::to_string(_slowdownDist));
        }
        LOG_INFO("Slowdowndist overlap set: " + std::to_string(_slowdownDist));
    }
    void setCornerZone(int dist)
    {
        _cornerZone = std::min(std::max(dist, 90),500);
        if (_cornerZone != dist) {
            LOG_WARNING("CornerZone overlap requested out of boundries [90-500]: " + std::to_string(dist) + " set to " + std::to_string(_cornerZone));
        }
        LOG_INFO("CornerZone overlap set: " + std::to_string(_cornerZone));
    }
    void setOppositeZone(int dist)
    {
        _oppositeZone = std::min(std::max(dist, 90),500);
        if (_oppositeZone != dist) {
            LOG_WARNING("OppositeZone overlap requested out of boundries [90-500]: " + std::to_string(dist) + " set to " + std::to_string(_oppositeZone));
        }
        LOG_INFO("OppositeZone overlap set: " + std::to_string(_oppositeZone));
    }
    void setTriggerPushWingDistance(int dist)
    {
        _triggerPushWingDistance = std::min(std::max(dist, 250),800);
        
        if (_triggerPushWingDistance != dist) {
            LOG_WARNING("TriggerPushWingDistance overlap requested out of boundries [250-800]: " + std::to_string(dist) + " set to " + std::to_string(_triggerPushWingDistance));
        }
        LOG_INFO("TriggerPushWingDistance overlap set: " + std::to_string(_triggerPushWingDistance));
    }

private:
    SystemSettings()
    {

        _chicanOverlap = 100; // Stopzone when two windows are that close to each other
        _chicanZone = 600; // overlap zone where we start pushing other vents
        _slowdownDist = 200; // demanding distance to slow down

        _cornerZone = 100; // no go zone for corner
        _oppositeZone = 100; // no go zone for opposite
        _triggerPushWingDistance = 300; // distance to push another wing
    }
    ~SystemSettings() = default;
    SystemSettings(const SystemSettings&) = delete;
    SystemSettings& operator=(const SystemSettings&) = delete;

    int _chicanOverlap; // Stopzone when two windows are that close to each other
    int _chicanZone; // overlap zone where we start pushing other vents
    int _slowdownDist; // demanding distance to slow down
    int _cornerZone;
    int _oppositeZone;
    int _triggerPushWingDistance;
};

#endif