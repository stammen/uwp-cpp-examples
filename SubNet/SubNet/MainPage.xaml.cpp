﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <sstream>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace SubNet;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

int MainPage::GetInterfaces()
{
    std::stringstream ss;

    WSADATA WinsockData;
    if (WSAStartup(MAKEWORD(2, 2), &WinsockData) != 0) {
        ss << "Failed to find Winsock 2.2!" << std::endl;
    }

    SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sd == SOCKET_ERROR) {
        ss << "Failed to get a socket. Error " << WSAGetLastError() <<
            std::endl; ;
    }

    INTERFACE_INFO InterfaceList[20];
    unsigned long nBytesReturned;
    if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
        sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
        ss << "Failed calling WSAIoctl: error " << WSAGetLastError() <<
            std::endl;
        return 1;
    }

    int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
    ss << "There are " << nNumInterfaces << " interfaces:" << std::endl;
    for (int i = 0; i < nNumInterfaces; ++i) {
        ss << std::endl;

        sockaddr_in *pAddress;
        pAddress = (sockaddr_in *) & (InterfaceList[i].iiAddress);
        ss << " " << inet_ntoa(pAddress->sin_addr);

        pAddress = (sockaddr_in *) & (InterfaceList[i].iiBroadcastAddress);
        ss << " has bcast " << inet_ntoa(pAddress->sin_addr);

        pAddress = (sockaddr_in *) & (InterfaceList[i].iiNetmask);
        ss << " and netmask " << inet_ntoa(pAddress->sin_addr) << std::endl;

        ss << " Iface is ";
        u_long nFlags = InterfaceList[i].iiFlags;
        if (nFlags & IFF_UP) ss << "up";
        else                 ss << "down";
        if (nFlags & IFF_POINTTOPOINT) ss << ", is point-to-point";
        if (nFlags & IFF_LOOPBACK)     ss << ", is a loopback iface";
        ss << ", and can do: ";
        if (nFlags & IFF_BROADCAST) ss << "bcast ";
        if (nFlags & IFF_MULTICAST) ss << "multicast ";
        ss << std::endl;
    }

    std::string s = ss.str();
    std::wstring w(s.begin(), s.end());
    results->Text = ref new Platform::String(w.c_str());

    WSACleanup();

    return 0;
}


MainPage::MainPage()
{
	InitializeComponent();
    GetInterfaces();
}
