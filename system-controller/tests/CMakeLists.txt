cmake_minimum_required(VERSION 3.1.3)


## Make GTEST availible by letting CMAKE download this into the project
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(SPDLOG_LIB_PATH_INC ${PROJECT_SOURCE_DIR}/lib/spdlog/include)
set(RAPIDJSON_LIB_PATH_INC ${PROJECT_SOURCE_DIR}/lib/rapidjson)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

## Setup test project
set (THIS systemControllerTests)


set(SRC_PATH src)
set(INCLUDE_PATH include 
                ${PROJECT_SOURCE_DIR}/include
                )
set(SOURCE_FILES
                ${SRC_PATH}/configBuilderTests.cpp 
                ${SRC_PATH}/systemSettingsParserTests.cpp
                ${SRC_PATH}/SystemSettingsTests.cpp
                ${SRC_PATH}/commandsManagerTests.cpp
                ${SRC_PATH}/cornerScenarioTests.cpp 
                ${SRC_PATH}/masterMotorizedWindowTests.cpp
                ${SRC_PATH}/motorizedWindowTests.cpp
                ${SRC_PATH}/mqttMotorSim.cpp
                ${SRC_PATH}/passiveWindowTests.cpp
                ${SRC_PATH}/testMotorMotionManager.cpp
                ${SRC_PATH}/topicHandlerTests.cpp
                ${SRC_PATH}/testWing.cpp
                ${SRC_PATH}/verifier.cpp
                ${SRC_PATH}/windowPushZoneTests.cpp
                ${SRC_PATH}/wingCalibrationHandlerTests.cpp
                ${SRC_PATH}/wingTests.cpp
                ${SRC_PATH}/wingDataTests.cpp
                ${SRC_PATH}/motorDataTests.cpp
                ${SRC_PATH}/motorsHandlerTests.cpp
                ${SRC_PATH}/movingWindowTests.cpp
                ${SRC_PATH}/motorMotionManagerTests.cpp
                ${SRC_PATH}/mqttMotorTests.cpp
                ${SRC_PATH}/wingsHandlerTests.cpp                
                ${SRC_PATH}/wingRelationManagerTests.cpp     
                ${SRC_PATH}/wingInputTranslatorTests.cpp
                )
                
file(COPY testData DESTINATION .)
add_executable( ${THIS} ${SOURCE_FILES})
target_include_directories( ${THIS} PRIVATE ${INCLUDE_PATH})
target_include_directories(${THIS} PRIVATE ${RAPIDJSON_LIB_PATH_INC})
target_include_directories(${THIS} PRIVATE ${SPDLOG_LIB_PATH_INC})
target_link_libraries(${THIS} PUBLIC 
        gtest_main
        systemControllerLib
)
add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)