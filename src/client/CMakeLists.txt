project(nc_client VERSION 1.0.0.1 LANGUAGES CXX)
configure_file("version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")
target_compile_options(${PROJECT_NAME}
PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>: -D_CRT_SECURE_NO_WARNINGS /Zi /std:c++17 /MT /DEBUG>
) 

target_link_options(${PROJECT_NAME}
PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang>: -fsanitize=thread>
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)
target_sources(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
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