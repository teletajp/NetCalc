project(nc_server VERSION 1.0.0.1 LANGUAGES CXX)
configure_file("version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_compile_options(${PROJECT_NAME}
PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>: -D_CRT_SECURE_NO_WARNINGS /Zi /std:c++17 /MT /DEBUG>
) 

target_link_options(${PROJECT_NAME}
PRIVATE
    #$<$<CXX_COMPILER_ID:AppleClang>: -fsanitize=thread>
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
    fmt::fmt
)
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${FMT}
)
target_sources(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/listener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/listener.cpp
)

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/thread/runnable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/thread/runnable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/iserver_engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/select_base_server_engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/newtork_initializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/newtork_initializer.cpp
)