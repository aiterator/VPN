cmake_minimum_required(VERSION 3.8)
project(VPN)

set(CMAKE_CXX_STANDARD 11)

set(vpn_client VPN_client/main.cpp)
add_executable(vpnClient ${vpn_client})