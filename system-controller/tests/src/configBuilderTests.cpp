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


#include "configBuilder.h"
using namespace std;

std::vector<ConfigObject> getObjsFromStr (std::string configStr)  {  
    std::vector<ConfigObject>  res;    
    for(char& c : configStr) {            
        res.push_back(ConfigObject(std::string(1,c)));
    }        
    return res;
};

class ConfigBuilderTest : public ConfigBuilder {
    public:
    static void parseJsonToObjectsTest(std::string json, std::vector<ConfigObject> & configObjects) {        
        std::string id;  
        parseJsonToObjects(json,configObjects,id);
    }
    static void parseFromJsonTest(std::string json, std::vector<std::shared_ptr<IWing>> & wings) {
        std::string id;  
        parseFromJson(json,wings,id);
    }
    static void parseFromConfigObjectsToWingList(std::vector<ConfigObject> & configObjects,std::vector<std::shared_ptr<IWing>> & wings) {
        std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> wingInfos;
        listWings(configObjects,wingInfos);
        connectWingsWithRelationsAndList(wingInfos,wings,"id");
    }
    static void listWingsTest(std::vector<ConfigObject> & configObjects, std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> & wingInfos) {
        listWings(configObjects,wingInfos);
    }
    static std::shared_ptr<IWing> parseWingTest(std::vector<ConfigObject>::iterator & startOfWing,std::vector<ConfigObject>::iterator & endOfWing, bool isLefOpening) {
        auto startType =  startOfWing->type;
        auto endType = (endOfWing - 1)->type;
        return parseWing(startOfWing,endOfWing,isLefOpening, "id");
    }
     
};


TEST(ConfigBuilder,basicsMiddle ){
    Log::Init();
    ifstream f(utils::getApplicationDirectory() + "/testData/QOX-XXQ_test.json"); //taking file as inputstream
        
    EXPECT_TRUE(f) << "failed to get the file stream";
    ostringstream ss;
    ss << f.rdbuf(); // reading data
   
    std::vector<ConfigObject> configObjects;
    ConfigBuilderTest::parseJsonToObjectsTest(ss.str(), configObjects);
    
    EXPECT_TRUE(configObjects.size() == 7);
    EXPECT_TRUE(configObjects[0].type.compare("Q") == 0);    
    EXPECT_TRUE(configObjects[1].type.compare("O") == 0);    
    EXPECT_TRUE(configObjects[2].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[3].type.compare("-") == 0);    
    EXPECT_TRUE(configObjects[4].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[5].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[6].type.compare("Q") == 0);    

    EXPECT_TRUE(configObjects[2].length == 1990);
    EXPECT_TRUE(configObjects[4].serial.compare("0000000000002") == 0);
    EXPECT_TRUE(configObjects[5].serial.compare("0000000000003") == 0);
    EXPECT_TRUE(configObjects[5].pn.compare("0628253") == 0);
}
TEST(ConfigBuilder,basicsInv){
    Log::Init();
    ifstream f(utils::getApplicationDirectory() + "/testData/X_XX_test.json"); //taking file as inputstream
        
    EXPECT_TRUE(f) << "failed to get the file stream";
    ostringstream ss;
    ss << f.rdbuf(); // reading data
   
    std::vector<ConfigObject> configObjects;
    ConfigBuilderTest::parseJsonToObjectsTest(ss.str(), configObjects);
    
    EXPECT_TRUE(configObjects.size() == 5);
    EXPECT_TRUE(configObjects[0].type.compare("X") == 0);   
    EXPECT_TRUE(configObjects[0].getParseType()==ConfigObjectType::motorized) << "[0] should be type Motorized";
    EXPECT_TRUE(configObjects[1].type.compare("(") == 0);    
    EXPECT_TRUE(configObjects[1].getParseType()==ConfigObjectType::inversStart) << "[1] should be type inversStart";
    EXPECT_TRUE(configObjects[2].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[2].getParseType()==ConfigObjectType::motorized) << "[2] should be type Motorized";
    EXPECT_TRUE(configObjects[3].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[3].getParseType()==ConfigObjectType::motorized) << "[3] should be type Motorized";
    EXPECT_TRUE(configObjects[4].type.compare(")") == 0);    
    EXPECT_TRUE(configObjects[4].getParseType()==ConfigObjectType::inversEnd) << "[4] should be type inversEnd";

    EXPECT_TRUE(configObjects[0].length == 2990);
    EXPECT_TRUE(configObjects[2].length == 2990);
    EXPECT_TRUE(configObjects[3].length == 4990);

    EXPECT_TRUE(configObjects[0].serial.compare("0000000000001") == 0);
    EXPECT_TRUE(configObjects[2].serial.compare("0000000000002") == 0);
    EXPECT_TRUE(configObjects[3].serial.compare("0000000000003") == 0);
}
TEST(ConfigBuilder,basicsCorner){
    Log::Init();
    ifstream f(utils::getApplicationDirectory() + "/testData/XvX_test.json"); //taking file as inputstream
        
    EXPECT_TRUE(f) << "failed to get the file stream";
    ostringstream ss;
    ss << f.rdbuf(); // reading data
   
    std::vector<ConfigObject> configObjects;
    ConfigBuilderTest::parseJsonToObjectsTest(ss.str(), configObjects);
        
    EXPECT_TRUE(configObjects.size() == 3);
    EXPECT_TRUE(configObjects[0].type.compare("X") == 0);    
    EXPECT_TRUE(configObjects[1].type.compare("v") == 0);    
    EXPECT_TRUE(configObjects[2].type.compare("X") == 0);    

    EXPECT_TRUE(configObjects[0].length == 3990);
    EXPECT_TRUE(configObjects[2].length == 3990);

    EXPECT_TRUE(configObjects[0].serial.compare("0000000000001") == 0);
    EXPECT_TRUE(configObjects[2].serial.compare("0000000000002") == 0);

}

