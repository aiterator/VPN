cmake_minimum_required(VERSION 3.8)
project(VPN)

set(CMAKE_CXX_STANDARD 11)

set(vpn_client_SourceFile VPN_client/client.cpp VPN_client/udp_wrapper.cpp VPN_client/udp_wrapper.h tun_device.cpp tun_device.h)
add_executable(vpnClient ${vpn_client_SourceFile})
target_link_libraries(vpnClient tins pcap pthread)

set(vpn_server_SourceFile VPN_server/server.cpp VPN_server/udp_wrapper.cpp VPN_server/udp_wrapper.h tun_device.cpp tun_device.h)
add_executable(vpnServer ${vpn_server_SourceFile})
target_link_libraries(vpnServer tins pcap pthread)

set(getopt_SourceFile Test_dir/getOpt.cpp)
add_executable(getopt_SourceFile ${getopt_SourceFile})
