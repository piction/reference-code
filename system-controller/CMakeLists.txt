cmake_minimum_required(VERSION 3.16)

#option(testingEnabled "TESTING_ENABLED" OFF)
message(ENABLE_TESTS: ${ENABLE_TESTS})

set (THISLIB systemControllerLib)
set (THISPROGRAM systemController)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#add_compile_options(-save-temps)
#set(THREADS_PREFER_PTHREAD_FLAG ON)


## setup project
project(system-controller LANGUAGES CXX)

set(RAPIDJSON_LIB_PATH_INC ${PROJECT_SOURCE_DIR}/lib/rapidjson)
set(SPDLOG_LIB_PATH_INC ${PROJECT_SOURCE_DIR}/lib/spdlog/include)

set(SRC_PATH ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)
set(SOURCE_FILES ${SRC_PATH}/commandsManager.cpp 
                ${SRC_PATH}/configBuilder.cpp 
                ${SRC_PATH}/commandLineParser.cpp
                ${SRC_PATH}/log.cpp 
                ${SRC_PATH}/motorizedWindow.cpp
                ${SRC_PATH}/masterMotorizedWindow.cpp
                ${SRC_PATH}/motorMotionManager.cpp
                ${SRC_PATH}/motorsHandler.cpp
                ${SRC_PATH}/motorData.cpp
                ${SRC_PATH}/movingWindow.cpp
                ${SRC_PATH}/mqttManager.cpp
                ${SRC_PATH}/mqttData.cpp
                ${SRC_PATH}/mqttMotor.cpp
                ${SRC_PATH}/passiveWindow.cpp
                ${SRC_PATH}/topicHandler.cpp
                ${SRC_PATH}/wingData.cpp
                ${SRC_PATH}/wingRelationManager.cpp                
                ${SRC_PATH}/wingsHandler.cpp
                ${SRC_PATH}/wingStatusPublisher.cpp
                ${SRC_PATH}/wing.cpp
                ${SRC_PATH}/wingInputTranslator.cpp
                ${SRC_PATH}/wingCalibrationHandler.cpp
                )


add_library(${THISLIB} STATIC ${SOURCE_FILES} )
target_precompile_headers(${THISLIB} PUBLIC ${INCLUDE_PATH}/pch.h)
target_include_directories(${THISLIB} PRIVATE ${INCLUDE_PATH})
target_include_directories(${THISLIB} PRIVATE ${RAPIDJSON_LIB_PATH_INC})
target_include_directories(${THISLIB} PRIVATE ${SPDLOG_LIB_PATH_INC})
target_link_libraries(${THISLIB} pthread mosquittopp )
target_sources(${THISLIB} PRIVATE ${SOURCE_FILES})

add_executable(${THISPROGRAM} main.cpp )
target_precompile_headers(${THISPROGRAM} PUBLIC ${INCLUDE_PATH}/pch.h)
target_link_libraries(${THISPROGRAM} ${THISLIB}  )
target_include_directories(${THISPROGRAM} PRIVATE ${RAPIDJSON_LIB_PATH_INC})
target_include_directories(${THISPROGRAM} PRIVATE ${SPDLOG_LIB_PATH_INC})
target_include_directories(${THISPROGRAM} PRIVATE ${INCLUDE_PATH})

if(${ENABLE_TESTS}) 
    message(done testing)
    enable_testing()
    add_subdirectory(tests)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")

