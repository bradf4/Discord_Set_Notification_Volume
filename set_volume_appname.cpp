#include <utility>
#include <iostream>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <wchar.h>
#include <psapi.h>
#include <string>


//To compile this it could look like this.
//g++ -o set_volume_appname.exe set_volume_appname.cpp -lole32 -lmmdevapi

// Function to get the process name from its ID
std::wstring GetProcessName(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        WCHAR processName[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, NULL, processName, MAX_PATH)) {
            CloseHandle(hProcess);

            // Extract the filename from the path
            WCHAR* processNameWithoutPath = wcsrchr(processName, L'\\');
            if (processNameWithoutPath) {
                return processNameWithoutPath + 1;
            } else {
                return processName;
            }
        }
        CloseHandle(hProcess);
    }
    return L"Unknown";
}

bool SetVolume(IAudioSessionControl* pSessionControl, float volume){
	if(pSessionControl == NULL)
		return false;
	
	HRESULT hr;
	ISimpleAudioVolume* pSimpleAudioVolume = nullptr;
	hr = pSessionControl->QueryInterface(
		__uuidof(ISimpleAudioVolume),
		(void**)&pSimpleAudioVolume
	);
	//code for changing one volume at application name
	if (SUCCEEDED(hr)) {
		hr = pSimpleAudioVolume->SetMasterVolume(volume, nullptr);
		if (SUCCEEDED(hr)) {
			std::wcout << "Set Volume" << std::endl;
			//std::wcout << L"Volume set successfully for process ID " << processId << std::endl;
		}
		pSimpleAudioVolume->Release();
	}
	return true;
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
        std::cerr << "Usage: SetVolumeByPID <Application> <Volume> <BigVolume>" << std::endl;
        return 1;
    }
	
	std::string appId = argv[1];
	std::wstring applicationId( appId.begin(), appId.end() );
    float volume = std::stof(argv[2]);
	float bigVolume = std::stof(argv[3]);
	//DWORD Pid = std::stoi(argv[x]);

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "CoInitialize failed: " << hr << std::endl;
        return 1;
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator
    );
    if (FAILED(hr)) {
        std::cerr << "CoCreateInstance failed: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        std::cerr << "GetDefaultAudioEndpoint failed: " << hr << std::endl;
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    IAudioSessionManager2* pSessionManager = NULL;
    hr = pDevice->Activate(
        __uuidof(IAudioSessionManager2),
        CLSCTX_ALL,
        NULL,
        (void**)&pSessionManager
    );
    if (FAILED(hr)) {
        std::cerr << "Activate failed: " << hr << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    IAudioSessionEnumerator* pSessionEnumerator = NULL;
    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) {
        std::cerr << "GetSessionEnumerator failed: " << hr << std::endl;
        pSessionManager->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    int sessionCount = 0;
    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) {
        std::cerr << "GetCount failed: " << hr << std::endl;
        pSessionEnumerator->Release();
        pSessionManager->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }
	
	
	std::pair<DWORD, IAudioSessionControl*> firstPair;
	bool found = false;
	bool foundBoth = false;
	bool writes = false;
	
	std::wcout << "Running For Loop\n";

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = NULL;
        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) {
            std::cerr << "GetSession failed: " << hr << std::endl;
            continue;
        }
		
		DWORD SessionProcessId = 0;

        // Temporary IAudioSessionControl2* pointer
        IAudioSessionControl2* pSessionControl2 = NULL;
        hr = pSessionControl->QueryInterface(
            __uuidof(IAudioSessionControl2),
            (void**)&pSessionControl2
        );

        if (SUCCEEDED(hr)) {
            // Get the session display name
            LPWSTR sessionDisplayName = NULL;
            hr = pSessionControl2->GetDisplayName(&sessionDisplayName);
            if (SUCCEEDED(hr) && sessionDisplayName && wcslen(sessionDisplayName) > 0) {
                std::wcout << L"Application: " << sessionDisplayName << std::endl;
                CoTaskMemFree(sessionDisplayName);
            } else {
                // Get the PID for an application will need for setting volume
                //DWORD SessionProcessId = 0;
                hr = pSessionControl2->GetProcessId(&SessionProcessId);
                if (SUCCEEDED(hr)) {
					std::wstring SessionAppName = GetProcessName(SessionProcessId);
                    std::wcout << L"Application: " << SessionAppName;
					if(SessionAppName.size() < 11)
						std::wcout << "\t";		//helps debugging format
					std::wcout << "\t| PID: " << SessionProcessId << "\t| ";
					
					// if(SessionProcessId == processId)  if you know the PID that you wish to change 
					// In the case of discord, or applications that use multiple processes that have volume control use SessionAppName
					if(SessionAppName == applicationId){
						std::pair<DWORD, IAudioSessionControl*> temp (SessionProcessId, pSessionControl);
						if(found)
							foundBoth = true;
						else{
							found = true;
							//store the first instance
							firstPair = temp;
						}
						/*
						ISimpleAudioVolume* pSimpleAudioVolume = nullptr;
						hr = pSessionControl->QueryInterface(
							__uuidof(ISimpleAudioVolume),
							(void**)&pSimpleAudioVolume
						);
						code for changing one volume at application name
						if (SUCCEEDED(hr)) {
							hr = pSimpleAudioVolume->SetMasterVolume(volume, nullptr);
							if (SUCCEEDED(hr)) {
								writes = true;
								//std::wcout << L"Volume set successfully for process ID " << processId << std::endl;
							}
							pSimpleAudioVolume->Release();
						}
						*/
					}
                }
            }

            // Further processing with pSessionControl2 (IAudioSessionControl2*)
            pSessionControl2->Release();
        }
		else{
            std::wcout << "Failed finding pSessionControl2\n";
        }

        ISimpleAudioVolume* pSimpleAudioVolume = NULL;
        hr = pSessionControl->QueryInterface(
            __uuidof(ISimpleAudioVolume),
            (void**)&pSimpleAudioVolume
        );
        if (FAILED(hr)) {
            std::cerr << "QueryInterface failed: " << hr << std::endl;
            pSessionControl->Release();
            continue;
        }

        float ReadVolume = 0.0f;
        hr = pSimpleAudioVolume->GetMasterVolume(&ReadVolume);
        if (SUCCEEDED(hr)) {
            std::cout << "Volume: " << ReadVolume << std::endl;
        }
		
		if(foundBoth){
			DWORD largerPID = 0;
			IAudioSessionControl* largerPTR = NULL;
			DWORD smallerPID = 0;
			IAudioSessionControl* smallerPTR = NULL;
			
			if(SessionProcessId > firstPair.first){
				largerPID = SessionProcessId;
				largerPTR = pSessionControl;
				smallerPID = firstPair.first;
				smallerPTR = firstPair.second;
			}
			else{
				largerPID = firstPair.first;
				largerPTR = firstPair.second;
				smallerPID = SessionProcessId;
				smallerPTR = pSessionControl;
			}
			
			//set smallerPID
			if(SetVolume(smallerPTR, bigVolume) && SetVolume(largerPTR, volume))
				writes = true;
			
			largerPTR->Release();
			smallerPTR->Release();

			foundBoth = false;
		}
		

        pSimpleAudioVolume->Release();
        pSessionControl->Release();
    }
	
	if (!found) {
        std::wcout << "No audio session found for application " << applicationId << std::endl;
    }
	
	if (!writes) {
        std::wcout << "No audio session writen for application " << applicationId << std::endl;
    }

    pSessionEnumerator->Release();
    pSessionManager->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;
}
