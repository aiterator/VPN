cmake_minimum_required(VERSION 3.8)
project(VPN)

set(CMAKE_CXX_STANDARD 11)

set(vpn_client_SourceFile VPN_client/main.cpp VPN_client/tun_device.cpp VPN_client/tun_device.h VPN_client/udp_wrapper.cpp VPN_client/udp_wrapper.h)
add_executable(vpnClient ${vpn_client_SourceFile})
target_link_libraries(vpnClient tins pcap)

set(funSource Test_dir/Function.cpp)
add_executable(fun ${funSource})