#ifndef MOTORPARSER_H
#define MOTORPARSER_H

#include "pch.h"

namespace motorparser {


    struct MotorParseResult {
        MotorParseResult(int stroke,std::string serial, std::string pn) : stroke(stroke),serial(serial),pn(pn){}
        int stroke;
        std::string serial;
        std::string pn; 
    };
    static void parse(std::string json, bool shouldHaveStrokes,std::vector<MotorParseResult> & motors) {

        rapidjson::Document doc;
        doc.Parse(json.data());

        if (doc.HasParseError()) {
            LOG_CRITICAL_THROW("invalid JSON found in config");        
        }
        if (doc.IsObject() == false) {
            LOG_CRITICAL_THROW("unexpected JSON format, expected object as root: ");
        }

        rapidjson::Value& c = doc["config"];
        if ( !c.IsArray()) {
            LOG_CRITICAL_THROW("Failed to parse a configuration due missing array");
        }

        for (int i =0; i<c.Size() ; i++ ){
            auto & e = c[i];
            rapidjson::Value& t = e["type"];
            if ( !t.IsString()) {
                LOG_CRITICAL_THROW("Failed to parse type of element in configuration");
            }
            std::string elType =  t.GetString();        

            if ( elType.compare("X") ==0 || elType.compare("x") ==0 ) {
            
                // only 
                rapidjson::Value& motorStroke = e["stroke"];
                if ( !motorStroke.IsNumber() && shouldHaveStrokes) {
                    LOG_CRITICAL_THROW("Failed to parse stroke flag of motor element");
                }
                rapidjson::Value& serial = e["serial"];
                if ( !serial.IsString()) {
                    LOG_CRITICAL_THROW("Failed to parse serial of motor element");
                }
                rapidjson::Value& pn = e["pn"];
                if ( !pn.IsString()) {
                    LOG_CRITICAL_THROW("Failed to parse productnumber (pn) of motor element");
                }

                motors.push_back( MotorParseResult(motorStroke.GetInt(),serial.GetString(),pn.GetString()));

            } 
        }

    }
}

#endif
