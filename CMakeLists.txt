cmake_minimum_required(VERSION 3.8)
project(VPN)

set(CMAKE_CXX_STANDARD 11)

set(vpn_client_SourceFile VPN_client/client.cpp tun_device.cpp tun_device.h udp_wrapper.cpp udp_wrapper.h)
add_executable(vpnClient ${vpn_client_SourceFile})

set(vpn_server_SourceFile VPN_server/server.cpp VPN_server/udp_wrapper.cpp VPN_server/udp_wrapper.h)
add_executable(vpnServer ${vpn_server_SourceFile})
target_link_libraries(vpnServer tins pcap pthread)

set(vpn_server1_SourceFile VPN_server1/server.cpp VPN_server1/udp_wrapper.cpp VPN_server1/udp_wrapper.h)
add_executable(vpnServer1 ${vpn_server1_SourceFile})
target_link_libraries(vpnServer1 tins pcap pthread)

set(vpn_server2_SourceFile VPN_server2/server.cpp tun_device.cpp tun_device.h udp_wrapper.cpp udp_wrapper.h)
add_executable(vpnServer2 ${vpn_server2_SourceFile})
target_link_libraries(vpnServer2 tins pcap pthread)

set(getopt_SourceFile Test_dir/getOpt.cpp)
add_executable(getopt_SourceFile ${getopt_SourceFile})
