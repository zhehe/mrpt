/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2011  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */


#include <mrpt/base.h>  // Precompiled headers

#include <mrpt/utils/CWirelessPower.h>


#include <mrpt/config.h>

using namespace mrpt::utils;


#ifdef MRPT_OS_WINDOWS

// Linking pragmas must always be in the .cpp, otherwise the lib will be linked by
// user apps, while if MRPT is a DLL this is unnecesary!
#pragma comment(lib, "Wlanapi.lib")


/*---------------------------------------------------------------
					setNet
   Set the SSID and GUID of the target network
 ---------------------------------------------------------------*/

void CWirelessPower::setNet(std::string ssid_, std::string guid_)
{
	ssid = ssid_;
	guid = guid_;
	ConnectWlanServerW();
}
/*---------------------------------------------------------------
					ConnectWlanServerW
   Get a connection to the WLAN server
 ---------------------------------------------------------------*/
void	CWirelessPower::ConnectWlanServerW()
{
	DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
	DWORD dwResult = 0;			// Result of the API call

	// open connection to server
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
		// if an error ocurred
		std::stringstream excmsg;
		excmsg << "WlanOpenHandle failed with error: " << dwResult << std::endl;
        // You can use FormatMessage here to find out why the function failed
		THROW_EXCEPTION(excmsg.str());
    }
}


/*---------------------------------------------------------------
					ListInterfaces
   Gets a list of the interfaces
 ---------------------------------------------------------------*/

std::vector<std::string>	CWirelessPower::ListInterfaces()
{
	// In windows, this function is a wrapper to ListInterfacesW

	std::vector<PWLAN_INTERFACE_INFO> ifaces;					// vector containing the interface entries (Windows format)
	std::vector<PWLAN_INTERFACE_INFO>::iterator ifacesIter;		// iterator to run through the previous list
	std::vector<std::string> output;							// output vector of strings

	// get the list
	ifaces = ListInterfacesW();

	// iterate thrugh list and get each GUID as a string
	for (ifacesIter = ifaces.begin(); ifacesIter != ifaces.end() ; ifacesIter++){
		output.push_back(GUID2Str((*ifacesIter)->InterfaceGuid));
	}

	return output;
}

/*---------------------------------------------------------------
					ListInterfacesW
   Gets a list of the interfaces (Windows)
 ---------------------------------------------------------------*/

std::vector<PWLAN_INTERFACE_INFO>	CWirelessPower::ListInterfacesW()
{
	// Get a list of the available interfaces

	std::vector<PWLAN_INTERFACE_INFO> outputVector;			// start the output vector
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;				// list of WLAN interfaces
    PWLAN_INTERFACE_INFO pIfInfo = NULL;					// information element for one interface
	DWORD dwResult = 0;

	int i;

	// Call the interface enumeration function of the API
	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
	// check result
    if (dwResult != ERROR_SUCCESS) {
		// In case of error, raise an exception
		std::stringstream excmsg;
		excmsg << "WlanEnumInterfaces failed with error: " << dwResult << std::endl;
		THROW_EXCEPTION(excmsg.str());
        // You can use FormatMessage here to find out why the function failed
    } else {
		// iterate throught interfaces to add them to the output vector

		for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) {

            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
			outputVector.push_back(pIfInfo);
		}
	}
	return outputVector;

}


/*---------------------------------------------------------------
					GetInterfaceW
   Gets a handler for the interface (Windows)
 ---------------------------------------------------------------*/

PWLAN_INTERFACE_INFO CWirelessPower::GetInterfaceW()
{
	// Get interface given the GUID as a string (by the guid property of the object)


	std::vector<PWLAN_INTERFACE_INFO> ifaceList;						// interface list
	std::vector<PWLAN_INTERFACE_INFO>::iterator ifaceIter;				// iterator
	PWLAN_INTERFACE_INFO output = NULL;									// interface info element

	// get a list of all the interfaces
	ifaceList = ListInterfacesW();

	// search for the interface that has the given GUID
	for(ifaceIter = ifaceList.begin(); ifaceIter != ifaceList.end(); ifaceIter++){
		if (GUID2Str((*ifaceIter)->InterfaceGuid) == guid){
			output = *ifaceIter;
			break;
		}
	}

	return output;
}


/*---------------------------------------------------------------
					ListNetworks
   Gets a list of the networks available for the interface
 ---------------------------------------------------------------*/

std::vector<std::string>	CWirelessPower::ListNetworks()
{
	// Wrapper function to ListNetworksW in Windows
	PWLAN_INTERFACE_INFO iface;			// Information element for an interface
	std::vector<std::string> output;	// Output vector

	iface = GetInterfaceW();			// Get the interface handler

	// Get the list of networks
	std::vector<PWLAN_AVAILABLE_NETWORK> pBssList = ListNetworksW(iface);

	// Iterate through the list and save the names as strings
	std::vector<PWLAN_AVAILABLE_NETWORK>::iterator netIter;
	for(netIter = pBssList.begin(); netIter != pBssList.end() ; netIter++){
		output.push_back( std::string((char*)((*netIter)->dot11Ssid.ucSSID)));
	}

	return output;

}

