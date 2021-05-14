CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)
PROJECT(hisi_driver)

SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/../SDK/v2/lib")

SET(SDK_LIBRARIES
    isp
    hdmi
    sns_gc2053

    mpi
    securec

    dnvqe
    upvqe
    VoiceEngine

    _hiawb
    _hiae
    _hidrc
    _hildci
    _hidehaze
    _hicalcflicker

    # static ordering requires duplicates
    mpi
    pthread
    m
    dl
)

INCLUDE_DIRECTORIES(
    "${CMAKE_SOURCE_DIR}/../SDK/v2/include"
    "${CMAKE_SOURCE_DIR}/Include"
)
FILE(GLOB_RECURSE DRIVER_SOURCE "${CMAKE_SOURCE_DIR}/Source/*.cpp")
ADD_LIBRARY(hi3516cv500_drivers ${DRIVER_SOURCE})

ADD_EXECUTABLE(${PROJECT_NAME} ${COMMON_SOURCE} "main.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    hi3516cv500_drivers
    ${SDK_LIBRARIES}
)
