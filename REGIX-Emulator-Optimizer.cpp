// ==============================================================================
// Project: REGIX EMULATOR ENGINE v6.0 (Ultimate Performance & Ad Purge)
// Developer: Jahid Ekbal Mallick | CEO, REGIX
// Features: Zero Ads, Force Max PC Resources, V-Sync Unlock, Hard Debloat
// ==============================================================================

#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <array>
#include <filesystem>
#include <algorithm>
#include <set>
#include <fstream>

namespace fs = std::filesystem;
#pragma comment(lib, "psapi.lib")

// Win32 Native Kernel API
typedef NTSTATUS(NTAPI* pfnNtSetInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
    );

void SetupConsoleEnvironment() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

std::string ExecCmd(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string GetAdbPath() {
    std::vector<std::string> searchPaths = {
        "C:\\Program Files\\BlueStacks_nxt\\HD-Adb.exe",
        "C:\\Program Files\\BlueStacks_msi5\\HD-Adb.exe",
        "C:\\Program Files\\BlueStacks\\HD-Adb.exe",
        "C:\\Program Files\\MSI App Player\\HD-Adb.exe",
        "C:\\Program Files (x86)\\BlueStacks\\HD-Adb.exe",
        "C:\\Program Files (x86)\\MSI App Player\\HD-Adb.exe"
    };
    for (const auto& path : searchPaths) {
        if (fs::exists(path)) return "\"" + path + "\"";
    }
    return "adb";
}

std::vector<DWORD> GetProcessIDsByName(const std::wstring& processName) {
    std::vector<DWORD> pids;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    pids.push_back(pe32.th32ProcessID);
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return pids;
}

std::string ConnectADB(const std::string& adbPath) {
    ExecCmd(adbPath + " start-server");
    std::vector<int> ports = { 5555, 5556, 5557, 5565, 5575 };
    for (int port : ports) {
        std::string target = "127.0.0.1:" + std::to_string(port);
        std::string res = ExecCmd(adbPath + " connect " + target);
        if (res.find("connected") != std::string::npos) {
            return target;
        }
    }
    return "";
}

void RenderBrandingBanner(const std::string& target) {
    std::cout << "\033[38;2;0;255;204m┌──────────────────────────────────────────────────────────────┐\033[0m\n";
    std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;0;128m██████╗ \033[38;2;255;100;0m███████╗\033[38;2;255;230;0m██████╗ \033[38;2;0;255;100m██╗██╗  ██╗  \033[38;2;0;200;255mENGINE v6.0   \033[38;2;0;255;204m│\033[0m\n";
    std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;0;128m██╔══██╗\033[38;2;255;100;0m██╔════╝\033[38;2;255;230;0m██╔══██╗\033[38;2;0;255;100m██║╚██╗██╔╝  \033[38;2;255;255;255mREGIX STUDIO  \033[38;2;0;255;204m│\033[0m\n";
    std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;0;128m██████╔╝\033[38;2;255;100;0m█████╗  \033[38;2;255;230;0m██████╔╝\033[38;2;0;255;100m██║ ╚███╔╝   \033[38;2;180;180;180mDEVELOPER     \033[38;2;0;255;204m│\033[0m\n";
    std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;0;128m██╔══██╗\033[38;2;255;100;0m██╔══╝  \033[38;2;255;230;0m██╔══██╗\033[38;2;0;255;100m██║ ██╔██╗   \033[38;2;255;200;0mJahid Ekbal   \033[38;2;0;255;204m│\033[0m\n";
    std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;0;128m██║  ██║\033[38;2;255;100;0m███████╗\033[38;2;255;230;0m██████╔╝\033[38;2;0;255;100m██║██╔╝ ██╗  \033[38;2;100;255;100m[CEO & Lead]  \033[38;2;0;255;204m│\033[0m\n";
    std::cout << "\033[38;2;0;255;204m├──────────────────────────────────────────────────────────────┤\033[0m\n";
    if (!target.empty()) {
        std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;255;255mTarget Status : \033[38;2;50;255;50mCONNECTED (" << target << ")\033[38;2;0;255;204m                      │\033[0m\n";
    }
    else {
        std::cout << "\033[38;2;0;255;204m│  \033[38;2;255;255;255mTarget Status : \033[38;2;255;50;50mDISCONNECTED (Start Emulator)\033[38;2;0;255;204m             │\033[0m\n";
    }
    std::cout << "\033[38;2;0;255;204m└──────────────────────────────────────────────────────────────┘\033[0m\n\n";
}

// Complete Ad Block & Config Modifier
void DisableAllAdsAndPromotions(const std::string& adbPath, const std::string& target) {
    std::cout << "\033[38;2;255;230;0m[*] Purging BlueStacks Home Screen Ads & Config Promos...\033[0m\n";
    std::vector<std::string> confPaths = {
        "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf",
        "C:\\ProgramData\\MSI App Player_nxt\\bluestacks.conf",
        "C:\\ProgramData\\BlueStacks\\bluestacks.conf",
        "C:\\ProgramData\\MSI App Player\\bluestacks.conf"
    };

    for (const auto& conf : confPaths) {
        if (fs::exists(conf)) {
            std::ifstream inFile(conf);
            std::string content, line;
            while (std::getline(inFile, line)) {
                if (line.find("bst.feature.ads=") != std::string::npos) line = "bst.feature.ads=\"0\"";
                if (line.find("enable_ads=") != std::string::npos) line = "enable_ads=\"0\"";
                if (line.find("show_ads_on_side=") != std::string::npos) line = "show_ads_on_side=\"0\"";
                if (line.find("game_center_enabled=") != std::string::npos) line = "game_center_enabled=\"0\"";
                if (line.find("enable_promotion=") != std::string::npos) line = "enable_promotion=\"0\"";
                content += line + "\n";
            }
            inFile.close();
            std::ofstream outFile(conf);
            outFile << content;
            outFile.close();
        }
    }

    if (!target.empty()) {
        // Clear Launcher Cache to kill "Popular Games" section instantly
        ExecCmd(adbPath + " -s " + target + " shell pm clear com.uncube.launcher3");
        ExecCmd(adbPath + " -s " + target + " shell pm clear com.bluestacks.nxt.launcher");
        ExecCmd(adbPath + " -s " + target + " shell settings put global show_ads 0");
        std::cout << "\033[38;2;50;255;50m  + Home Screen Ads & Game Center Cache Wiped!\033[0m\n";
    }
}

// Master Resource & Smoothness Engine
void ExecuteAutoTune(const std::string& adbPath, const std::string& target) {
    DisableAllAdsAndPromotions(adbPath, target);

    std::cout << "\n\033[38;2;255;230;0m[*] [1/4] Forcing Max PC Resources (Realtime CPU & Power Throttling OFF)...\033[0m\n";
    std::vector<DWORD> pids = GetProcessIDsByName(L"HD-Player.exe");
    if (!pids.empty()) {
        for (DWORD pid : pids) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (hProcess) {
                // Realtime CPU Priority
                SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);

                // High I/O Priority
                HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
                if (hNtdll) {
                    pfnNtSetInformationProcess NtSetInfo = (pfnNtSetInformationProcess)GetProcAddress(hNtdll, "NtSetInformationProcess");
                    if (NtSetInfo) {
                        ULONG ioPriority = 3;
                        NtSetInfo(hProcess, 33, &ioPriority, sizeof(ioPriority));
                    }
                }
                EmptyWorkingSet(hProcess);
                CloseHandle(hProcess);
            }
            std::cout << "\033[38;2;50;255;50m  + PID " << pid << ": Forced Realtime Resource Allocation & Zero Delay\033[0m\n";
        }
    }

    if (!target.empty()) {
        std::cout << "\n\033[38;2;255;230;0m[*] [2/4] Completely Disabling GPS & Location Services...\033[0m\n";
        ExecCmd(adbPath + " -s " + target + " shell settings put secure location_mode 0");
        ExecCmd(adbPath + " -s " + target + " shell settings put secure location_providers_allowed -gps");
        ExecCmd(adbPath + " -s " + target + " shell settings put secure location_providers_allowed -network");
        ExecCmd(adbPath + " -s " + target + " shell pm disable-user --user 0 com.google.android.location");
        ExecCmd(adbPath + " -s " + target + " shell pm disable-user --user 0 com.android.location.fused");
        std::cout << "\033[38;2;50;255;50m  + Location Services & GPS Background Tracking Killed\033[0m\n";

        std::cout << "\n\033[38;2;255;230;0m[*] [3/4] Unlocking Max FPS & Disabling V-Sync for Ultimate Smoothness...\033[0m\n";
        ExecCmd(adbPath + " -s " + target + " shell setprop debug.gr.swapinterval 0"); // V-Sync OFF
        ExecCmd(adbPath + " -s " + target + " shell setprop persist.sys.NV_FPSLIMIT 0");
        ExecCmd(adbPath + " -s " + target + " shell setprop debug.egl.hw 1");
        ExecCmd(adbPath + " -s " + target + " shell setprop debug.sf.hw 1");
        ExecCmd(adbPath + " -s " + target + " shell settings put global window_animation_scale 0.0");
        ExecCmd(adbPath + " -s " + target + " shell settings put global transition_animation_scale 0.0");
        ExecCmd(adbPath + " -s " + target + " shell settings put global animator_duration_scale 0.0");
        ExecCmd(adbPath + " -s " + target + " shell settings put system pointer_speed 7");
        std::cout << "\033[38;2;50;255;50m  + V-Sync Disabled & Maximum Smoothness Locked\033[0m\n";

        std::cout << "\n\033[38;2;255;230;0m[*] [4/4] Fixing Free Fire Freeze, Heap & Buffer Delay...\033[0m\n";
        ExecCmd(adbPath + " -s " + target + " shell logcat -G 16M");
        ExecCmd(adbPath + " -s " + target + " shell setprop dalvik.vm.heapsize 1024m");
    }

    std::cout << "\n\033[38;2;50;255;50m==============================================================\033[0m\n";
    std::cout << "\033[38;2;50;255;50m  [+] FULL MAXIMUM RESOURCE OPTIMIZATION COMPLETE!            \033[0m\n";
    std::cout << "\033[38;2;50;255;50m==============================================================\033[0m\n";
}

// Absolute Hard Package Removal
void ExecuteAppRemoval(const std::string& adbPath, const std::string& target, const std::string& pkg) {
    std::cout << "\033[38;2;255;230;0m[*] Hard Removing Package: \033[0m" << pkg << "\n";
    ExecCmd(adbPath + " -s " + target + " shell pm clear " + pkg);
    ExecCmd(adbPath + " -s " + target + " shell pm disable-user --user 0 " + pkg);
    ExecCmd(adbPath + " -s " + target + " shell pm hide " + pkg);
    ExecCmd(adbPath + " -s " + target + " shell pm uninstall -k --user 0 " + pkg);

    ExecCmd(adbPath + " -s " + target + " shell rm -rf /sdcard/Android/data/" + pkg);
    ExecCmd(adbPath + " -s " + target + " shell rm -rf /sdcard/Android/obb/" + pkg);
    ExecCmd(adbPath + " -s " + target + " shell rm -rf /data/data/" + pkg);
    std::cout << "\033[38;2;50;255;50m  [+] Package Destroyed: \033[0m" << pkg << "\n";
}

// Absolute Hard Debloater (Purges Store, Console Mode, Camera, Files, Gallery, Contacts, Ads)
void AbsoluteDeepDebloat(const std::string& adbPath, const std::string& target) {
    if (target.empty()) {
        std::cout << "\033[38;2;255;50;50m[-] Emulator disconnected.\033[0m\n";
        return;
    }

    std::cout << "\n\033[38;2;255;0;128m[*] Executing Hard Debloater (Purging Store, Console Mode, GPS, Ads, Camera, Files)...\033[0m\n";

    std::vector<std::string> targetBloatware = {
        "com.bluestacks.appmart",           // BlueStacks Store
        "com.uncube.appmart",              // Store
        "com.uncube.store",                // Store
        "com.android.vending",              // Play Store
        "com.bluestacks.gamepad",           // Console Mode
        "com.uncube.gamepad",              // Console Mode
        "com.bluestacks.gamepad.input",     // Console Input
        "com.android.camera",               // Camera
        "com.android.camera2",              // Camera 2
        "com.android.contacts",            // Contacts
        "com.android.gallery3d",            // Gallery
        "com.android.documentsui",          // Files UI
        "com.android.filemanager",          // File Manager
        "com.google.android.location",      // Location Services
        "com.android.location.fused",       // GPS Service
        "gg.now.billing.service",           // Ads
        "gg.now.billing.interceptor",       // Ads
        "gg.now.accounts",                  // Ads
        "io.wallpaperengine.weclient"       // Wallpaper Engine
    };

    int count = 0;
    for (const auto& pkg : targetBloatware) {
        ExecuteAppRemoval(adbPath, target, pkg);
        count++;
    }

    // Force Reset Launcher Cache to Clear Ad Widgets
    ExecCmd(adbPath + " -s " + target + " shell pm clear com.uncube.launcher3");
    ExecCmd(adbPath + " -s " + target + " shell pm clear com.bluestacks.nxt.launcher");

    std::cout << "\n\033[38;2;50;255;50m[+] Hard Debloat Complete! Purged Store, Console Mode, GPS, and Ads Cache.\033[0m\n";
}

int main() {
    SetupConsoleEnvironment();
    std::string adbPath = GetAdbPath();

    while (true) {
        system("cls");
        std::string target = ConnectADB(adbPath);
        RenderBrandingBanner(target);

        std::cout << "  \033[38;2;255;230;0m[1] Single-Click Auto-Tune All (Realtime PC Power + Zero V-Sync + Zero Ads)\033[0m\n";
        std::cout << "  \033[38;2;255;0;128m[2] Hard Debloat Engine (Purge Store, Console Mode, Location/GPS, Camera & Ads)\033[0m\n";
        std::cout << "  \033[38;2;150;150;150m[3] Exit REGIX Engine\033[0m\n\n";
        std::cout << " Select Option [1-3]: ";

        int choice = 0;
        std::cin >> choice;

        if (choice == 1) {
            ExecuteAutoTune(adbPath, target);
            std::cout << "\nPress Enter to return...";
            std::cin.ignore();
            std::cin.get();
        }
        else if (choice == 2) {
            AbsoluteDeepDebloat(adbPath, target);
            std::cout << "\nPress Enter to return...";
            std::cin.ignore();
            std::cin.get();
        }
        else if (choice == 3) {
            break;
        }
    }
    return 0;
}