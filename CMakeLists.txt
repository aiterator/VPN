cmake_minimum_required(VERSION 3.8)
project(VPN)

set(CMAKE_CXX_STANDARD 11)

set(vpn_client_SourceFile VPN_client/main.cpp VPN_client/TunDevice.cpp VPN_client/TunDevice.h)
add_executable(vpnClient ${vpn_client_SourceFile})
target_link_libraries(vpnClient tins pcap)

set(funSource Test_dir/Function.cpp)
add_executable(fun ${funSource})