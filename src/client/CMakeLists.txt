project(nc_client VERSION 1.0.0.1 LANGUAGES CXX)
configure_file("version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_compile_options(${PROJECT_NAME}
PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>: -D_CRT_SECURE_NO_WARNINGS /Zi /std:c++14 /MT /DEBUG>
) 

target_link_options(${PROJECT_NAME}
PRIVATE
    #$<$<CXX_COMPILER_ID:AppleClang>: -fsanitize=thread>
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:pthread>
)
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)
target_sources(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/terminal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/terminal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_connector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_connector.cpp
)

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/thread/runnable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/thread/runnable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/newtork_initializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/net/newtork_initializer.cpp
)