TEST(ConfigBuilder,listWings ){
    Log::Init();
    // define testing routines for testing multiple configurations
    typedef std::vector<std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool>> WingTupleList;
    typedef std::vector<ConfigObject> ObjVec;

    auto debugWings = [](WingTupleList &wingInfos, std::string config) {
        std::cout << "CONFIG:" << config << std::endl;
        int counter = 0;
        for (auto &t : wingInfos)
        {
            std::cout << "Wing " << counter << ": ";
            for (auto i = std::get<0>(t); i != std::get<1>(t); ++i)
            {
                std::cout << i->type;
            }
            counter++;
            std::cout << " - " ;
        }
        std::cout << std::endl;
    };
    auto test = [&debugWings](
                    std::string configInfo, ObjVec setup, std::function<void(ObjVec & setup, WingTupleList & wingInfos, std::string configInfo)> testFn) {
        WingTupleList wingInfos;
        ConfigBuilderTest::listWingsTest(setup, wingInfos);
        //debugWings(wingInfos, configInfo);
        testFn(setup, wingInfos, configInfo);
    };

    test("QOX", {ConfigObject("Q"), ConfigObject("O"), ConfigObject("X")}, [](ObjVec &setup, WingTupleList &wingInfos, std::string configInfo) {
        EXPECT_TRUE(wingInfos.size() == 1) << "Wrong wing size for " << configInfo << std::endl;
        int wingNr = 0;
        EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" << configInfo << "] Wrong first element for wing " << wingNr;
        EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" << configInfo << "] Wrong last element for wing " << wingNr;
        EXPECT_FALSE(std::get<2>(wingInfos[wingNr])) << "[" << configInfo << "] " << wingNr << " wing should have opening right";
        EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]))) == 3) << " wing should have correct size";
    });

    test("X(X)", {ConfigObject("X"), ConfigObject("("), ConfigObject("X"), ConfigObject(")")}, [](ObjVec &setup, WingTupleList &wingInfos, std::string configInfo) {
        EXPECT_TRUE(wingInfos.size() == 2) << "Wrong wing size for " << configInfo << std::endl;
        int wingNr = 0;
        EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" << configInfo << "] Wrong first element for wing " << wingNr;
        EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin() + 1) << "[" << configInfo << "] Wrong last element for wing " << wingNr;
        EXPECT_TRUE(std::get<2>(wingInfos[wingNr])) << "[" << configInfo << "] " << wingNr << " wing should have opening left";
        EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]))) == 1) << " wing should have correct size";
        int count = (int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]));
        std::cout<<"wing count for wing " << wingNr << " :" << count << std::endl;
        wingNr = 1;
        EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin() + 1) << "[" << configInfo << "] Wrong first element for wing " << wingNr;
        EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" << configInfo << "] Wrong last element for wing " << wingNr;
        EXPECT_FALSE(std::get<2>(wingInfos[wingNr])) << "[" << configInfo << "] " << wingNr << " wing should have opening right";
        EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]))) == 3) << " wing should have correct size";
        count = (int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]));
        std::cout<<"wing count for wing " << wingNr << " :" << count << std::endl;
    });


    test("(X)X", { ConfigObject("("), ConfigObject("X"), ConfigObject(")"), ConfigObject("X")}, [](ObjVec &setup, WingTupleList &wingInfos, std::string configInfo) {
        EXPECT_TRUE(wingInfos.size() == 2) << "Wrong wing size for " << configInfo << std::endl;
        int wingNr = 0;
        EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" << configInfo << "] Wrong first element for wing " << wingNr;
        EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin() + 3) << "[" << configInfo << "] Wrong last element for wing " << wingNr;
        EXPECT_TRUE(std::get<2>(wingInfos[wingNr])) << "[" << configInfo << "] " << wingNr << " wing should have opening left";
        EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]))) == 3) << " wing should have correct size";
        int count = (int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]));
        std::cout<<"wing count for wing " << wingNr << " :" << count << std::endl;

        wingNr = 1;
        EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin() + 3) << "[" << configInfo << "] Wrong first element for wing " << wingNr;
        EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" << configInfo << "] Wrong last element for wing " << wingNr;
        EXPECT_FALSE(std::get<2>(wingInfos[wingNr])) << "[" << configInfo << "] " << wingNr << " wing should have opening right";
        EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]))) == 1) << " wing should have correct size";
        count = (int)std::distance(std::get<0>(wingInfos[wingNr]), std::get<1>(wingInfos[wingNr]));
        std::cout<<"wing count for wing " << wingNr << " :" << count << std::endl;
    });

    test("XOQ"
        ,{ConfigObject("X"),ConfigObject("O"),ConfigObject("Q")}
        ,[](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            EXPECT_EQ(wingInfos.size(), 1) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] " << wingNr << " wing should have opening left" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 3) << " wing should have correct size" ;
    });

    test("XO(X)"
        ,{ConfigObject("X"),ConfigObject("O"),ConfigObject("("),ConfigObject("X"),ConfigObject(")")}
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {

            //debugWings(wingInfos,configInfo);
            EXPECT_EQ(wingInfos.size(), 2) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) != setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] " << wingNr << " wing should have opening left" ;
            EXPECT_EQ(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))), 2) << wingNr << " wing should have correct size" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) != setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have opening right" ;
            EXPECT_EQ(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) , 3)<< wingNr << " wing should have correct size" ;
    });
    test("(XO)X"
        ,{ConfigObject("("),ConfigObject("X"),ConfigObject("O"),ConfigObject(")"),ConfigObject("X")}
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            //configuration starts with '(' and that is not info of wing 0 so first pointer is to the 'X' and not the '('
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+4) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] " << wingNr << " wing should have opening left" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 4) << " wing should have correct size" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) != setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have opening right" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 1) << " wing should have correct size" ;
    });

    test("QX-XQ"
        ,{ConfigObject("Q"),ConfigObject("X"),ConfigObject("-"),ConfigObject("X"),ConfigObject("Q")}
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
             //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+2) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 2) << " wing should have correct size" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) != setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 2) << " wing should have correct size" ;
    });
    test("QXvXQ"
        ,{ConfigObject("Q"),ConfigObject("X"),ConfigObject("v"),ConfigObject("X"),ConfigObject("Q")}
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+2) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 2) << " wing should have correct size" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) != setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 2) << " wing should have correct size" ;
    });
    test("QOXOXvXQ"
        , getObjsFromStr("QOXOXvXQ")
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+5) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 5) << " wing should have correct size" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) != setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            EXPECT_TRUE(((int)std::distance(std::get<0>(wingInfos[wingNr]),std::get<1>(wingInfos[wingNr]))) == 2) << " wing should have correct size" ;
    });
    test("QX-X(XX)"
        , getObjsFromStr("QX-X(XX)")
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 3) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+2) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+3) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+4) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;            
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            
            wingNr = 2;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+4) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
    });

  test("(X)XX-XQ"
        , getObjsFromStr("(X)XX-XQ")
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 3) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+3) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+3) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+5) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;            
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            
            wingNr = 2;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+6) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
    });


    test("X(X)-X(X)-XOQ"
        , getObjsFromStr("X(X)-X(X)-XOQ")
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 5) <<"Wrong wing size for " << configInfo << std::endl;
            int wingNr = 0;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+1) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            
            wingNr = 1;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+1) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+4) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"]  wing"<< wingNr<<" should have the opening right" ;
            
            wingNr = 2;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+5) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+6) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;
            
            wingNr = 3;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+6) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.begin()+9) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_FALSE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening right" ;
            
            wingNr = 4;
            EXPECT_TRUE(std::get<0>(wingInfos[wingNr]) == setup.begin()+10) << "[" <<configInfo<<"] Wrong first element for wing " << wingNr;
            EXPECT_TRUE(std::get<1>(wingInfos[wingNr]) == setup.end()) << "[" <<configInfo<<"] Wrong last element for wing " << wingNr;
            EXPECT_TRUE(std::get<2>(wingInfos[wingNr]) ) << "[" <<configInfo<<"] "<< wingNr << " wing should have the opening left" ;    
            
    });

    test("QOOX-X(X)-(X)XOX"
        , getObjsFromStr("QOOX-X(X)-(X)XOX")
        ,[& debugWings](ObjVec & setup , WingTupleList & wingInfos, std::string configInfo) {
            //debugWings(wingInfos,configInfo);
            EXPECT_TRUE(wingInfos.size() == 5) <<"Wrong wing size for " << configInfo << std::endl;
    });
   
}
TEST(ConfigBuilder,parseWing ){
    Log::Init();
    // define testing routines for testing multiple configurations
    typedef std::tuple<std::vector<ConfigObject>::iterator,std::vector<ConfigObject>::iterator,bool> WingTuple;
    typedef std::vector<ConfigObject> ObjVec;

    auto test = [](
        std::string configInfo, ObjVec objVec,bool isOpeningLeft, int expectedNrMotors, std::function<void(std::shared_ptr<IWing> & wing, std::string configInfo)>  testFn)
        {
            auto b=objVec.begin();
            auto e=objVec.end();            
            try {
                auto wingRes = ConfigBuilderTest::parseWingTest(b,e,isOpeningLeft);
                testFn(wingRes,configInfo);
                EXPECT_EQ(wingRes->getMotors().size(), expectedNrMotors) <<"Wrong number of motors on wing " << configInfo << std::endl;
            }
            catch (...) { 
                EXPECT_TRUE(false)<< configInfo<< " Exception thrown !" ;
            }
            
        };
       
    test("QOX",{ConfigObject("Q"),ConfigObject("O"),ConfigObject("X")},false,1,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
          // expect no exception to be thrown
        std::vector<SlaveType> slavesTree ;
        auto masterWindow = wing->getMasterWindow();
        masterWindow->getSlaveTypesTree(slavesTree);
        EXPECT_EQ(slavesTree.size(),1) << "Only the master has one slave" ;
        EXPECT_TRUE(slavesTree[0]==SlaveType::Passive) << "The master has a passive element as slave";
    });
    test("QXXX",{ConfigObject("Q"),ConfigObject("X"),ConfigObject("X"),ConfigObject("X")},false,3,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        std::vector<SlaveType> slavesTree ;
        auto masterWindow = wing->getMasterWindow();
        masterWindow->getSlaveTypesTree(slavesTree);
        EXPECT_EQ(slavesTree.size(),2) << "slave of master also has one slave" ;     
        EXPECT_TRUE(slavesTree[0]==SlaveType::Motor) << "The master has a motorized element as slave";
        EXPECT_TRUE(slavesTree[1]==SlaveType::Motor) << "The slave has a motorized element as slave";
    });
    test("XXQ",{ConfigObject("X"),ConfigObject("X"),ConfigObject("Q")},true,2,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        auto masterWindow = wing->getMasterWindow();
        EXPECT_TRUE(true);
    });


    test("XOQ",{ConfigObject("X"),ConfigObject("O"),ConfigObject("Q")},true,1,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("XXXXQ",{ConfigObject("X"),ConfigObject("X"),ConfigObject("X"),ConfigObject("X"),ConfigObject("Q")}
            ,true,4,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
                // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("QXXXX",{ConfigObject("Q"),ConfigObject("X"),ConfigObject("X"),ConfigObject("X"),ConfigObject("X")}
            ,false,4,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("X",{ConfigObject("X")},false,1,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("X",{ConfigObject("X")},true,1,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("(XX)",{ConfigObject("("),ConfigObject("X"),ConfigObject("X"),ConfigObject(")")},false,2
        ,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    test("(XX)",{ConfigObject("("),ConfigObject("X"),ConfigObject("X"),ConfigObject(")")},true,2
        ,[](std::shared_ptr<IWing> & wing, std::string configInfo) {
        // expect no exception to be thrown
        EXPECT_TRUE(true) ;
    });
    
}
TEST(ConfigBuilder,testWingRelations ){

      Log::Init();
    // define testing routines for testing multiple configurations
    typedef std::vector<ConfigObject> ObjVec;    
    auto test = [](
        std::string configInfo, ObjVec setup, std::function<void(ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo)>  testFn)
        {   
            std::vector<std::shared_ptr<IWing>> wings;
            ConfigBuilderTest::parseFromConfigObjectsToWingList(setup,wings);
            testFn(setup,wings,configInfo);
        };    
        
    test("QOX"
        , getObjsFromStr("QOX")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 1) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->empty()) << "There should be no sibling when config is only one wing "  << configInfo <<  std::endl;
    });

     test("QXvXQ"
        , getObjsFromStr("QXvXQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::CornerFemale) << "Wing idx 0 of "  << configInfo << " should have a female sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::CornerMale) << "Wing idx 1 of "  << configInfo << " should have a male sibling" << std::endl;
    });

    test("QOOXXvXXXOXXQ"
        , getObjsFromStr("QOOXXvXXXOXXQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::CornerFemale) << "Wing idx 0 of "  << configInfo << " should have a female sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::CornerMale) << "Wing idx 1 of "  << configInfo << " should have a male sibling" << std::endl;
    });


     test("QX-XQ"
        , getObjsFromStr("QX-XQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::MiddleFemale) << "Wing idx 0 of "  << configInfo << " should have a female sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::MiddleMale) << "Wing idx 1 of "  << configInfo << " should have a male sibling" << std::endl;
    });
 

    test("QOX-XXQ"
       , getObjsFromStr("QOX-XXQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::MiddleFemale) << "Wing idx 0 of "  << configInfo << " should have a female sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::MiddleMale) << "Wing idx 1 of "  << configInfo << " should have a male sibling" << std::endl;
    });


    test("(X)X"
    , getObjsFromStr("(X)X")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::Opposite) << "Wing idx 0 of "  << configInfo << " should have an opposite sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::Opposite) << "Wing idx 1 of "  << configInfo << " should have an opposite sibling" << std::endl;
    });

    test("X(X)"
    , getObjsFromStr("X(X)")
    ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
        EXPECT_TRUE(wings.size() == 2) <<"Wrong wing size for " << configInfo << std::endl;            
        EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
        EXPECT_TRUE(wings[1]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 1 of "  << configInfo <<  std::endl;

        auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
        auto siblingTupleWing1 = (*(wings[1]->getSiblings()))[0];
        EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::Opposite) << "Wing idx 0 of "  << configInfo << " should have an opposite sibling" << std::endl;
        EXPECT_TRUE(std::get<1>(siblingTupleWing1) ==  WingSiblingType::Opposite) << "Wing idx 1 of "  << configInfo << " should have an opposite sibling" << std::endl;
    });



    test("(X)XvXXQ"
           , getObjsFromStr("(X)XvXXQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 3) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 2) << "There should be two sibling when for wing idx 1 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[2]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 2 of "  << configInfo <<  std::endl;

            auto siblingTupleWing0 = (*(wings[0]->getSiblings()))[0];
            auto siblingTupleWing1_A = (*(wings[1]->getSiblings()))[0];
            auto siblingTupleWing1_B = (*(wings[1]->getSiblings()))[1];
            auto siblingTupleWing2 = (*(wings[2]->getSiblings()))[0];
            EXPECT_TRUE(std::get<1>(siblingTupleWing0) == WingSiblingType::Opposite) << "Wing idx 0 of "  << configInfo << " should have an opposite sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1_A) ==  WingSiblingType::Opposite) << "Wing idx 1A of "  << configInfo << " should have an opposite sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing1_B) ==  WingSiblingType::CornerFemale) << "Wing idx 1B of "  << configInfo << " should have an Corner female sibling" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingTupleWing2) ==  WingSiblingType::CornerMale) << "Wing idx 2 of "  << configInfo << " should have an Corner male sibling" << std::endl;
    });


      test("X(X)-X(X)-XOQ"
        , getObjsFromStr("X(X)-X(X)-XOQ")
        ,[](ObjVec & setup , std::vector<std::shared_ptr<IWing>> & wings, std::string configInfo) {
            EXPECT_TRUE(wings.size() == 5) <<"Wrong wing size for " << configInfo << std::endl;            
            EXPECT_TRUE(wings[0]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 0 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[1]->getSiblings()->size() == 2) << "There should be two sibling when for wing idx 1 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[2]->getSiblings()->size() == 2) << "There should be two sibling when for wing idx 2 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[3]->getSiblings()->size() == 2) << "There should be two sibling when for wing idx 3 of "  << configInfo <<  std::endl;
            EXPECT_TRUE(wings[4]->getSiblings()->size() == 1) << "There should be one sibling when for wing idx 4 of "  << configInfo <<  std::endl;

            auto siblingFullTupleWing0 = *(wings[0]->getSiblings());
            auto siblingFullTupleWing1 = *(wings[1]->getSiblings());
            auto siblingFullTupleWing2 = *(wings[2]->getSiblings());
            auto siblingFullTupleWing3 = *(wings[3]->getSiblings());
            auto siblingFullTupleWing4 = *(wings[4]->getSiblings());

        
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing0[0]) == WingSiblingType::Opposite) << "check wing 0" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing1[0]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing1[0]) == WingSiblingType::MiddleFemale) << "check wing 1" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing1[1]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing1[1]) == WingSiblingType::MiddleFemale) << "check wing 1" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing1[0]) != std::get<1>(siblingFullTupleWing1[1])) << "check wing 1" << std::endl;

            EXPECT_TRUE(std::get<1>(siblingFullTupleWing2[0]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing2[0]) == WingSiblingType::MiddleMale) << "check wing 2" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing2[1]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing2[1]) == WingSiblingType::MiddleMale) << "check wing 2" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing2[0]) != std::get<1>(siblingFullTupleWing2[1])) << "check wing 2" << std::endl;

               EXPECT_TRUE(std::get<1>(siblingFullTupleWing3[0]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing3[0]) == WingSiblingType::MiddleFemale) << "check wing 3" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing3[1]) == WingSiblingType::Opposite
                || std::get<1>(siblingFullTupleWing3[1]) == WingSiblingType::MiddleFemale) << "check wing 3" << std::endl;
            EXPECT_TRUE(std::get<1>(siblingFullTupleWing3[0]) != std::get<1>(siblingFullTupleWing3[1])) << "check wing 3" << std::endl;

            EXPECT_TRUE(std::get<1>(siblingFullTupleWing4[0]) == WingSiblingType::MiddleMale) << "check wing 4" << std::endl;
    });

}

       
