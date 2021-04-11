#include "../Headers/AmirHardwareInformation.h"
#include <stdio.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <cpuid.h>

//#pragma comment(lib, "iphlpapi")

using namespace std;

//==========================================================================================
string AmirGetSystemInfo::getCommandOutput(const char* cmd)
{
    FILE *fpipe;
    char buff[129];// = 0;
    size_t len;
    string result;
    if (0 == (fpipe = (FILE*)popen(cmd, "r"))) return "popen() failed.";
    else
    {
        while ((len = fread(buff, sizeof(char), 128, fpipe)))//((len = fread(buff, sizeof(char), 128, fpipe)))//!feof(pipe.get())
        {
            buff[len] = 0;
            result += buff;
        }
        pclose(fpipe);
    }
    return result;
}
//==========================================================================================
string AmirGetSystemInfo::getHDDSerial()
{
    AmirString as = getCommandOutput("wmic path win32_physicalmedia get SerialNumber");
    string result;
    as>>result;
    if (result == "SerialNumber") as>>result;
    return result;
}
//==========================================================================================
string AmirGetSystemInfo::getBiosSerial()
{
    AmirString as = getCommandOutput("WMIC BIOS GET SERIALNUMBER");
    string result;
    as>>result;
    if (result == "SerialNumber") as>>result;
    return result;
}
//==========================================================================================
string AmirGetSystemInfo::getMacAddress()
{
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    char mac_addr[18];
    string result;

    AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL) return "Error allocating memory needed to call GetAdaptersinfo";

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
        if (AdapterInfo == NULL) return "Error allocating memory needed to call GetAdaptersinfo";
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
    {
        // Contains pointer to current adapter info
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        do
        {
            // technically should look at pAdapterInfo->AddressLength
            //   and not assume it is 6.
            sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                    pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                    pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
            //printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
            // print them all, return the last one.
            // return mac_addr;
            result += mac_addr;
            result += '\n';
            pAdapterInfo = pAdapterInfo->Next;
        }
        while(pAdapterInfo);
    }
    free(AdapterInfo);
    return result;
}
//==========================================================================================
string AmirGetSystemInfo::getCpuID()
{
    string result;
    char tmp[34];
    unsigned int inp[4], z1=0;
    for(z1 = 0; z1<12; z1++)
    {
        __get_cpuid(z1, &inp[0], &inp[1], &inp[2], &inp[3]);
        sprintf(tmp, "%8x%8x%8x%8x\n", inp[0], inp[1], inp[2], inp[3]);//"%8x%8x%8x%8x\n"
        //tmp[33] = 0;//sprintf set it 0
        result += tmp;
    }
    return result;
}






