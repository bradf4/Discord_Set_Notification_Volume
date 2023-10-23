#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <Audioclient.h>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#include <Audiopolicy.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Ole32.lib")

float voiceVolume = -1.0f;
float notifVolume = -1.0f;

//To compile this it could look like this.
//cl /EHsc set_discord_volumes.cpp

void ChangeProcessVolume(DWORD processId, float volumeLevel) {
    HRESULT hr;

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create IMMDeviceEnumerator. Error code = 0x" << std::hex << hr << std::endl;
        CoUninitialize();
        return;
    }

    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint. Error code = 0x" << std::hex << hr << std::endl;
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    IAudioSessionManager2* pManager = NULL;
    hr = pDevice->Activate(
        __uuidof(IAudioSessionManager2),
        CLSCTX_ALL,
        NULL,
        (void**)&pManager
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to activate IAudioSessionManager2. Error code = 0x" << std::hex << hr << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    IAudioSessionEnumerator* pSessionEnum = NULL;
    hr = pManager->GetSessionEnumerator(&pSessionEnum);

    if (FAILED(hr)) {
        std::cerr << "Failed to get audio session enumerator. Error code = 0x" << std::hex << hr << std::endl;
        pManager->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    int sessionCount = 0;
    hr = pSessionEnum->GetCount(&sessionCount);

    if (FAILED(hr)) {
        std::cerr << "Failed to get session count. Error code = 0x" << std::hex << hr << std::endl;
        pSessionEnum->Release();
        pManager->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = NULL;
        hr = pSessionEnum->GetSession(i, &pSessionControl);

        if (SUCCEEDED(hr)) {
            IAudioSessionControl2* pSessionControl2 = NULL;
            hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);

            if (SUCCEEDED(hr)) {
                DWORD SessionProcessId = 0;
                hr = pSessionControl2->GetProcessId(&SessionProcessId);

                
                std::wcout << L"sessionProcessID: " << SessionProcessId << "\n"; 
                if (SUCCEEDED(hr) && SessionProcessId == processId) {
                    std::wcout << L"\n\nsessionProcessID: " << SessionProcessId << "\n\n";
                    ISimpleAudioVolume* pSimpleAudioVolume = NULL;
                    hr = pSessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleAudioVolume);

                    if (SUCCEEDED(hr)) {
                        hr = pSimpleAudioVolume->SetMasterVolume(volumeLevel, NULL);
                        if (FAILED(hr)) {
                            std::cerr << "Failed to set volume. Error code = 0x" << std::hex << hr << std::endl;
                        }
                        else{
                            std::wcout << "SET VOLUME " << volumeLevel << "\n";
                        }

                        pSimpleAudioVolume->Release();
                    }
                }

                pSessionControl2->Release();
            }

            pSessionControl->Release();
        }
    }

    // Cleanup
    pSessionEnum->Release();
    pManager->Release();
    pDevice->Release();
    pEnumerator->Release();
}

void PrintProcessInfo(IWbemClassObject* pProcess) {
    VARIANT vtProp;
    HRESULT hr;
    DWORD processId;
    //Process ID
    hr = pProcess->Get(L"ProcessId", 0, &vtProp, 0, 0);
    if (SUCCEEDED(hr)) {
        std::wcout << L"Process ID: " << vtProp.uintVal << std::endl;
        processId = vtProp.uintVal;
        VariantClear(&vtProp);
    } else {
        std::cerr << "Failed to get Process ID. Error code = 0x" << std::hex << hr << std::endl;
    }
    
    //Command Line
    hr = pProcess->Get(L"CommandLine", 0, &vtProp, 0, 0);
    if (SUCCEEDED(hr)) {
        std::wcout << L"Command Line: " << vtProp.bstrVal << std::endl;
        std::wcout << L"\n\n";

        //if statements 
        //float newVolumeLevel = 0.5f; // Set the desired volume level between 0.0 and 1.0
        std::wstring commandLine(vtProp.bstrVal);
        if (commandLine.find(L"com.squirrel.Discord.Discord") != std::wstring::npos) {
            // Handle Discord.exe with "com.squirrel.Discord.Discord" in CommandLine
            //VOICE VOLUME
            std::wcout << L"Found Voice Chat PID: " << processId;
            ChangeProcessVolume(processId, voiceVolume);
        }
        else if (commandLine.find(L"audio.mojom.AudioService") != std::wstring::npos) {
            // Handle Discord.exe with "audio.mojom.AudioService" in CommandLine
            //NOTIFICATION VOLUME
            std::wcout << L"Found Notification PID : " << processId;
            ChangeProcessVolume(processId, notifVolume);
        }



        VariantClear(&vtProp);
    } else {
        std::cerr << "Failed to get command line. Error code = 0x" << std::hex << hr << std::endl;
    }
}

void GetProcessesByExecutableName(IWbemServices* pSvc, const std::wstring& processName) {
    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hres = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t((L"SELECT * FROM Win32_Process WHERE Name = '" + processName + L"'").c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (FAILED(hres)) {
        std::cerr << "Query failed. Error code = 0x" << std::hex << hres << std::endl;
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 0)
            break;

        PrintProcessInfo(pclsObj);
        pclsObj->Release();
    }

    pEnumerator->Release();
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::wcout << "Usage: SetVolumeByPID <Application> <NotificationVolume> <VoiceVolume>" << "\n";
        return 1;
    }
    std::string appId = argv[1];
	std::wstring targetProcessName( appId.begin(), appId.end() );
    notifVolume = std::stof(argv[2]);
	voiceVolume = std::stof(argv[3]);

    HRESULT hres;

    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cerr << "Failed to initialize COM library. Error code = 0x" << std::hex << hres << std::endl;
        return 1;
    }

    // Set general COM security levels
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres)) {
        std::cerr << "Failed to initialize security. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return 1;
    }

    // Obtain the initial locator to WMI
    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );

    if (FAILED(hres)) {
        std::cerr << "Failed to create IWbemLocator object. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return 1;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),  // Object path of WMI namespace
        NULL,                      // User name
        NULL,                      // User password
        0,                         // Locale
        NULL,                      // Security flags
        0,                         // Authority
        0,                         // Context object
        &pSvc                      // IWbemServices proxy
    );

    if (FAILED(hres)) {
        std::cerr << "Could not connect. Error code = 0x" << std::hex << hres << std::endl;
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                       // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,          // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,           // RPC_C_AUTHZ_xxx
        NULL,                       // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,     // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE,// RPC_C_IMP_LEVEL_xxx
        NULL,                       // client identity
        EOAC_NONE                   // proxy capabilities
    );

    if (FAILED(hres)) {
        std::cerr << "Could not set proxy blanket. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }
    std::wcout << L"Searching for Discord.exe...\n\n";
    // Example: Get command lines of processes with a specific name
    //std::wstring targetProcessName = L"Discord.exe"; // Change to your desired process name
    GetProcessesByExecutableName(pSvc, targetProcessName);

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return 0;
}