/*---------------------------------------------------------------
					ListNetworksW
   Gets a list of the networks available for the interface (in Windows)
 ---------------------------------------------------------------*/
std::vector<PWLAN_AVAILABLE_NETWORK>	CWirelessPower::ListNetworksW(PWLAN_INTERFACE_INFO iface)
{
	// Start variables
	DWORD dwResult = 0;
    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;	// list of available networks
    PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;		// information element for one interface
	GUID ifaceGuid = iface->InterfaceGuid;			// Get GUID of the interface
	int j;
	std::vector<PWLAN_AVAILABLE_NETWORK> outputVector;	// output vector
	WCHAR GuidString[39] = {0};

	// Call the Windows API and get a list of the networks available through the interface
	dwResult = WlanGetAvailableNetworkList(hClient, &ifaceGuid, 0, NULL, &pBssList);

	// Check the result of the call
	if (dwResult != ERROR_SUCCESS) {
		// In case an error ocurred
		std::stringstream excmsg;
		excmsg << "WlanGetAvailableNetworkList failed with error: " << dwResult << std::endl;
		THROW_EXCEPTION(excmsg.str());
	} else {
		// for each network, get its info and save it
		for (j = 0; j < pBssList->dwNumberOfItems; j++) {
			pBssEntry = (WLAN_AVAILABLE_NETWORK *) & pBssList->Network[j];	// get entry for network
			outputVector.push_back(pBssEntry);	// save entry
		}
	}
	return outputVector;
}


/*---------------------------------------------------------------
					GetNetworkW
   Gets a handler to a wireless network in Windows
 ---------------------------------------------------------------*/
PWLAN_AVAILABLE_NETWORK CWirelessPower::GetNetworkW()
{
	// Variables
	PWLAN_INTERFACE_INFO iface;			// interface handler
	PWLAN_AVAILABLE_NETWORK output;		// output network handler

	// Get a handler to the interface
	iface = GetInterfaceW();
	// Get the list of networks
	std::vector<PWLAN_AVAILABLE_NETWORK> pBssList = ListNetworksW(iface);

	// Iterate through the list and find the network that has the matching SSID
	std::vector<PWLAN_AVAILABLE_NETWORK>::iterator netIter;
	for(netIter = pBssList.begin(); netIter != pBssList.end() ; netIter++){
		if (std::string((char*)((*netIter)->dot11Ssid.ucSSID)) == ssid ){
			output = *netIter;
			break;
		}
	}

	return output;
}



/*---------------------------------------------------------------
					GetPower
   Gets the power of the network
 ---------------------------------------------------------------*/
int		CWirelessPower::GetPower()
{
	//int iRSSI = 0;					// Received signal strength indication
	PWLAN_AVAILABLE_NETWORK wlan;	// handler to the network

	// Get a handler to the network
	wlan = GetNetworkW();
/*
	// Calculate the RSSI
	if (wlan->wlanSignalQuality == 0)
		iRSSI = -100;
	else if (wlan->wlanSignalQuality == 100)
		iRSSI = -50;
	else
		iRSSI = -100 + (wlan->wlanSignalQuality/2);

	return iRSSI;
*/

	return wlan->wlanSignalQuality;
}


/*---------------------------------------------------------------
					GUID2Str
   Gets the GUID of a network based on its handler in Windows
 ---------------------------------------------------------------*/

std::string CWirelessPower::GUID2Str(GUID ifaceGuid)
{
	// Variables
	int iRet;
	errno_t wctostr;
	size_t sizeGUID;

	WCHAR GuidString[39] = {0};
	char GuidChar[100];


	std::string outputString;

	// Call the API function that gets the name of the GUID as a WCHAR[]
    iRet = StringFromGUID2(ifaceGuid, (LPOLESTR) &GuidString, sizeof(GuidString)/sizeof(*GuidString));
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString,
            //     sizeof(GuidString)/sizeof(*GuidString));


	// translate from a WCHAR to string if no error happened
	if (iRet == 0){
		THROW_EXCEPTION("StringFromGUID2 failed\n");
	}	else {
		wctostr = wcstombs_s(&sizeGUID, GuidChar, 100, GuidString, 100 );
		if ( (wctostr == EINVAL) || (wctostr == ERANGE) ){
			THROW_EXCEPTION("wcstombs_s failed\n");
		} else {
			outputString = std::string(GuidChar);
		}
	}

	return outputString;
}
#endif //  end of Windows code