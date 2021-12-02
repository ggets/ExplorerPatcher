#ifdef _WIN64
#include "hooking.h"
#endif
#include <initguid.h>
#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib") // required by funchook
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <windowsx.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include <Shlobj_core.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <roapi.h>
#include <ShellScalingApi.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#include <tlhelp32.h>
#include <UIAutomationClient.h>
#ifdef _WIN64
#include <valinet/pdb/pdb.h>
#endif
#if defined(DEBUG) | defined(_DEBUG)
#define _LIBVALINET_DEBUG_HOOKING_IATPATCH
#endif
#include <valinet/hooking/iatpatch.h>
#include <valinet/utility/memmem.h>

#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

#define CHECKFOREGROUNDELAPSED_TIMEOUT 300
#define POPUPMENU_SAFETOREMOVE_TIMEOUT 300
#define POPUPMENU_BLUETOOTH_TIMEOUT 700
#define POPUPMENU_PNIDUI_TIMEOUT 300
#define POPUPMENU_SNDVOLSSO_TIMEOUT 300
#define POPUPMENU_EX_ELAPSED 300

BOOL bIsExplorerProcess = FALSE;
BOOL bInstanced = FALSE;
HWND archivehWnd;
DWORD bOldTaskbar = TRUE;
DWORD bAllocConsole = FALSE;
DWORD bHideExplorerSearchBar = FALSE;
DWORD bMicaEffectOnTitlebar = FALSE;
DWORD bHideControlCenterButton = FALSE;
DWORD bFlyoutMenus = TRUE;
DWORD bCenterMenus = TRUE;
DWORD bSkinMenus = TRUE;
DWORD bSkinIcons = TRUE;
DWORD bReplaceNetwork = FALSE;
DWORD dwExplorerReadyDelay = 0;
DWORD bEnableArchivePlugin = FALSE;
DWORD bMonitorOverride = TRUE;
DWORD bOpenAtLogon = FALSE;
DWORD bClockFlyoutOnWinC = FALSE;
DWORD bDisableImmersiveContextMenu = FALSE;
DWORD bClassicThemeMitigations = FALSE;
DWORD bHookStartMenu = TRUE;
DWORD bPropertiesInWinX = FALSE;
DWORD bNoMenuAccelerator = FALSE;
DWORD bTaskbarMonitorOverride = 0;
DWORD dwIMEStyle = 0;
DWORD dwTaskbarAl = 1;
DWORD bShowUpdateToast = FALSE;
DWORD bToolbarSeparators = FALSE;
DWORD bTaskbarAutohideOnDoubleClick = FALSE;
DWORD dwOrbStyle = 0;
DWORD bEnableSymbolDownload = TRUE;
HMODULE hModule = NULL;
HANDLE hDelayedInjectionThread = NULL;
HANDLE hIsWinXShown = NULL;
HANDLE hWinXThread = NULL;
HANDLE hSwsSettingsChanged = NULL;
HANDLE hSwsOpacityMaybeChanged = NULL;
HANDLE hWin11AltTabInitialized = NULL;
BYTE* lpShouldDisplayCCButton = NULL;
HMONITOR hMonitorList[30];
DWORD dwMonitorCount = 0;
int Code = 0;
HRESULT InjectStartFromExplorer();

#define ORB_STYLE_WINDOWS10 0
#define ORB_STYLE_WINDOWS11 1
#define ORB_WINDOWS11_SEPARATOR 1

void* P_Icon_Light_Search = NULL;
DWORD S_Icon_Light_Search = 0;

void* P_Icon_Light_TaskView = NULL;
DWORD S_Icon_Light_TaskView = 0;

void* P_Icon_Light_Widgets = NULL;
DWORD S_Icon_Light_Widgets = 0;

void* P_Icon_Dark_Search = NULL;
DWORD S_Icon_Dark_Search = 0;

void* P_Icon_Dark_TaskView = NULL;
DWORD S_Icon_Dark_TaskView = 0;

void* P_Icon_Dark_Widgets = NULL;
DWORD S_Icon_Dark_Widgets = 0;



#include "utility.h"
#include "resource.h"
#ifdef USE_PRIVATE_INTERFACES
#include "ep_private.h"
#endif
#ifdef _WIN64
#include "symbols.h"
#include "dxgi_imp.h"
#include "ArchiveMenu.h"
#include "StartupSound.h"
#include "StartMenu.h"
#include "GUI.h"
#include "TaskbarCenter.h"
#include "../libs/sws/SimpleWindowSwitcher/sws_WindowSwitcher.h"
#endif
#include "SettingsMonitor.h"
#include "HideExplorerSearchBar.h"
#include "ImmersiveFlyouts.h"
#include "updates.h"
DWORD dwUpdatePolicy = UPDATE_POLICY_DEFAULT;

HRESULT WINAPI _DllRegisterServer();
HRESULT WINAPI _DllUnregisterServer();
HRESULT WINAPI _DllCanUnloadNow();
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
);


#pragma region "Updates"
#ifdef _WIN64
DWORD CheckForUpdatesThread(LPVOID unused)
{
    HRESULT hr = S_OK;
    HSTRING_HEADER header_AppIdHString;
    HSTRING AppIdHString = NULL;
    HSTRING_HEADER header_ToastNotificationManagerHString;
    HSTRING ToastNotificationManagerHString = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationManagerStatics* toastStatics = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier = NULL;
    HSTRING_HEADER header_ToastNotificationHString;
    HSTRING ToastNotificationHString = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification* toast = NULL;

    while (TRUE)
    {
        HWND hShell_TrayWnd = FindWindowExW(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hShell_TrayWnd)
        {
            Sleep(5000);
            break;
        }
        Sleep(100);
    }
    printf("[Updates] Starting daemon.\n");

    if (SUCCEEDED(hr))
    {
        hr = RoInitialize(RO_INIT_MULTITHREADED);
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            APPID,
            (UINT32)(sizeof(APPID) / sizeof(TCHAR) - 1),
            &header_AppIdHString,
            &AppIdHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            RuntimeClass_Windows_UI_Notifications_ToastNotificationManager,
            (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager) / sizeof(wchar_t) - 1),
            &header_ToastNotificationManagerHString,
            &ToastNotificationManagerHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            ToastNotificationManagerHString,
            &UIID_IToastNotificationManagerStatics,
            (LPVOID*)&toastStatics
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = toastStatics->lpVtbl->CreateToastNotifierWithId(
            toastStatics,
            AppIdHString,
            &notifier
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            RuntimeClass_Windows_UI_Notifications_ToastNotification,
            (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotification) / sizeof(wchar_t) - 1),
            &header_ToastNotificationHString,
            &ToastNotificationHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            ToastNotificationHString,
            &UIID_IToastNotificationFactory,
            (LPVOID*)&notifFactory
        );
    }

    HANDLE hEvents[2];
    hEvents[0] = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_CheckForUpdates_" _T(EP_CLSID));
    hEvents[1] = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_InstallUpdates_" _T(EP_CLSID));
    if (hEvents[0] && hEvents[1])
    {
        if (dwUpdatePolicy != UPDATE_POLICY_MANUAL)
        {
            InstallUpdatesIfAvailable(hModule, bShowUpdateToast, notifier, notifFactory, &toast, UPDATES_OP_DEFAULT, bAllocConsole, dwUpdatePolicy);
        }
        DWORD dwRet = 0;
        while (TRUE)
        {
            switch (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
            {
                InstallUpdatesIfAvailable(hModule, bShowUpdateToast, notifier, notifFactory, &toast, UPDATES_OP_CHECK, bAllocConsole, dwUpdatePolicy);
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                InstallUpdatesIfAvailable(hModule, bShowUpdateToast, notifier, notifFactory, &toast, UPDATES_OP_INSTALL, bAllocConsole, dwUpdatePolicy);
                break;
            }
            default:
            {
                break;
            }
            }
        }
        CloseHandle(hEvents[0]);
        CloseHandle(hEvents[1]);
    }

    if (toast)
    {
        toast->lpVtbl->Release(toast);
    }
    if (notifFactory)
    {
        notifFactory->lpVtbl->Release(notifFactory);
    }
    if (ToastNotificationHString)
    {
        WindowsDeleteString(ToastNotificationHString);
    }
    if (notifier)
    {
        notifier->lpVtbl->Release(notifier);
    }
    if (toastStatics)
    {
        toastStatics->lpVtbl->Release(toastStatics);
    }
    if (ToastNotificationManagerHString)
    {
        WindowsDeleteString(ToastNotificationManagerHString);
    }
    if (AppIdHString)
    {
        WindowsDeleteString(AppIdHString);
    }
}
#endif
#pragma endregion


#pragma region "Generics"
#ifdef _WIN64
HWND GetMonitorInfoFromPointForTaskbarFlyoutActivation(POINT ptCursor, DWORD dwFlags, LPMONITORINFO lpMi)
{
    HMONITOR hMonitor = MonitorFromPoint(ptCursor, dwFlags);
    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowEx(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (MonitorFromWindow(hWnd, dwFlags) == hMonitor)
        {
            if (lpMi)
            {
                GetMonitorInfo(
                    MonitorFromPoint(
                        ptCursor,
                        dwFlags
                    ),
                    lpMi
                );
            }
            break;
        }
    } while (hWnd);
    if (!hWnd)
    {
        hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        //ptCursor.x = 0;
        //ptCursor.y = 0;
        if (lpMi)
        {
            GetMonitorInfo(
                MonitorFromWindow(
                    hWnd,
                    dwFlags
                ),
                lpMi
            );
        }
    }
    return hWnd;
}

POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight, BOOL bAdjust)
{
    if (lpBottom) *lpBottom = FALSE;
    if (lpRight) *lpRight = FALSE;
    POINT point;
    point.x = 0;
    point.y = 0;
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        &mi
    );
    if (hWnd)
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        if (rc.left - mi.rcMonitor.left <= 0)
        {
            if (bUseRcWork)
            {
                point.x = mi.rcWork.left;
            }
            else
            {
                point.x = mi.rcMonitor.left;
            }
            if (bAdjust)
            {
                point.x++;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
        else
        {
            if (lpRight) *lpRight = TRUE;
            if (bUseRcWork)
            {
                point.x = mi.rcWork.right;
            }
            else
            {
                point.x = mi.rcMonitor.right;
            }
            if (bAdjust)
            {
                point.x--;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
    }
    return point;
}

BOOL TerminateShellExperienceHost()
{
    BOOL bRet = FALSE;
    WCHAR wszKnownPath[MAX_PATH];
    GetWindowsDirectoryW(wszKnownPath, MAX_PATH);
    wcscat_s(wszKnownPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe");
    HANDLE hSnapshot = NULL;
    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(PROCESSENTRY32));
    pe32.dwSize = sizeof(PROCESSENTRY32);
    hSnapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );
    if (Process32First(hSnapshot, &pe32) == TRUE)
    {
        do
        {
            if (!wcscmp(pe32.szExeFile, TEXT("ShellExperienceHost.exe")))
            {
                HANDLE hProcess = OpenProcess(
                    PROCESS_QUERY_LIMITED_INFORMATION |
                    PROCESS_TERMINATE,
                    FALSE,
                    pe32.th32ProcessID
                );
                if (hProcess)
                {
                    TCHAR wszProcessPath[MAX_PATH];
                    DWORD dwLength = MAX_PATH;
                    QueryFullProcessImageNameW(
                        hProcess,
                        0,
                        wszProcessPath,
                        &dwLength
                    );
                    if (!_wcsicmp(wszProcessPath, wszKnownPath))
                    {
                        TerminateProcess(hProcess, 0);
                        bRet = TRUE;
                    }
                    CloseHandle(hProcess);
                    hProcess = NULL;
                }
            }
        } while (Process32Next(hSnapshot, &pe32) == TRUE);
        if (hSnapshot)
        {
            CloseHandle(hSnapshot);
        }
    }
    return bRet;
}

long long elapsedCheckForeground = 0;
HANDLE hCheckForegroundThread = NULL;
DWORD CheckForegroundThread(DWORD dwMode)
{
    printf("Started \"Check foreground window\" thread.\n");
    UINT i = 0;
    while (TRUE)
    {
        wchar_t text[200];
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (!wcscmp(text, L"Windows.UI.Core.CoreWindow"))
        {
            break;
        }
        i++;
        if (i >= 15) break;
        Sleep(100);
    }
    while (TRUE)
    {
        wchar_t text[200];
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (wcscmp(text, L"Windows.UI.Core.CoreWindow"))
        {
            break;
        }
        Sleep(100);
    }
    elapsedCheckForeground = milliseconds_now();
    if (!dwMode)
    {
        RegDeleteKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH));
        TerminateShellExperienceHost();
        Sleep(100);
    }
    printf("Ended \"Check foreground window\" thread.\n");
    return 0;
}

void LaunchNetworkTargets(DWORD dwTarget)
{
    // very helpful: https://www.tenforums.com/tutorials/3123-clsid-key-guid-shortcuts-list-windows-10-a.html
    if (!dwTarget)
    {
        InvokeFlyout(INVOKE_FLYOUT_SHOW, INVOKE_FLYOUT_NETWORK);
    }
    else if (dwTarget == 6)
    {
        InvokeActionCenter();
        return 0;
        // ShellExecuteW(
        //     NULL,
        //     L"open",
        //     L"ms-actioncenter:controlcenter/&showFooter=true",
        //     NULL,
        //     NULL,
        //     SW_SHOWNORMAL
        // );
    }
    else if (dwTarget == 5)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"ms-availablenetworks:",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 1)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"ms-settings:network",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 2)
    {
        HMODULE hVan = LoadLibraryW(L"van.dll");
        if (hVan)
        {
            long(*ShowVAN)(BOOL, BOOL, void*) = GetProcAddress(hVan, "ShowVAN");
            if (ShowVAN)
            {
                ShowVAN(0, 0, 0);
            }
            FreeLibrary(hVan);
        }
    }
    else if (dwTarget == 3)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"shell:::{8E908FC9-BECC-40f6-915B-F4CA0E70D03D}",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 4)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"shell:::{7007ACC7-3202-11D1-AAD2-00805FC1270E}",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
}
#endif
#pragma endregion


#pragma region "Toggle shell features"
BOOL CALLBACK ToggleImmersiveCallback(HWND hWnd, LPARAM lParam)
{
    WORD ClassWord;

    ClassWord = GetClassWord(hWnd, GCW_ATOM);
    if (ClassWord == RegisterWindowMessageW(L"WorkerW"))
    {
        PostMessageW(hWnd, WM_HOTKEY, lParam, 0);
    }

    return TRUE;
}

void ToggleHelp()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 505, 0);
}

void ToggleRunDialog()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 502, MAKELPARAM(MOD_WIN, 0x52));
}

void ToggleSystemProperties()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 512, 0);
}

void FocusSystray()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 514, 0);
}

void TriggerAeroShake()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 515, 0);
}

void PeekDesktop()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 516, 0);
}

void ToggleEmojiPanel()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 579, 0);
}

void ShowDictationPanel()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 577, 0);
}

void ToggleClipboardViewer()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 578, 0);
}

void ToggleSearch()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 507, MAKELPARAM(MOD_WIN, 0x53));
}

void ToggleTaskView()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 11);
}

void ToggleWidgetsPanel()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 0x66);
}

void ToggleMainClockFlyout()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 0x6B);
}

void ToggleNotificationsFlyout()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 591, 0);
}

void ToggleActionCenter()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 500, MAKELPARAM(MOD_WIN, 0x41));
}
#pragma endregion


#pragma region "twinui.pcshell.dll hooks"
#ifdef _WIN64
#define LAUNCHERTIP_CLASS_NAME L"LauncherTipWnd"
static INT64(*winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc)(
    void* _this,
    INT64 a2,
    INT a3
    ) = NULL;
static INT64(*CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)(
    void* _this,
    POINT* pt
    ) = NULL;
static void(*CLauncherTipContextMenu_ExecuteCommandFunc)(
    void* _this,
    int a2
    ) = NULL;
static void(*CLauncherTipContextMenu_ExecuteShutdownCommandFunc)(
    void* _this,
    void* a2
    ) = NULL;
static INT64(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)(
    HMENU h1,
    HMENU h2,
    HWND a3,
    unsigned int a4,
    void* data
    ) = NULL;
static void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)(
    HMENU _this,
    HMENU hWnd,
    HWND a3
    ) = NULL;
static INT64(*CLauncherTipContextMenu_GetMenuItemsAsyncFunc)(
    void* _this,
    void* rect,
    void** iunk
    ) = NULL;
static INT64(*CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc)(
    HWND hWnd,
    int a2,
    HWND a3,
    int a4,
    BOOL* a5
    ) = NULL;

LRESULT CALLBACK CLauncherTipContextMenu_WndProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    LRESULT result;

    if (hWnd == archivehWnd && !ArchiveMenuWndProc(
        hWnd, 
        uMsg, 
        wParam, 
        lParam,
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc,
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc
    ))
    {
        return 0;
    }

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCs = lParam;
        if (pCs->lpCreateParams)
        {
            *((HWND*)((char*)pCs->lpCreateParams + 0x78)) = hWnd;
            SetWindowLongPtr(
                hWnd, 
                GWLP_USERDATA,
                pCs->lpCreateParams
            );
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
        else
        {
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
            //result = 0;
        }
    }
    else
    {
        void* _this = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        BOOL v12 = FALSE;
        if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc(
                hWnd,
                uMsg,
                wParam,
                lParam,
                &v12
            ))
        {
            result = 0;
        }
        else
        {
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
        if (_this)
        {
            if (uMsg == WM_NCDESTROY)
            {
                SetWindowLongPtrW(
                    hWnd, 
                    GWLP_USERDATA,
                    0
                );
                *((HWND*)((char*)_this + 0x78)) = 0;
            }
        }
    }
    return result;
}

typedef struct
{
    void* _this;
    POINT point;
    IUnknown* iunk;
    BOOL bShouldCenterWinXHorizontally;
} ShowLauncherTipContextMenuParameters;
HWND hWinXWnd;
DWORD ShowLauncherTipContextMenu(
    ShowLauncherTipContextMenuParameters* params
)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CLauncherTipContextMenu_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = LAUNCHERTIP_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hWinXWnd = CreateWindowInBand(
        0,
        LAUNCHERTIP_CLASS_NAME,
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        (char*)params->_this - 0x58,
        7
    );
    // DO NOT USE ShowWindow here; it breaks the window order
    // and renders the desktop toggle unusable; but leave
    // SetForegroundWindow as is so that the menu gets dismissed
    // when the user clicks outside it
    // 
    // ShowWindow(hWinXWnd, SW_SHOW);
    SetForegroundWindow(hWinXWnd);

    while (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        Sleep(1);
    }
    if (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        goto finalize;
    }
   
    TCHAR buffer[260];
    LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
    if (!bNoMenuAccelerator)
    {
        buffer[0] = L'&';
    }
    wchar_t* p = wcschr(buffer, L'(');
    if (p)
    {
        p--;
        if (p == L' ')
        {
            *p = 0;
        }
        else
        {
            p++;
            *p = 0;
        }
    }

    BOOL bCreatedMenu = FALSE;
    MENUITEMINFOW menuInfo;
    ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
    menuInfo.cbSize = sizeof(MENUITEMINFOW);
    menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
    menuInfo.wID = 3999;
    menuInfo.dwItemData = 0;
    menuInfo.fType = MFT_STRING;
    menuInfo.dwTypeData = buffer;
    menuInfo.cch = wcslen(buffer);
    if (bPropertiesInWinX)
    {
        InsertMenuItemW(
            *((HMENU*)((char*)params->_this + 0xe8)),
            GetMenuItemCount(*((HMENU*)((char*)params->_this + 0xe8))) - 1,
            TRUE,
            &menuInfo
        );
        bCreatedMenu = TRUE;
    }

    INT64* unknown_array = NULL;
    if (bSkinMenus)
    {
        unknown_array = calloc(4, sizeof(INT64));
        if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
        {
            ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                *((HMENU*)((char*)params->_this + 0xe8)),
                hWinXWnd,
                &(params->point),
                0xc,
                unknown_array
            );
        }
    }

    BOOL res = TrackPopupMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        TPM_RETURNCMD | TPM_RIGHTBUTTON | (params->bShouldCenterWinXHorizontally ? TPM_CENTERALIGN : 0),
        params->point.x,
        params->point.y,
        0,
        hWinXWnd,
        0
    );

    if (bSkinMenus)
    {
        if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
        {
            ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                *((HMENU*)((char*)params->_this + 0xe8)),
                hWinXWnd,
                &(params->point)
            );
        }
        free(unknown_array);
    }

    if (bCreatedMenu)
    {
        RemoveMenu(
            *((HMENU*)((char*)params->_this + 0xe8)),
            3999,
            MF_BYCOMMAND
        );
    }

    if (res > 0)
    {
        if (bCreatedMenu && res == 3999)
        {
            LaunchPropertiesGUI(hModule);
        }
        else if (res < 4000)
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xa8 - 0x58)) + (INT64)res * 8 - 8);
            if (CLauncherTipContextMenu_ExecuteCommandFunc)
            {
                CLauncherTipContextMenu_ExecuteCommandFunc(
                    (char*)params->_this - 0x58,
                    &info
                );
            }
        }
        else
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xc8 - 0x58)) + ((INT64)res - 4000) * 8);
            if (CLauncherTipContextMenu_ExecuteShutdownCommandFunc)
            {
                CLauncherTipContextMenu_ExecuteShutdownCommandFunc(
                    (char*)params->_this - 0x58,
                    &info
                );
            }
        }
    }

finalize:
    params->iunk->lpVtbl->Release(params->iunk);
    SendMessage(
        hWinXWnd,
        WM_CLOSE,
        0,
        0
    );
    free(params);
    hIsWinXShown = NULL;
    return 0;
}

INT64 CLauncherTipContextMenu_ShowLauncherTipContextMenuHook(
    void* _this,
    POINT* pt
)
{
    if (hWinXThread)
    {
        WaitForSingleObject(hWinXThread, INFINITE);
        CloseHandle(hWinXThread);
        hWinXThread = NULL;
    }

    if (hIsWinXShown)
    {
        goto finalize;
    }

    BOOL bShouldCenterWinXHorizontally = FALSE;
    POINT point;
    if (pt)
    {
        point = *pt;
        BOOL bBottom, bRight;
        POINT dPt = GetDefaultWinXPosition(FALSE, &bBottom, &bRight, FALSE);
        POINT posCursor;
        GetCursorPos(&posCursor);
        RECT rcHitZone;
        rcHitZone.left = pt->x - 5;
        rcHitZone.right = pt->x + 5;
        rcHitZone.top = pt->y - 5;
        rcHitZone.bottom = pt->y + 5;
        //printf("%d %d = %d %d %d %d\n", posCursor.x, posCursor.y, rcHitZone.left, rcHitZone.right, rcHitZone.top, rcHitZone.bottom);
        if (bBottom && IsThemeActive() && PtInRect(&rcHitZone, posCursor))
        {
            HMONITOR hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO mi;
            mi.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor, &mi);
            HWND hWndUnder = WindowFromPoint(*pt);
            TCHAR wszClassName[100];
            GetClassNameW(hWndUnder, wszClassName, 100);
            if (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd"))
            {
                hWndUnder = FindWindowEx(
                    hWndUnder,
                    NULL,
                    L"Start",
                    NULL
                );
            }
            RECT rcUnder;
            GetWindowRect(hWndUnder, &rcUnder);
            if (mi.rcMonitor.left != rcUnder.left)
            {
                bShouldCenterWinXHorizontally = TRUE;
                point.x = rcUnder.left + (rcUnder.right - rcUnder.left) / 2;
                point.y = rcUnder.top;
            }
            else
            {
                UINT dpiX, dpiY;
                HRESULT hr = GetDpiForMonitor(
                    hMonitor,
                    MDT_DEFAULT,
                    &dpiX,
                    &dpiY
                );
                double dx = dpiX / 96.0, dy = dpiY / 96.0;
                BOOL xo = FALSE, yo = FALSE;
                if (point.x - WINX_ADJUST_X * dx < mi.rcMonitor.left)
                {
                    xo = TRUE;
                }
                if (point.y + WINX_ADJUST_Y * dy > mi.rcMonitor.bottom)
                {
                    yo = TRUE;
                }
                POINT ptCursor;
                GetCursorPos(&ptCursor);
                if (xo)
                {
                    ptCursor.x += (WINX_ADJUST_X * 2) * dx;
                }
                else
                {
                    point.x -= WINX_ADJUST_X * dx;
                }
                if (yo)
                {
                    ptCursor.y -= (WINX_ADJUST_Y * 2) * dy;
                }
                else
                {
                    point.y += WINX_ADJUST_Y * dy;
                }
                SetCursorPos(ptCursor.x, ptCursor.y);
            }
        }
    }
    else
    {
        point = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE);
    }

    IUnknown* iunk = NULL;
    if (CLauncherTipContextMenu_GetMenuItemsAsyncFunc)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc(
            _this,
            &point,
            &iunk
        );
    }
    if (iunk)
    {
        iunk->lpVtbl->AddRef(iunk);

        ShowLauncherTipContextMenuParameters* params = malloc(
            sizeof(ShowLauncherTipContextMenuParameters)
        );
        params->_this = _this;
        params->point = point;
        params->iunk = iunk;
        params->bShouldCenterWinXHorizontally = bShouldCenterWinXHorizontally;
        hIsWinXShown = CreateThread(
            0,
            0,
            ShowLauncherTipContextMenu,
            params,
            0,
            0
        );
        hWinXThread = hIsWinXShown;
    }
finalize:
    if (CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)
    {
        return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
    }
    return 0;
}
#endif
#pragma endregion


#pragma region "Show Start in correct location according to TaskbarAl"
#ifdef _WIN64
void UpdateStartMenuPositioning(LPARAM loIsShouldInitializeArray_hiIsShouldRoInitialize)
{
    BOOL bShouldInitialize = LOWORD(loIsShouldInitializeArray_hiIsShouldRoInitialize);
    BOOL bShouldRoInitialize = HIWORD(loIsShouldInitializeArray_hiIsShouldRoInitialize);

    if (!bOldTaskbar)
    {
        return;
    }

    DWORD dwPosCurrent = GetStartMenuPosition(SHRegGetValueFromHKCUHKLMFunc);
    if (bShouldInitialize || InterlockedAdd(&dwTaskbarAl, 0) != dwPosCurrent)
    {
        HRESULT hr = S_OK;
        if (bShouldRoInitialize)
        {
            hr = RoInitialize(RO_INIT_MULTITHREADED);
        }
        if (SUCCEEDED(hr))
        {
            InterlockedExchange(&dwTaskbarAl, dwPosCurrent);
            StartMenuPositioningData spd;
            spd.pMonitorCount = &dwMonitorCount;
            spd.pMonitorList = hMonitorList;
            spd.location = dwPosCurrent;
            if (bShouldInitialize)
            {
                spd.operation = STARTMENU_POSITIONING_OPERATION_REMOVE;
                unsigned int k = InterlockedAdd(&dwMonitorCount, 0);
                for (unsigned int i = 0; i < k; ++i)
                {
                    NeedsRo_PositionStartMenuForMonitor(hMonitorList[i], NULL, NULL, &spd);
                }
                InterlockedExchange(&dwMonitorCount, 0);
                spd.operation = STARTMENU_POSITIONING_OPERATION_ADD;
            }
            else
            {
                spd.operation = STARTMENU_POSITIONING_OPERATION_CHANGE;
            }
            EnumDisplayMonitors(NULL, NULL, NeedsRo_PositionStartMenuForMonitor, &spd);
            if (bShouldRoInitialize)
            {
                RoUninitialize();
            }
        }
    }
}
#else
void UpdateStartMenuPositioning(LPARAM loIsShouldInitializeArray_hiIsShouldRoInitialize) {}
#endif
#pragma endregion


#pragma region "Shell_TrayWnd subclass"
#ifdef _WIN64
HMENU explorer_LoadMenuW(HINSTANCE hInstance, LPCWSTR lpMenuName)
{
    HMENU hMenu = LoadMenuW(hInstance, lpMenuName);
    if (hInstance == GetModuleHandle(NULL) && lpMenuName == MAKEINTRESOURCEW(205))
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            TCHAR buffer[260];
            LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
            if (!bNoMenuAccelerator)
            {
                buffer[0] = L'&';
            }
            wchar_t* p = wcschr(buffer, L'(');
            if (p)
            {
                p--;
                if (p == L' ')
                {
                    *p = 0;
                }
                else
                {
                    p++;
                    *p = 0;
                }
            }
            MENUITEMINFOW menuInfo;
            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
            menuInfo.cbSize = sizeof(MENUITEMINFOW);
            menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
            menuInfo.wID = 12100;
            menuInfo.dwItemData = CheckForUpdatesThread;
            menuInfo.fType = MFT_STRING;
            menuInfo.dwTypeData = buffer;
            menuInfo.cch = wcslen(buffer);
            InsertMenuItemW(
                hSubMenu,
                GetMenuItemCount(hSubMenu) - 4,
                TRUE,
                &menuInfo
            );
        }
    }
    return hMenu;
}

HHOOK Shell_TrayWndMouseHook = NULL;

BOOL Shell_TrayWnd_IsTaskbarRightClick(POINT pt)
{
    HRESULT hr = S_OK;
    IUIAutomation2* pIUIAutomation2 = NULL;
    IUIAutomationElement* pIUIAutomationElement = NULL;
    HWND hWnd = NULL;
    BOOL bRet = FALSE;
    
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER, &IID_IUIAutomation2, &pIUIAutomation2);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIUIAutomation2->lpVtbl->ElementFromPoint(pIUIAutomation2, pt, &pIUIAutomationElement);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIUIAutomationElement->lpVtbl->get_CurrentNativeWindowHandle(pIUIAutomationElement, &hWnd);
    }
    if (SUCCEEDED(hr))
    {
        if (IsWindow(hWnd))
        {
            HWND hAncestor = GetAncestor(hWnd, GA_ROOT);
            HWND hWindow = FindWindowExW(hAncestor, NULL, L"Windows.UI.Composition.DesktopWindowContentBridge", NULL);
            if (IsWindow(hWindow))
            {
                hWindow = FindWindowExW(hWindow, NULL, L"Windows.UI.Input.InputSite.WindowClass", NULL);
                if (IsWindow(hWindow))
                {
                    if (hWindow == hWnd)
                    {
                        bRet = TRUE;
                    }
                }
            }
        }
    }
    if (pIUIAutomationElement)
    {
        pIUIAutomationElement->lpVtbl->Release(pIUIAutomationElement);
    }
    if (pIUIAutomation2)
    {
        pIUIAutomation2->lpVtbl->Release(pIUIAutomation2);
    }
    return bRet;
}

LRESULT CALLBACK Shell_TrayWndMouseProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (nCode == HC_ACTION && wParam == WM_RBUTTONUP && Shell_TrayWnd_IsTaskbarRightClick(((MOUSEHOOKSTRUCT*)lParam)->pt))
    {
        PostMessageW(
            FindWindowW(L"Shell_TrayWnd", NULL),
            RegisterWindowMessageW(L"Windows11ContextMenu_" _T(EP_CLSID)),
            0,
            MAKELPARAM(((MOUSEHOOKSTRUCT*)lParam)->pt.x, ((MOUSEHOOKSTRUCT*)lParam)->pt.y)
        );
        return 1;
    }
    return CallNextHookEx(Shell_TrayWndMouseHook, nCode, wParam, lParam);
}

INT64 Shell_TrayWndSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc);
    }
    else if (uMsg == WM_LBUTTONDBLCLK && bTaskbarAutohideOnDoubleClick)
    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        if (SHAppBarMessage(ABM_GETSTATE, &abd) == ABS_AUTOHIDE)
        {
            abd.lParam = 0;
            SHAppBarMessage(ABM_SETSTATE, &abd);
        }
        else
        {
            abd.lParam = ABS_AUTOHIDE;
            SHAppBarMessage(ABM_SETSTATE, &abd);
        }
    }
    else if (uMsg == WM_HOTKEY && wParam == 500 && lParam == MAKELPARAM(MOD_WIN, 0x41))
    {
        InvokeActionCenter();
        return 0;
        /*if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = 1;
        }
        LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = bHideControlCenterButton;
        }
        return lRes;*/
    }
    else if (uMsg == WM_DISPLAYCHANGE)
    {
        UpdateStartMenuPositioning(MAKELPARAM(TRUE, FALSE));
    }
    else if (!bOldTaskbar && uMsg == WM_PARENTNOTIFY && wParam == WM_RBUTTONDOWN && !Shell_TrayWndMouseHook) // && !IsUndockingDisabled
    {
        DWORD dwThreadId = GetCurrentThreadId();
        Shell_TrayWndMouseHook = SetWindowsHookExW(WH_MOUSE, Shell_TrayWndMouseProc, NULL, dwThreadId);
    }
    else if (uMsg == RegisterWindowMessageW(L"Windows11ContextMenu_" _T(EP_CLSID)))
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        HMENU hMenu = LoadMenuW(GetModuleHandle(NULL), MAKEINTRESOURCEW(205));
        if (hMenu)
        {
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            if (hSubMenu)
            {
                if (GetAsyncKeyState(VK_SHIFT) >= 0 || GetAsyncKeyState(VK_CONTROL) >= 0)
                {
                    DeleteMenu(hSubMenu, 518, MF_BYCOMMAND); // Exit Explorer
                }
                DeleteMenu(hSubMenu, 424, MF_BYCOMMAND); // Lock the taskbar
                DeleteMenu(hSubMenu, 425, MF_BYCOMMAND); // Lock all taskbars
                DeleteMenu(hSubMenu, 416, MF_BYCOMMAND); // Undo
                DeleteMenu(hSubMenu, 437, MF_BYCOMMAND); // Show Pen button
                DeleteMenu(hSubMenu, 438, MF_BYCOMMAND); // Show touchpad button
                DeleteMenu(hSubMenu, 435, MF_BYCOMMAND); // Show People on the taskbar
                DeleteMenu(hSubMenu, 430, MF_BYCOMMAND); // Show Task View button
                DeleteMenu(hSubMenu, 449, MF_BYCOMMAND); // Show Cortana button
                DeleteMenu(hSubMenu, 621, MF_BYCOMMAND); // News and interests
                DeleteMenu(hSubMenu, 445, MF_BYCOMMAND); // Cortana
                DeleteMenu(hSubMenu, 431, MF_BYCOMMAND); // Search
                DeleteMenu(hSubMenu, 421, MF_BYCOMMAND); // Customize notification icons
                DeleteMenu(hSubMenu, 408, MF_BYCOMMAND); // Adjust date/time
                DeleteMenu(hSubMenu, 436, MF_BYCOMMAND); // Show touch keyboard button
                DeleteMenu(hSubMenu, 0, MF_BYPOSITION); // Separator
                DeleteMenu(hSubMenu, 0, MF_BYPOSITION); // Separator

                TCHAR buffer[260];
                LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
                if (!bNoMenuAccelerator)
                {
                    buffer[0] = L'&';
                }
                wchar_t* p = wcschr(buffer, L'(');
                if (p)
                {
                    p--;
                    if (p == L' ')
                    {
                        *p = 0;
                    }
                    else
                    {
                        p++;
                        *p = 0;
                    }
                }
                MENUITEMINFOW menuInfo;
                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                menuInfo.cbSize = sizeof(MENUITEMINFOW);
                menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | MIIM_STATE;
                menuInfo.wID = 3999;
                menuInfo.dwItemData = 0;
                menuInfo.fType = MFT_STRING;
                menuInfo.dwTypeData = buffer;
                menuInfo.cch = wcslen(buffer);
                InsertMenuItemW(
                    hSubMenu,
                    GetMenuItemCount(hSubMenu) - 1,
                    TRUE,
                    &menuInfo
                );

                INT64* unknown_array = NULL;
                if (bSkinMenus)
                {
                    unknown_array = calloc(4, sizeof(INT64));
                    if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
                    {
                        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                            hSubMenu,
                            hWnd,
                            &pt,
                            0xc,
                            unknown_array
                        );
                    }
                }

                BOOL res = TrackPopupMenu(
                    hSubMenu,
                    TPM_RETURNCMD | TPM_RIGHTBUTTON,
                    pt.x,
                    pt.y,
                    0,
                    hWnd,
                    0
                );
                if (res == 3999)
                {
                    LaunchPropertiesGUI(hModule);
                }
                else if (res == 403)
                {
                    CascadeWindows(NULL, 0, NULL, 0, NULL);
                }
                else if (res == 404)
                {
                    TileWindows(NULL, 0, NULL, 0, NULL);
                }
                else if (res == 405)
                {
                    TileWindows(NULL, 1, NULL, 0, NULL);
                }
                else
                {
                    PostMessageW(hWnd, WM_COMMAND, res, 0);
                }

                if (bSkinMenus)
                {
                    if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                    {
                        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                            hSubMenu,
                            hWnd,
                            &pt
                        );
                    }
                    free(unknown_array);
                }

                DestroyMenu(hSubMenu);
            }
            DestroyMenu(hMenu);
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Allow legacy volume applet"
#ifdef _WIN64
LSTATUS sndvolsso_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    if (SHRegGetValueFromHKCUHKLMFunc &&
        hkey == HKEY_LOCAL_MACHINE &&
        !_wcsicmp(lpSubKey, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\MTCUVC") &&
        !_wcsicmp(lpValue, L"EnableMTCUVC"))
    {
        return SHRegGetValueFromHKCUHKLMFunc(
            lpSubKey,
            lpValue,
            SRRF_RT_REG_DWORD,
            pdwType,
            pvData,
            pcbData
        );
    }
    return RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}
#endif
#pragma endregion


#pragma region "Allow legacy date and time"
#ifdef _WIN64
DEFINE_GUID(GUID_Win32Clock,
    0x0A323554A,
    0x0FE1, 0x4E49, 0xae, 0xe1,
    0x67, 0x22, 0x46, 0x5d, 0x79, 0x9f
);
DEFINE_GUID(IID_Win32Clock,
    0x7A5FCA8A,
    0x76B1, 0x44C8, 0xa9, 0x7c,
    0xe7, 0x17, 0x3c, 0xca, 0x5f, 0x4f
);
typedef interface Win32Clock Win32Clock;

typedef struct Win32ClockVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            Win32Clock* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        Win32Clock* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        Win32Clock* This);

    HRESULT(STDMETHODCALLTYPE* ShowWin32Clock)(
        Win32Clock* This,
        /* [in] */ HWND hWnd,
        /* [in] */ LPRECT lpRect);

    END_INTERFACE
} Win32ClockVtbl;

interface Win32Clock
{
    CONST_VTBL struct Win32ClockVtbl* lpVtbl;
};
DWORD ShouldShowLegacyClockExperience()
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell"),
        TEXT("UseWin32TrayClockExperience"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwVal,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS)
    {
        return dwVal;
    }
    return 0;
}
BOOL ShowLegacyClockExpierience(HWND hWnd)
{
    if (!hWnd)
    {
        return FALSE;
    }
    HRESULT hr = S_OK;
    Win32Clock* pWin32Clock = NULL;
    hr = CoCreateInstance(
        &GUID_Win32Clock,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
        &IID_Win32Clock,
        &pWin32Clock
    );
    if (SUCCEEDED(hr))
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        pWin32Clock->lpVtbl->ShowWin32Clock(pWin32Clock, hWnd, &rc);
        pWin32Clock->lpVtbl->Release(pWin32Clock);
    }
    return TRUE;
}

INT64 ClockButtonSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, ClockButtonSubclassProc, ClockButtonSubclassProc);
    }
    else if (uMsg == WM_LBUTTONDOWN || (uMsg == WM_KEYDOWN && wParam == VK_RETURN))
    {
        if (ShouldShowLegacyClockExperience() == 1)
        {
            if (!FindWindowW(L"ClockFlyoutWindow", NULL))
            {
                return ShowLegacyClockExpierience(hWnd);
            }
            else
            {
                return 1;
            }
        }
        else if (ShouldShowLegacyClockExperience() == 2)
        {
            if (FindWindowW(L"Windows.UI.Core.CoreWindow", NULL))
            {
                ToggleNotificationsFlyout();
            }
            return 1;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Popup menu hooks"
BOOL IsImmersiveMenu = FALSE;
BOOL CheckIfImmersiveContextMenu(
    HWND unnamedParam1,
    LPCSTR unnamedParam2,
    HANDLE unnamedParam3
)
{
    if ((*((WORD*)&(unnamedParam2)+1)))
    {
        if (!strncmp(unnamedParam2, "ImmersiveContextMenuArray", 25))
        {
            IsImmersiveMenu = TRUE;
            return FALSE;
        }
    }
    return TRUE;
}
void RemoveOwnerDrawFromMenu(int level, HMENU hMenu)
{
    if (hMenu)
    {
        int k = GetMenuItemCount(hMenu);
        for (int i = 0; i < k; ++i)
        {
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_FTYPE | MIIM_SUBMENU;
            if (GetMenuItemInfoW(hMenu, i, TRUE, &mii) && (mii.fType & MFT_OWNERDRAW))
            {
                mii.fType &= ~MFT_OWNERDRAW;
                printf("[ROD]: Level %d Position %d/%d Status %d\n", level, i, k, SetMenuItemInfoW(hMenu, i, TRUE, &mii));
                RemoveOwnerDrawFromMenu(level + 1, mii.hSubMenu);
            }
        }
    }
}
BOOL CheckIfMenuContainsOwnPropertiesItem(HMENU hMenu)
{
#ifdef _WIN64
    if (hMenu)
    {
        int k = GetMenuItemCount(hMenu);
        for (int i = k - 1; i >= 0; i--)
        {
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_DATA | MIIM_ID;
            BOOL b = GetMenuItemInfoW(hMenu, i, TRUE, &mii);
            if (b && (mii.wID >= 12000 && mii.wID <= 12200) && mii.dwItemData == CheckForUpdatesThread)
            {
                return TRUE;
            }
        }
    }
#endif
    return FALSE;
}
BOOL TrackPopupMenuHookEx(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    IsImmersiveMenu = FALSE;

    wchar_t wszClassName[200];
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;
#ifndef _WIN64
            if (bIsExplorerProcess)
            {
#else
            if (bIsExplorerProcess && ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
#endif
            }
            else
            {
                RemoveOwnerDrawFromMenu(0, hMenu);
            }

            BOOL bRet = TrackPopupMenuEx(
                hMenu,
                uFlags,
                x,
                y,
                hWnd,
                lptpm
            );
#ifdef _WIN64
            if (bContainsOwn && (bRet >= 12000 && bRet <= 12200))
            {
                LaunchPropertiesGUI(hModule);
                return FALSE;
            }
#endif
            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    BOOL b = TrackPopupMenuEx(
        hMenu,
        uFlags,
        x,
        y,
        hWnd,
        lptpm
    );
#ifdef _WIN64
    if (bContainsOwn && (b >= 12000 && b <= 12200))
    {
        LaunchPropertiesGUI(hModule);
        return FALSE;
    }
#endif
    return b;
}
BOOL TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    IsImmersiveMenu = FALSE;

    wchar_t wszClassName[200];
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;

#ifndef _WIN64
            if (bIsExplorerProcess)
            {
#else
            if (bIsExplorerProcess && ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
#endif
            }
            else
            {
                RemoveOwnerDrawFromMenu(0, hMenu);
            }

            BOOL bRet = TrackPopupMenu(
                hMenu,
                uFlags,
                x,
                y,
                0,
                hWnd,
                prcRect
            );
#ifdef _WIN64
            if (bContainsOwn && (bRet >= 12000 && bRet <= 12200))
            {
                LaunchPropertiesGUI(hModule);
                return FALSE;
            }
#endif
            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    BOOL b = TrackPopupMenu(
        hMenu,
        uFlags,
        x,
        y,
        0,
        hWnd,
        prcRect
    );
#ifdef _WIN64
    if (bContainsOwn && (b >= 12000 && b <= 12200))
    {
        LaunchPropertiesGUI(hModule);
        return FALSE;
    }
#endif
    return b;
}
#ifdef _WIN64
#define TB_POS_NOWHERE 0
#define TB_POS_BOTTOM 1
#define TB_POS_TOP 2
#define TB_POS_LEFT 3
#define TB_POS_RIGHT 4
void PopupMenuAdjustCoordinatesAndFlags(int* x, int* y, UINT* uFlags)
{
    POINT pt;
    GetCursorPos(&pt);
    RECT rc;
    UINT tbPos = GetTaskbarLocationAndSize(pt, &rc);
    if (tbPos == TB_POS_BOTTOM)
    {
        *y = MIN(*y, rc.top);
        *uFlags |= TPM_CENTERALIGN | TPM_BOTTOMALIGN;
    }
    else if (tbPos == TB_POS_TOP)
    {
        *y = MAX(*y, rc.bottom);
        *uFlags |= TPM_CENTERALIGN | TPM_TOPALIGN;
    }
    else if (tbPos == TB_POS_LEFT)
    {
        *x = MAX(*x, rc.right);
        *uFlags |= TPM_VCENTERALIGN | TPM_LEFTALIGN;
    }
    if (tbPos == TB_POS_RIGHT)
    {
        *x = MIN(*x, rc.left);
        *uFlags |= TPM_VCENTERALIGN | TPM_RIGHTALIGN;
    }
}
UINT GetTaskbarLocationAndSize(POINT ptCursor, RECT* rc)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        &mi
    );
    if (hWnd)
    {
        GetWindowRect(hWnd, rc);
        RECT rcC = *rc;
        rcC.left -= mi.rcMonitor.left;
        rcC.right -= mi.rcMonitor.left;
        rcC.top -= mi.rcMonitor.top;
        rcC.bottom -= mi.rcMonitor.top;
        if (rcC.left < 5 && rcC.top > 5)
        {
            return TB_POS_BOTTOM;
        }
        else if (rcC.left < 5 && rcC.top < 5 && rcC.right > rcC.bottom)
        {
            return TB_POS_TOP;
        }
        else if (rcC.left < 5 && rcC.top < 5 && rcC.right < rcC.bottom)
        {
            return TB_POS_LEFT;
        }
        else if (rcC.left > 5 && rcC.top < 5)
        {
            return TB_POS_RIGHT;
        }
    }
    return TB_POS_NOWHERE;
}
INT64 OwnerDrawSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    BOOL v12 = FALSE;
    if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc &&
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc(
            hWnd,
            uMsg,
            wParam,
            lParam,
            &v12
        ))
    {
        return 0;
    }
    return DefSubclassProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}
long long explorer_TrackPopupMenuExElapsed = 0;
BOOL explorer_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - explorer_TrackPopupMenuExElapsed;
    BOOL b = FALSE;

    wchar_t wszClassName[200];
    GetClassNameW(hWnd, wszClassName, 200);
    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }
    
    if (elapsed > POPUPMENU_EX_ELAPSED || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags,
            x,
            y,
            hWnd,
            lptpm
        );
        if (bContainsOwn && (b >= 12000 && b <= 12200))
        {
            LaunchPropertiesGUI(hModule);
            return FALSE;
        }
        explorer_TrackPopupMenuExElapsed = milliseconds_now();
    }
    return b;
}
long long pnidui_TrackPopupMenuElapsed = 0;
BOOL pnidui_TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    long long elapsed = milliseconds_now() - pnidui_TrackPopupMenuElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_PNIDUI_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }
        b = TrackPopupMenu(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            0,
            hWnd,
            prcRect
        );
        if (bReplaceNetwork && b == 3109)
        {
            LaunchNetworkTargets(bReplaceNetwork + 2);
            b = 0;
        }
        pnidui_TrackPopupMenuElapsed = milliseconds_now();
    }
    return b;
}
long long sndvolsso_TrackPopupMenuExElapsed = 0;
BOOL sndvolsso_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - sndvolsso_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SNDVOLSSO_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }

        /*MENUITEMINFOW menuInfo;
        ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
        menuInfo.cbSize = sizeof(MENUITEMINFOW);
        menuInfo.fMask = MIIM_ID | MIIM_STRING;
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        menuInfo.dwTypeData = malloc(menuInfo.cch + sizeof(wchar_t));
        menuInfo.cch++;
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wcscpy_s(menuInfo.dwTypeData, menuInfo.cch, L"test");
        menuInfo.fMask = MIIM_STRING;
        wprintf(L"SetMenuItemInfoW %s %d\n", menuInfo.dwTypeData, SetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wcscpy_s(menuInfo.dwTypeData, menuInfo.cch, L"");
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wprintf(L"%s\n", menuInfo.dwTypeData);
        free(menuInfo.dwTypeData);*/

        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        sndvolsso_TrackPopupMenuExElapsed = milliseconds_now();
    }
    return b;
}
long long stobject_TrackPopupMenuExElapsed = 0;
BOOL stobject_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - stobject_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SAFETOREMOVE_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        stobject_TrackPopupMenuExElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
long long stobject_TrackPopupMenuElapsed = 0;
BOOL stobject_TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    long long elapsed = milliseconds_now() - stobject_TrackPopupMenuElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SAFETOREMOVE_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenu(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            0,
            hWnd,
            prcRect
        );
        stobject_TrackPopupMenuElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
long long bthprops_TrackPopupMenuExElapsed = 0;
BOOL bthprops_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - bthprops_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_BLUETOOTH_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        bthprops_TrackPopupMenuExElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
#endif
#pragma endregion


#pragma region "Disable immersive menus"
BOOL WINAPI DisableImmersiveMenus_SystemParametersInfoW(
    UINT  uiAction,
    UINT  uiParam,
    PVOID pvParam,
    UINT  fWinIni
)
{
    if (bDisableImmersiveContextMenu && uiAction == SPI_GETSCREENREADER)
    {
        printf("SystemParametersInfoW\n");
        *(BOOL*)pvParam = TRUE;
        return TRUE;
    }
    return SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);
}
#pragma endregion


#pragma region "Explorer: Hide search bar, Mica effect (private), hide navigation bar"
static HWND(__stdcall *explorerframe_SHCreateWorkerWindowFunc)(
    WNDPROC  	wndProc,
    HWND  	hWndParent,
    DWORD  	dwExStyle,
    DWORD  	dwStyle,
    HMENU  	hMenu,
    LONG_PTR  	wnd_extra
    );

HWND WINAPI explorerframe_SHCreateWorkerWindowHook(
    WNDPROC  	wndProc,
    HWND  	hWndParent,
    DWORD  	dwExStyle,
    DWORD  	dwStyle,
    HMENU  	hMenu,
    LONG_PTR  	wnd_extra
)
{
    HWND result;
    LSTATUS lRes = ERROR_FILE_NOT_FOUND;
    DWORD dwSize = 0;
    
    printf("%x %x\n", dwExStyle, dwStyle);

    if (SHRegGetValueFromHKCUHKLMWithOpt(
        TEXT("SOFTWARE\\Classes\\CLSID\\{056440FD-8568-48e7-A632-72157243B55B}\\InProcServer32"),
        TEXT(""),
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS && (dwSize < 4) && dwExStyle == 0x10000 && dwStyle == 1174405120)
    {
        result = 0;
    }
    else
    {
        result = explorerframe_SHCreateWorkerWindowFunc(
            wndProc,
            hWndParent,
            dwExStyle,
            dwStyle,
            hMenu,
            wnd_extra
        );
    }
    if (dwExStyle == 0x10000 && dwStyle == 0x46000000)
    {
#ifdef USE_PRIVATE_INTERFACES
        if (bMicaEffectOnTitlebar && result)
        {
            BOOL value = TRUE;
            SetPropW(hWndParent, L"NavBarGlass", HANDLE_FLAG_INHERIT);
            DwmSetWindowAttribute(hWndParent, DWMWA_MICA_EFFFECT, &value, sizeof(BOOL));
            if (result) SetWindowSubclass(result, ExplorerMicaTitlebarSubclassProc, ExplorerMicaTitlebarSubclassProc, 0);
        }
#endif

        if (bHideExplorerSearchBar && result)
        {
            SetWindowSubclass(hWndParent, HideExplorerSearchBarSubClass, HideExplorerSearchBarSubClass, 0);
        }
    }
    return result;
}
#pragma endregion


#pragma region "Fix battery flyout"
#ifdef _WIN64
LSTATUS stobject_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    if (!lstrcmpW(lpValue, L"UseWin32BatteryFlyout"))
    {
        if (SHRegGetValueFromHKCUHKLMFunc)
        {
            return SHRegGetValueFromHKCUHKLMFunc(
                lpSubKey,
                lpValue,
                SRRF_RT_REG_DWORD,
                pdwType,
                pvData,
                pcbData
            );
        }
    }
    return RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

HRESULT stobject_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID* ppv
)
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (IsEqualGUID(rclsid, &CLSID_ImmersiveShell) &&
        IsEqualGUID(riid, &IID_IServiceProvider) &&
        SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell"),
            TEXT("UseWin32BatteryFlyout"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwVal,
            (LPDWORD)(&dwSize)
        ) == ERROR_SUCCESS)
    {
        if (!dwVal)
        {
            if (hCheckForegroundThread)
            {
                if (WaitForSingleObject(hCheckForegroundThread, 0) == WAIT_TIMEOUT)
                {
                    return E_NOINTERFACE;
                }
                WaitForSingleObject(hCheckForegroundThread, INFINITE);
                CloseHandle(hCheckForegroundThread);
                hCheckForegroundThread = NULL;
            }
            HKEY hKey = NULL;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                _T(SEH_REGPATH),
                0,
                NULL,
                REG_OPTION_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                NULL
            ) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
            }
            TerminateShellExperienceHost();
            InvokeFlyout(0, INVOKE_FLYOUT_BATTERY);
            Sleep(100);
            hCheckForegroundThread = CreateThread(
                0,
                0,
                CheckForegroundThread,
                dwVal,
                0,
                0
            );
        }
    }
    return CoCreateInstance(
        rclsid,
        pUnkOuter,
        dwClsContext,
        riid,
        ppv
    );
}
#endif
#pragma endregion



#pragma region "Show WiFi networks on network icon click"
#ifdef _WIN64
HRESULT pnidui_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID* ppv
)
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (IsEqualGUID(rclsid, &CLSID_ImmersiveShell) && 
        IsEqualGUID(riid, &IID_IServiceProvider) &&
        SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Settings\\Network"),
            TEXT("ReplaceVan"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwVal,
            (LPDWORD)(&dwSize)
        ) == ERROR_SUCCESS)
    {
        if (dwVal)
        {
            if (dwVal == 5 || dwVal == 6)
            {
                if (hCheckForegroundThread)
                {
                    WaitForSingleObject(hCheckForegroundThread, INFINITE);
                    CloseHandle(hCheckForegroundThread);
                    hCheckForegroundThread = NULL;
                }
                if (milliseconds_now() - elapsedCheckForeground > CHECKFOREGROUNDELAPSED_TIMEOUT)
                {
                    LaunchNetworkTargets(dwVal);
                    hCheckForegroundThread = CreateThread(
                        0,
                        0,
                        CheckForegroundThread,
                        dwVal,
                        0,
                        0
                    );
                }
            }
            else
            {
                LaunchNetworkTargets(dwVal);
            }
            return E_NOINTERFACE;
        }
        else
        {
            if (hCheckForegroundThread)
            {
                if (WaitForSingleObject(hCheckForegroundThread, 0) == WAIT_TIMEOUT)
                {
                    return E_NOINTERFACE;
                }
                WaitForSingleObject(hCheckForegroundThread, INFINITE);
                CloseHandle(hCheckForegroundThread);
                hCheckForegroundThread = NULL;
            }
            HKEY hKey = NULL;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                _T(SEH_REGPATH),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                NULL
            ) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
            }
            TerminateShellExperienceHost();
            InvokeFlyout(0, INVOKE_FLYOUT_NETWORK);
            Sleep(100);
            hCheckForegroundThread = CreateThread(
                0,
                0,
                CheckForegroundThread,
                dwVal,
                0,
                0
            );
        }
    }
    return CoCreateInstance(
        rclsid,
        pUnkOuter,
        dwClsContext,
        riid,
        ppv
    );
}
#endif
#pragma endregion


#pragma region "Show Clock flyout on Win+C"
#ifdef _WIN64
typedef struct _ClockButton_ToggleFlyoutCallback_Params
{
    void* TrayUIInstance;
    unsigned int CLOCKBUTTON_OFFSET_IN_TRAYUI;
    void* oldClockButtonInstance;
} ClockButton_ToggleFlyoutCallback_Params;
void ClockButton_ToggleFlyoutCallback(
    HWND hWnd,
    UINT uMsg,
    ClockButton_ToggleFlyoutCallback_Params* params,
    LRESULT lRes
)
{
    *((INT64*)params->TrayUIInstance + params->CLOCKBUTTON_OFFSET_IN_TRAYUI) = params->oldClockButtonInstance;
    free(params);
}
INT64 winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook(
    void* _this,
    INT64 a2,
    INT a3
)
{
    if (!bClockFlyoutOnWinC)
    {
        if (winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc)
        {
            return winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc(_this, a2, a3);
        }
        return 0;
    }
    if (a2 == 786 && a3 == 107)
    {
        POINT ptCursor;
        GetCursorPos(&ptCursor);
        HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
            ptCursor,
            MONITOR_DEFAULTTOPRIMARY,
            NULL
        );
        HWND prev_hWnd = hWnd;
        HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
        const unsigned int WM_TOGGLE_CLOCK_FLYOUT = 1486;
        if (hWnd == hShellTray_Wnd)
        {
            if (ShouldShowLegacyClockExperience() == 1)
            {
                if (!FindWindowW(L"ClockFlyoutWindow", NULL))
                {
                    return ShowLegacyClockExpierience(FindWindowExW(FindWindowExW(hShellTray_Wnd, NULL, L"TrayNotifyWnd", NULL), NULL, L"TrayClockWClass", NULL));
                }
                else
                {
                    return PostMessageW(FindWindowW(L"ClockFlyoutWindow", NULL), WM_CLOSE, 0, 0);
                }
            }
            else if (ShouldShowLegacyClockExperience() == 2)
            {
                ToggleNotificationsFlyout();
                return 0;
            }
            // On the main monitor, the TrayUI component of CTray handles this
            // message and basically does a `ClockButton::ToggleFlyout`; that's
            // the only place in code where that is used, otherwise, clicking and
            // dismissing the clock flyout probably involves 2 separate methods
            PostMessageW(hShellTray_Wnd, WM_TOGGLE_CLOCK_FLYOUT, 0, 0);
        }
        else
        {
            // Of course, on secondary monitors, the situation is much more
            // complicated; there is no simple way to do this, afaik; the way I do it
            // is to obtain a pointer to TrayUI from CTray (pointers to the classes
            // that created the windows are always available at location 0 in the hWnd)
            // and from there issue a "show clock flyout" message manually, taking care to temporarly
            // change the internal clock button pointer of the class to point
            // to the clock button on the secondary monitor.
            hWnd = FindWindowExW(hWnd, NULL, L"ClockButton", NULL);
            if (hWnd)
            {
                if (ShouldShowLegacyClockExperience() == 1)
                {
                    if (!FindWindowW(L"ClockFlyoutWindow", NULL))
                    {
                        return ShowLegacyClockExpierience(hWnd);
                    }
                    else
                    {
                        return PostMessageW(FindWindowW(L"ClockFlyoutWindow", NULL), WM_CLOSE, 0, 0);
                    }
                }
                else if (ShouldShowLegacyClockExperience() == 2)
                {
                    ToggleNotificationsFlyout();
                    return 0;
                }
                INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
                void* ClockButtonInstance = (BYTE*)(GetWindowLongPtrW(hWnd, 0)); // -> ClockButton

                // inspect CTray::v_WndProc, look for mentions of
                // CTray::_HandlePowerStatus or patterns like **((_QWORD **)this + 110) + 184i64
                const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
                // simply inspect vtable of TrayUI
                const unsigned int TRAYUI_WNDPROC_POSITION_IN_VTABLE = 4;
                // inspect TrayUI::WndProc, specifically this section
                /*
                    {
                      if ( (_DWORD)a3 == 1486 )
                      {
                        v80 = (ClockButton *)*((_QWORD *)this + 100);
                        if ( v80 )
                          ClockButton::ToggleFlyout(v80);
                */
                const unsigned int CLOCKBUTTON_OFFSET_IN_TRAYUI = 100;
                void* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);
                void* oldClockButtonInstance = *((INT64*)TrayUIInstance + CLOCKBUTTON_OFFSET_IN_TRAYUI);
                ClockButton_ToggleFlyoutCallback_Params* params = malloc(sizeof(ClockButton_ToggleFlyoutCallback_Params));
                if (params)
                {
                    *((INT64*)TrayUIInstance + CLOCKBUTTON_OFFSET_IN_TRAYUI) = ClockButtonInstance;
                    params->TrayUIInstance = TrayUIInstance;
                    params->CLOCKBUTTON_OFFSET_IN_TRAYUI = CLOCKBUTTON_OFFSET_IN_TRAYUI;
                    params->oldClockButtonInstance = oldClockButtonInstance;
                    SendMessageCallbackW(hShellTray_Wnd, WM_TOGGLE_CLOCK_FLYOUT, 0, 0, ClockButton_ToggleFlyoutCallback, params);
                }
            }
        }
    }
    return 0;
}
#endif
#pragma endregion


#pragma region "Enable old taskbar"
#ifdef _WIN64
DEFINE_GUID(GUID_18C02F2E_2754_5A20_8BD5_0B34CE79DA2B,
    0x18C02F2E,
    0x2754, 0x5A20, 0x8b, 0xd5,
    0x0b, 0x34, 0xce, 0x79, 0xda, 0x2b
);
HRESULT explorer_RoGetActivationFactoryHook(HSTRING activatableClassId, GUID* iid, void** factory)
{
    PCWSTR StringRawBuffer = WindowsGetStringRawBuffer(activatableClassId, 0);
    if (!wcscmp(StringRawBuffer, L"WindowsUdk.ApplicationModel.AppExtensions.XamlExtensions") && IsEqualGUID(iid, &GUID_18C02F2E_2754_5A20_8BD5_0B34CE79DA2B))
    {
        *factory = &XamlExtensionsFactory;
        return S_OK;
    }
    return RoGetActivationFactory(activatableClassId, iid, factory);
}

FARPROC explorer_GetProcAddressHook(HMODULE hModule, const CHAR* lpProcName)
{
    if ((*((WORD*)&(lpProcName)+1)) && !strncmp(lpProcName, "RoGetActivationFactory", 22))
        return (FARPROC)explorer_RoGetActivationFactoryHook;
    else
        return GetProcAddress(hModule, lpProcName);
}
#endif
#pragma endregion


#pragma region "Open power user menu on Win+X"
#ifdef _WIN64
LRESULT explorer_SendMessageW(HWND hWndx, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == 0x579) // "Raise desktop" - basically shows desktop or the windows
                       // wParam = 3 => show desktop
                       // wParam = 2 => raise windows
    {
        
    }
    else if (uMsg == TB_GETTEXTROWS)
    {
        HWND hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hWnd)
        {
            hWnd = FindWindowEx(
                hWnd,
                NULL,
                L"RebarWindow32",
                NULL
            );
            if (hWnd)
            {
                hWnd = FindWindowEx(
                    hWnd,
                    NULL,
                    L"MSTaskSwWClass",
                    NULL
                );
                if (hWnd && hWnd == hWndx && wParam == -1)
                {
                    if (hIsWinXShown)
                    {
                        SendMessage(hWinXWnd, WM_CLOSE, 0, 0);
                    }
                    else
                    {
                        hWnd = FindWindowEx(
                            NULL,
                            NULL,
                            L"Shell_TrayWnd",
                            NULL
                        );
                        if (hWnd)
                        {
                            hWnd = FindWindowEx(
                                hWnd,
                                NULL,
                                L"Start",
                                NULL
                            );
                            if (hWnd)
                            {
                                POINT pt = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE);
                                // Finally implemented a variation of
                                // https://github.com/valinet/ExplorerPatcher/issues/3
                                // inspired by how the real Start button activates this menu
                                // (CPearl::_GetLauncherTipContextMenu)
                                // This also works when auto hide taskbar is on (#63)
                                HRESULT hr = S_OK;
                                IUnknown* pImmersiveShell = NULL;
                                hr = CoCreateInstance(
                                    &CLSID_ImmersiveShell,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    &IID_IServiceProvider,
                                    &pImmersiveShell
                                );
                                if (SUCCEEDED(hr))
                                {
                                    IImmersiveMonitorService* pMonitorService = NULL;
                                    IUnknown_QueryService(
                                        pImmersiveShell,
                                        &SID_IImmersiveMonitorService,
                                        &IID_IImmersiveMonitorService,
                                        &pMonitorService
                                    );
                                    if (pMonitorService)
                                    {
                                        ILauncherTipContextMenu* pMenu = NULL;
                                        pMonitorService->lpVtbl->QueryServiceFromWindow(
                                            pMonitorService,
                                            hWnd,
                                            &IID_ILauncherTipContextMenu,
                                            &IID_ILauncherTipContextMenu,
                                            &pMenu
                                        );
                                        if (pMenu)
                                        {
                                            pMenu->lpVtbl->ShowLauncherTipContextMenu(
                                                pMenu,
                                                &pt
                                            );
                                            pMenu->lpVtbl->Release(pMenu);
                                        }
                                        pMonitorService->lpVtbl->Release(pMonitorService);
                                    }
                                    pImmersiveShell->lpVtbl->Release(pImmersiveShell);
                                }
                            }
                        }
                    }
                    return 0;
                }
            }
        }
    }
    return SendMessageW(hWndx, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Set up taskbar button hooks"
#ifdef _WIN64

DWORD ShouldShowWidgetsInsteadOfCortana()
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
        TEXT("TaskbarDa"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwVal,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS)
    {
        return dwVal;
    }
    return 0;
}

__int64 (*Widgets_OnClickFunc)(__int64 a1, __int64 a2) = 0;
__int64 Widgets_OnClickHook(__int64 a1, __int64 a2)
{
    if (ShouldShowWidgetsInsteadOfCortana() == 1)
    {
        ToggleWidgetsPanel();
        return 0;
    }
    else
    {
        if (Widgets_OnClickFunc)
        {
            return Widgets_OnClickFunc(a1, a2);
        }
        return 0;
    }
}

HRESULT (*Widgets_GetTooltipTextFunc)(__int64 a1, __int64 a2, __int64 a3, WCHAR* a4, UINT a5) = 0;
HRESULT WINAPI Widgets_GetTooltipTextHook(__int64 a1, __int64 a2, __int64 a3, WCHAR* a4, UINT a5)
{
    if (ShouldShowWidgetsInsteadOfCortana() == 1)
    {
        return SHLoadIndirectString(
            L"@{windows?ms-resource://Windows.UI.SettingsAppThreshold/SystemSettings/Resources/SystemSettings_DesktopTaskbar_Da2/DisplayName}",
            a4,
            a5,
            0
        );
    }
    else
    {
        if (Widgets_GetTooltipTextFunc)
        {
            return Widgets_GetTooltipTextFunc(a1, a2, a3, a4, a5);
        }
        return 0;
    }
}

void stub1(void* i)
{
}

static BOOL(*SetChildWindowNoActivateFunc)(HWND);
BOOL explorer_SetChildWindowNoActivateHook(HWND hWnd)
{
    TCHAR className[100];
    GetClassNameW(hWnd, className, 100);
    if (!wcscmp(className, L"ControlCenterButton"))
    {
        lpShouldDisplayCCButton = (BYTE*)(GetWindowLongPtrW(hWnd, 0) + 120);
        if (*lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = !bHideControlCenterButton;
        }
    }
    // get a look at vtable by searching for v_IsEnabled
    if (!wcscmp(className, L"TrayButton"))
    {
        uintptr_t Instance = *(uintptr_t*)GetWindowLongPtrW(hWnd, 0);
        uintptr_t TrayButton_GetComponentName = *(INT_PTR(WINAPI**)())(Instance + 304);
        if (!IsBadCodePtr(TrayButton_GetComponentName))
        {
            wchar_t* wszComponentName = (const WCHAR*)(*(uintptr_t (**)(void))(Instance + 304))();
            if (!wcscmp(wszComponentName, L"CortanaButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!Widgets_OnClickFunc) Widgets_OnClickFunc = *(uintptr_t*)(Instance + 160);
                *(uintptr_t*)(Instance + 160) = Widgets_OnClickHook;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
                VirtualProtect(Instance + 216, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!Widgets_GetTooltipTextFunc) Widgets_GetTooltipTextFunc = *(uintptr_t*)(Instance + 216);
                *(uintptr_t*)(Instance + 216) = Widgets_GetTooltipTextHook; // OnTooltipShow
                VirtualProtect(Instance + 216, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }
            else if (!wcscmp(wszComponentName, L"MultitaskingButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                *(uintptr_t*)(Instance + 160) = ToggleTaskView;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }
            /*else if (!wcscmp(wszComponentName, L"PeopleButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                *(uintptr_t*)(Instance + 160) = ToggleMainClockFlyout;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }*/
        }
    }
    return SetChildWindowNoActivateFunc(hWnd);
}
#endif
#pragma endregion


#pragma region "Hide Show desktop button"
#ifdef _WIN64
INT64 ShowDesktopSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, ShowDesktopSubclassProc, ShowDesktopSubclassProc);
    }
    else if (uMsg == WM_USER + 100)
    {
        LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (lRes > 0)
        {
            DWORD dwVal = 0, dwSize = sizeof(DWORD);
            if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                TEXT("TaskbarSd"),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal,
                (LPDWORD)(&dwSize)
            ) == ERROR_SUCCESS && !dwVal)
            {
                lRes = 0;
            }
        }
        return lRes;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Notify shell ready (fixes delay at logon)"
#ifdef _WIN64
DWORD SignalShellReady(DWORD wait)
{
    printf("Started \"Signal shell ready\" thread.\n");
    //UpdateStartMenuPositioning(MAKELPARAM(TRUE, TRUE));

    while (!wait && TRUE)
    {
        HWND hShell_TrayWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hShell_TrayWnd)
        {
            HWND hWnd = FindWindowEx(
                hShell_TrayWnd,
                NULL,
                L"Start",
                NULL
            );
            if (hWnd)
            {
                if (IsWindowVisible(hWnd))
                {
                    UpdateStartMenuPositioning(MAKELPARAM(TRUE, TRUE));
                    break;
                }
            }
        }
        Sleep(100);
    }

    if (!wait)
    {
        Sleep(600);
    }
    else
    {
        Sleep(wait);
    }

    HANDLE hEvent = CreateEventW(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (hEvent)
    {
        printf(">>> Signal shell ready.\n");
        SetEvent(hEvent);
    }

    printf("Ended \"Signal shell ready\" thread.\n");
    return 0;
}
#endif
#pragma endregion


#pragma region "Window Switcher"
#ifdef _WIN64
DWORD sws_IsEnabled = FALSE;

void sws_ReadSettings(sws_WindowSwitcher* sws)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        DWORD val = 0;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("AltTabSettings"),
            0,
            NULL,
            &val,
            &dwSize
        );
        sws_IsEnabled = (val == 2);
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\sws",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        if (sws)
        {
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("IncludeWallpaper"),
                0,
                NULL,
                &(sws->bIncludeWallpaper),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("RowHeight"),
                0,
                NULL,
                &(sws->dwRowHeight),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxWidth"),
                0,
                NULL,
                &(sws->dwMaxWP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxHeight"),
                0,
                NULL,
                &(sws->dwMaxHP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("ColorScheme"),
                0,
                NULL,
                &(sws->dwColorScheme),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("Theme"),
                0,
                NULL,
                &(sws->dwTheme),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("CornerPreference"),
                0,
                NULL,
                &(sws->dwCornerPreference),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("ShowDelay"),
                0,
                NULL,
                &(sws->dwShowDelay),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("PrimaryOnly"),
                0,
                NULL,
                &(sws->bPrimaryOnly),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("PerMonitor"),
                0,
                NULL,
                &(sws->bPerMonitor),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxWidthAbs"),
                0,
                NULL,
                &(sws->dwMaxAbsoluteWP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxHeightAbs"),
                0,
                NULL,
                &(sws->dwMaxAbsoluteHP),
                &dwSize
            );
            if (sws)
            {
                sws_WindowSwitcher_RefreshTheme(sws);
            }
        }
        RegCloseKey(hKey);
    }
}

DWORD WindowSwitcher(DWORD unused)
{
    if (!bOldTaskbar)
    {
        WaitForSingleObject(hWin11AltTabInitialized, INFINITE);
        Sleep(1000);
    }

    while (TRUE)
    {
        sws_ReadSettings(NULL);
        if (sws_IsEnabled)
        {
            sws_error_t err;
            sws_WindowSwitcher* sws = NULL;
            err = sws_error_Report(sws_error_GetFromInternalError(sws_WindowSwitcher_Initialize(&sws, FALSE)), NULL);
            sws_ReadSettings(sws);
            if (err == SWS_ERROR_SUCCESS)
            {
                sws_WindowSwitcher_RefreshTheme(sws);
                HANDLE hEvents[3];
                hEvents[0] = sws->hEvExit;
                hEvents[1] = hSwsSettingsChanged;
                hEvents[2] = hSwsOpacityMaybeChanged;
                while (TRUE)
                {
                    DWORD dwRes = MsgWaitForMultipleObjectsEx(
                        3,
                        hEvents,
                        INFINITE,
                        QS_ALLINPUT,
                        MWMO_INPUTAVAILABLE
                    );
                    if (dwRes == WAIT_OBJECT_0 + 0)
                    {
                        break;
                    }
                    if (dwRes == WAIT_OBJECT_0 + 1)
                    {
                        sws_ReadSettings(sws);
                        if (!sws_IsEnabled)
                        {
                            break;
                        }
                    }
                    else if (dwRes == WAIT_OBJECT_0 + 2)
                    {
                        sws_WindowSwitcher_RefreshTheme(sws);
                    }
                    else if (dwRes == WAIT_OBJECT_0 + 3)
                    {
                        MSG msg;
                        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                sws_WindowSwitcher_Clear(sws);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            WaitForSingleObject(
                hSwsSettingsChanged,
                INFINITE
            );
        }
    }
}
#endif
#pragma endregion


#pragma region "Load Settings from registry"
void WINAPI LoadSettings(BOOL bIsExplorer)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0, dwTemp = 0;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("AllocConsole"),
            0,
            NULL,
            &bAllocConsole,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        dwTemp = 0;
        RegQueryValueExW(
            hKey,
            TEXT("Memcheck"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (dwTemp)
        {
#if defined(DEBUG) | defined(_DEBUG)
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
            _CrtDumpMemoryLeaks();
#endif
            dwTemp = 0;
            RegSetValueExW(
                hKey,
                TEXT("Memcheck"),
                0,
                REG_DWORD,
                &dwTemp,
                sizeof(DWORD)
            );
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HideExplorerSearchBar"),
            0,
            NULL,
            &bHideExplorerSearchBar,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DisableImmersiveContextMenu"),
            0,
            NULL,
            &bDisableImmersiveContextMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ClassicThemeMitigations"),
            0,
            NULL,
            &bClassicThemeMitigations,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("SkinMenus"),
            0,
            NULL,
            &bSkinMenus,
            &dwSize
        );
        if (bIsExplorerProcess)
        {
            if (bAllocConsole)
            {
                FILE* conout;
                AllocConsole();
                freopen_s(
                    &conout,
                    "CONOUT$",
                    "w",
                    stdout
                );
            }
            else
            {
                FreeConsole();
            }
        }
        if (!bIsExplorer)
        {
            RegCloseKey(hKey);
            return;
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OldTaskbar"),
            0,
            NULL,
            &bOldTaskbar,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MicaEffectOnTitlebar"),
            0,
            NULL,
            &bMicaEffectOnTitlebar,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HideControlCenterButton"),
            0,
            NULL,
            &bHideControlCenterButton,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("FlyoutMenus"),
            0,
            NULL,
            &bFlyoutMenus,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("CenterMenus"),
            0,
            NULL,
            &bCenterMenus,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("SkinIcons"),
            0,
            NULL,
            &bSkinIcons,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ReplaceNetwork"),
            0,
            NULL,
            &bReplaceNetwork,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ExplorerReadyDelay"),
            0,
            NULL,
            &dwExplorerReadyDelay,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ArchiveMenu"),
            0,
            NULL,
            &bEnableArchivePlugin,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ClockFlyoutOnWinC"),
            0,
            NULL,
            &bClockFlyoutOnWinC,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DisableImmersiveContextMenu"),
            0,
            NULL,
            &bDisableImmersiveContextMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HookStartMenu"),
            0,
            NULL,
            &bHookStartMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("PropertiesInWinX"),
            0,
            NULL,
            &bPropertiesInWinX,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("NoMenuAccelerator"),
            0,
            NULL,
            &bNoMenuAccelerator,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("TaskbarMonitorOverride"),
            0,
            NULL,
            &bTaskbarMonitorOverride,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("IMEStyle"),
            0,
            NULL,
            &dwIMEStyle,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("UpdatePolicy"),
            0,
            NULL,
            &dwUpdatePolicy,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("IsUpdatePending"),
            0,
            NULL,
            &bShowUpdateToast,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ToolbarSeparators"),
            0,
            NULL,
            &bToolbarSeparators,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("TaskbarAutohideOnDoubleClick"),
            0,
            NULL,
            &bTaskbarAutohideOnDoubleClick,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OrbStyle"),
            0,
            NULL,
            &dwOrbStyle,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("EnableSymbolDownload"),
            0,
            NULL,
            &bEnableSymbolDownload,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        dwTemp = 0;
        RegQueryValueExW(
            hKey,
            TEXT("OpenPropertiesAtNextStart"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (dwTemp)
        {
#ifdef _WIN64
            LaunchPropertiesGUI(hModule);
#endif
        }
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MonitorOverride"),
            0,
            NULL,
            &bMonitorOverride,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OpenAtLogon"),
            0,
            NULL,
            &bOpenAtLogon,
            &dwSize
        );
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\sws",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\TabletTip\\1.7",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }
}

void Explorer_RefreshClockHelper(HWND hClockButton)
{
    INT64* ClockButtonInstance = (BYTE*)(GetWindowLongPtrW(hClockButton, 0)); // -> ClockButton
    // we call v_Initialize because all it does is to query the
    // registry and update the internal state to display seconds or not
    // to get the offset, simply inspect the vtable of ClockButton
    ((void(*)(void*))(*(INT64*)((*(INT64*)ClockButtonInstance) + 6 * sizeof(uintptr_t))))(ClockButtonInstance); // v_Initialize
    // we need to refresh the button; for the text to actually change, we need to set this:
    // inspect ClockButton::v_OnTimer
    *((BYTE*)ClockButtonInstance + 547) = 1;
    // then, we simply invalidate the area
    InvalidateRect(hClockButton, NULL, TRUE);
}

void Explorer_RefreshClock(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        HWND hTrayNotifyWnd = FindWindowExW(hShellTray_Wnd, NULL, L"TrayNotifyWnd", NULL);
        if (hTrayNotifyWnd)
        {
            HWND hClockButton = FindWindowExW(hTrayNotifyWnd, NULL, L"TrayClockWClass", NULL);
            if (hClockButton)
            {
                Explorer_RefreshClockHelper(hClockButton);
            }
        }
    }

    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowExW(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (hWnd)
        {
            HWND hClockButton = FindWindowExW(hWnd, NULL, L"ClockButton", NULL);
            if (hClockButton)
            {
                Explorer_RefreshClockHelper(hClockButton);
            }
        }
    } while (hWnd);
}

void WINAPI Explorer_RefreshUI(int unused)
{
    SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
    Explorer_RefreshClock(0);
}

void Explorer_TogglePeopleButton(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
        const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
        INT64* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);

        ((void(*)(void*))(*(INT64*)((*(INT64*)TrayUIInstance) + 57 * sizeof(uintptr_t))))(TrayUIInstance);
    }
}

void Explorer_ToggleTouchpad(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
        const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
        INT64* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);

        ((void(*)(void*))(*(INT64*)((*(INT64*)TrayUIInstance) + 60 * sizeof(uintptr_t))))(TrayUIInstance);
    }
}
#pragma endregion


#pragma region "Fix taskbar for classic theme and set Explorer window hooks"
HWND(*CreateWindowExWFunc)(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
    );
HWND CreateWindowExWHook(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
)
{
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayNotifyWnd"))
    {
        dwExStyle |= WS_EX_STATICEDGE;
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"NotifyIconOverflowWindow"))
    {
        dwExStyle |= WS_EX_STATICEDGE;
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"SysListView32") || !wcscmp(lpClassName, L"SysTreeView32"))) // !wcscmp(lpClassName, L"FolderView")
    {
        wchar_t wszClassName[200];
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            dwExStyle |= WS_EX_CLIENTEDGE;
        }
    }
    if (bIsExplorerProcess && bToolbarSeparators && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"ReBarWindow32"))
    {
        wchar_t wszClassName[200];
        GetClassNameW(hWndParent, wszClassName, 200);
        if (!wcscmp(wszClassName, L"Shell_TrayWnd"))
        {
            dwStyle |= RBS_BANDBORDERS;
        }
    }
    HWND hWnd = CreateWindowExWFunc(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam
    );
#ifdef _WIN64
    if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"TrayClockWClass") || !wcscmp(lpClassName, L"ClockButton")))
    {
        SetWindowSubclass(hWnd, ClockButtonSubclassProc, ClockButtonSubclassProc, 0);
    }
    else if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayShowDesktopButtonWClass"))
    {
        SetWindowSubclass(hWnd, ShowDesktopSubclassProc, ShowDesktopSubclassProc, 0);
    }
    else if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"Shell_TrayWnd"))
    {
        SetWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc, 0);
    }
#endif
    /*
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"FolderView")))
    {
        wchar_t wszClassName[200];
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            SendMessageW(hWnd, 0x108, 0, 0);
        }
    }
    */
    //SetWindowTheme(hWnd, L" ", L" ");
    return hWnd;
}

LONG_PTR(*SetWindowLongPtrWFunc)(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
    );
LONG_PTR SetWindowLongPtrWHook(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
)
{
    WCHAR lpClassName[200];
    GetClassNameW(hWnd, lpClassName, 200);
    HWND hWndParent = GetParent(hWnd);

    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayNotifyWnd"))
    {
        if (nIndex == GWL_EXSTYLE)
        {
            dwNewLong |= WS_EX_STATICEDGE;
        }
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"NotifyIconOverflowWindow"))
    {
        if (nIndex == GWL_EXSTYLE)
        {
            dwNewLong |= WS_EX_STATICEDGE;
        }
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"SysListView32") || !wcscmp(lpClassName, L"SysTreeView32"))) // !wcscmp(lpClassName, L"FolderView")
    {
        wchar_t wszClassName[200];
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            if (nIndex == GWL_EXSTYLE)
            {
                dwNewLong |= WS_EX_CLIENTEDGE;
            }
        }
    }
    if (bIsExplorerProcess && bToolbarSeparators && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"ReBarWindow32"))
    {
        wchar_t wszClassName[200];
        GetClassNameW(hWndParent, wszClassName, 200);
        if (!wcscmp(wszClassName, L"Shell_TrayWnd"))
        {
            if (nIndex == GWL_STYLE)
            {
                dwNewLong |= RBS_BANDBORDERS;
            }
        }
    }
    return SetWindowLongPtrWFunc(hWnd, nIndex, dwNewLong);
}

#ifdef _WIN64
HRESULT (*explorer_SetWindowThemeFunc)(
    HWND    hwnd,
    LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList
);
HRESULT explorer_SetWindowThemeHook(
    HWND    hwnd,
    LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList
)
{
    if (bClassicThemeMitigations)
    {
        printf("SetWindowTheme\n");
        return explorer_SetWindowThemeFunc(hwnd, L" ", L" ");
    }
    return explorer_SetWindowThemeFunc(hwnd, pszSubAppName, pszSubIdList);
}

HTHEME hStartOrbTheme = NULL;
HRESULT explorer_DrawThemeBackground(
    HTHEME  hTheme,
    HDC     hdc,
    int     iPartId,
    int     iStateId,
    LPCRECT pRect,
    LPCRECT pClipRect
)
{
    if (dwOrbStyle == ORB_STYLE_WINDOWS11 && hStartOrbTheme == hTheme)
    {
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(BITMAPINFO));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = 1;
        bi.bmiHeader.biHeight = 1;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        RGBQUAD transparent = { 0, 0, 0, 0 };
        RGBQUAD color = { 0xFF, 0xFF, 0xFF, 0xFF };

        StretchDIBits(hdc,
            pRect->left,
            pRect->top,
            pRect->right - pRect->left,
            pRect->bottom - pRect->top,
            0, 0, 1, 1, &color, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdc, 
            pRect->left + ((pRect->right - pRect->left) / 2) - ORB_WINDOWS11_SEPARATOR / 2, 
            pRect->top,
            ORB_WINDOWS11_SEPARATOR,
            pRect->bottom - pRect->top,
            0, 0, 1, 1, &transparent, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdc,
            pRect->left,
            pRect->top + ((pRect->bottom - pRect->top) / 2) - ORB_WINDOWS11_SEPARATOR / 2,
            pRect->right - pRect->left,
            ORB_WINDOWS11_SEPARATOR,
            0, 0, 1, 1, &transparent, &bi,
            DIB_RGB_COLORS, SRCCOPY);

        return S_OK;
    }
    if (bClassicThemeMitigations)
    {
        if (iPartId == 4 && iStateId == 1)
        {
            COLORREF bc = GetBkColor(hdc);
            COLORREF fc = GetTextColor(hdc);
            int mode = SetBkMode(hdc, TRANSPARENT);

            SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

            NONCLIENTMETRICSW ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICSW);
            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);

            HFONT hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));

            UINT dpiX, dpiY;
            HRESULT hr = GetDpiForMonitor(
                MonitorFromWindow(WindowFromDC(hdc), MONITOR_DEFAULTTOPRIMARY),
                MDT_DEFAULT,
                &dpiX,
                &dpiY
            );
            double dx = dpiX / 96.0, dy = dpiY / 96.0;

            HGDIOBJ hOldFont = SelectObject(hdc, hFont);
            DWORD dwTextFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
            RECT rc = *pRect;
            rc.bottom -= 7 * dy;
            DrawTextW(
                hdc,
                L"\u2026",
                -1, 
                &rc,
                dwTextFlags
            );
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            SetBkColor(hdc, bc);
            SetTextColor(hdc, fc);
            SetBkMode(hdc, mode);
        }
        return S_OK;
    }
    return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

INT64 explorer_SetWindowCompositionAttribute(HWND hWnd, WINCOMPATTRDATA* data)
{
    if (bClassicThemeMitigations)
    {
        return TRUE;
    }
    return SetWindowCompositionAttribute(hWnd, data);
}

HTHEME explorer_OpenThemeDataForDpi(
    HWND    hwnd,
    LPCWSTR pszClassList,
    UINT    dpi
)
{
    if (dwOrbStyle == ORB_STYLE_WINDOWS11 && (*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"TaskbarPearl"))
    {
        HTHEME hTheme = OpenThemeDataForDpi(hwnd, pszClassList, dpi);
        if (hTheme)
        {
            hStartOrbTheme = hTheme;
        }
        return hTheme;
    }

    // task list - Taskband2 from CTaskListWnd::_HandleThemeChanged
    if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"Taskband2"))
    {
        return 0xDeadBeef;
    }
    // system tray notification area more icons
    else if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"TrayNotifyFlyout"))
    {
        return 0xABadBabe;
    }
    /*else if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && wcsstr(pszClassList, L"::Taskband2"))
    {
        wprintf(L"%s\n", pszClassList);
        return 0xB16B00B5;
    }*/
    return OpenThemeDataForDpi(hwnd, pszClassList, dpi);
}

HRESULT explorer_GetThemeMetric(
    HTHEME hTheme,
    HDC    hdc,
    int    iPartId,
    int    iStateId,
    int    iPropId,
    int* piVal
)
{
    if (!bClassicThemeMitigations || (hTheme != 0xABadBabe))
    {
        return GetThemeMetric(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            piVal
        );
    }
    const int TMT_WIDTH = 2416;
    const int TMT_HEIGHT = 2417;
    if (hTheme == 0xABadBabe && iPropId == TMT_WIDTH && iPartId == 3 && iStateId == 0)
    {
        *piVal = GetSystemMetrics(SM_CXICON);
    }
    else if (hTheme == 0xABadBabe && iPropId == TMT_HEIGHT && iPartId == 3 && iStateId == 0)
    {
        *piVal = GetSystemMetrics(SM_CYICON);
    }
    return S_OK;
}

HRESULT explorer_GetThemeMargins(
    HTHEME  hTheme,
    HDC     hdc,
    int     iPartId,
    int     iStateId,
    int     iPropId,
    LPCRECT prc,
    MARGINS* pMargins
)
{
    if (!bClassicThemeMitigations || (hTheme != 0xDeadBeef && hTheme != 0xABadBabe))
    {
        HRESULT hr = GetThemeMargins(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            prc,
            pMargins
        );
        return hr;
    }
    const int TMT_SIZINGMARGINS = 3601;
    const int TMT_CONTENTMARGINS = 3602;
    HRESULT hr = S_OK;
    if (hTheme)
    {
        hr = GetThemeMargins(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            prc,
            pMargins
        );
    }
    /*if (hTheme == 0xB16B00B5)
    {
        printf(
            "GetThemeMargins %d %d %d - %d %d %d %d\n", 
            iPartId, 
            iStateId, 
            iPropId, 
            pMargins->cxLeftWidth, 
            pMargins->cyTopHeight, 
            pMargins->cxRightWidth, 
            pMargins->cyBottomHeight
        );
    }*/
    if (hTheme == 0xDeadBeef && iPropId == TMT_CONTENTMARGINS && iPartId == 5 && iStateId == 1)
    {
        // task list button measurements
        pMargins->cxLeftWidth = 4;
        pMargins->cyTopHeight = 3;
        pMargins->cxRightWidth = 4;
        pMargins->cyBottomHeight = 3;
    }
    else if (hTheme == 0xDeadBeef && iPropId == TMT_CONTENTMARGINS && iPartId == 1 && iStateId == 0)
    {
        // task list measurements
        pMargins->cxLeftWidth = 0;
        pMargins->cyTopHeight = 0;
        pMargins->cxRightWidth = 4;
        pMargins->cyBottomHeight = 0;
    }
    else if (hTheme == 0xDeadBeef && iPropId == TMT_SIZINGMARGINS && iPartId == 5 && iStateId == 1)
    {
        pMargins->cxLeftWidth = 10;
        pMargins->cyTopHeight = 10;
        pMargins->cxRightWidth = 10;
        pMargins->cyBottomHeight = 10;
    }
    else if (hTheme = 0xABadBabe && iPropId == TMT_CONTENTMARGINS && iPartId == 3 && iStateId == 0)
    {
        pMargins->cxLeftWidth = 6;// GetSystemMetrics(SM_CXICONSPACING);
        pMargins->cyTopHeight = 6;// GetSystemMetrics(SM_CYICONSPACING);
        pMargins->cxRightWidth = 6;//GetSystemMetrics(SM_CXICONSPACING);
        pMargins->cyBottomHeight = 6;// GetSystemMetrics(SM_CYICONSPACING);
    }
    HWND hShell_TrayWnd = FindWindowEx(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShell_TrayWnd)
    {
        LONG dwStyle = 0;
        dwStyle = GetWindowLongW(hShell_TrayWnd, GWL_STYLE);
        dwStyle |= WS_DLGFRAME;
        SetWindowLongW(hShell_TrayWnd, GWL_STYLE, dwStyle);
        dwStyle &= ~WS_DLGFRAME;
        SetWindowLongW(hShell_TrayWnd, GWL_STYLE, dwStyle);
    }
    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowEx(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (hWnd)
        {
            LONG dwStyle = 0;
            dwStyle = GetWindowLongW(hWnd, GWL_STYLE);
            dwStyle |= WS_DLGFRAME;
            SetWindowLongW(hWnd, GWL_STYLE, dwStyle);
            dwStyle &= ~WS_DLGFRAME;
            SetWindowLongW(hWnd, GWL_STYLE, dwStyle);
        }
    } while (hWnd);
    return S_OK;
}

HRESULT explorer_DrawThemeTextEx(
    HTHEME        hTheme,
    HDC           hdc,
    int           iPartId,
    int           iStateId,
    LPCWSTR       pszText,
    int           cchText,
    DWORD         dwTextFlags,
    LPRECT        pRect,
    const DTTOPTS* pOptions
)
{
    if (!bClassicThemeMitigations)
    {
        return DrawThemeTextEx(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            pszText,
            cchText,
            dwTextFlags,
            pRect,
            pOptions
        );
    }

    COLORREF bc = GetBkColor(hdc);
    COLORREF fc = GetTextColor(hdc);
    int mode = SetBkMode(hdc, TRANSPARENT);
    
    wchar_t text[200];
    GetWindowTextW(GetForegroundWindow(), text, 200);

    BOOL bIsActiveUnhovered = (iPartId == 5 && iStateId == 5);
    BOOL bIsInactiveUnhovered = (iPartId == 5 && iStateId == 1);
    BOOL bIsInactiveHovered = (iPartId == 5 && iStateId == 2);
    BOOL bIsActiveHovered = bIsInactiveHovered && !wcscmp(text, pszText);

    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);

    HFONT hFont = NULL;
    if (bIsActiveUnhovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));
    }
    else if (bIsInactiveUnhovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
    }
    else if (bIsActiveHovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));
    }
    else if (bIsInactiveHovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
    }
    else
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
        //wprintf(L"DrawThemeTextEx %d %d %s\n", iPartId, iStateId, pszText);
    }

    if (iPartId == 5 && iStateId == 0) // clock
    {
        pRect->top += 2;
    }

    HGDIOBJ hOldFont = SelectObject(hdc, hFont);
    DrawTextW(
        hdc,
        pszText,
        cchText, 
        pRect,
        dwTextFlags
    );
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    SetBkColor(hdc, bc);
    SetTextColor(hdc, fc);
    SetBkMode(hdc, mode);
    return S_OK;
}
#endif
#pragma endregion


#pragma region "Change clock links"
#ifdef _WIN64
HINSTANCE explorer_ShellExecuteW(
    HWND    hwnd,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT     nShowCmd
)
{
    if (!wcscmp(lpFile, L"ms-settings:notifications"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{05d7b0f4-2121-4eff-bf6b-ed3f69b894d9}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    else if (!wcscmp(lpFile, L"ms-settings:dateandtime"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{E2E7934B-DCE5-43C4-9576-7FE4F75E7480}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    /*else if (!wcscmp(lpFile, L"ms-settings:taskbar"))
    {
        LaunchPropertiesGUI(hModule);
        return 0;
    }*/
    return ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}
#endif
#pragma endregion


#pragma region "Change language UI style"
DEFINE_GUID(CLSID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x86,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

DEFINE_GUID(IID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x82,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

#define LANGUAGEUI_STYLE_DESKTOP 0
#define LANGUAGEUI_STYLE_TOUCHKEYBOARD 1
#define LANGUAGEUI_STYLE_LOGONUI 2
#define LANGUAGEUI_STYLE_UAC 3
#define LANGUAGEUI_STYLE_SETTINGSPANE 4
#define LANGUAGEUI_STYLE_OOBE 5
#define LANGUAGEUI_STYLE_OTHER 100

char mov_edx_val[6] = { 0xBA, 0x00, 0x00, 0x00, 0x00, 0xC3 };
char* ep_pf = NULL;

HRESULT explorer_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID*   ppv
)
{
    if (IsEqualCLSID(rclsid, &CLSID_InputSwitchControl) && IsEqualIID(riid, &IID_InputSwitchControl))
    {
        HRESULT hr = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
        if (SUCCEEDED(hr))
        {
            // Pff... how this works:
            // 
            // * This `CoCreateInstance` call will get a pointer to an IInputSwitchControl interface
            // (the call to here is made from `explorer!CTrayInputIndicator::_RegisterInputSwitch`);
            // the next call on this pointer will be on the `IInputSwitchControl::Init` function.
            // 
            // * `IInputSwitchControl::Init`'s second parameter is a number (x) which tells which
            // language switcher UI to prepare (check `IsUtil::MapClientTypeToString` in
            // `InputSwitch.dll`). "explorer" requests the "DESKTOP" UI (x = 0), which is the new
            // Windows 11 UI; if we replace that number with something else, some other UI will
            // be created
            // 
            // * We cannot patch the vtable of the COM object because the executable is protected
            // by control flow guard and we would make a jump to an invalid site (maybe there is
            // some clever workaround fpr this as well, somehow telling the compiler to place a certain
            // canary before our trampoline, so it matches with what the runtime support for CFG expects,
            // but we'd have to keep that canary in sync with the one in explorer.exe, so not very
            // future proof).
            // 
            // * Taking advantage of the fact that the call to `IInputSwitchControl::Init` is the thing
            // that happens right after we return from here, and looking on the disassembly, we see nothing
            // else changes `rdx` (which is the second argument to a function call), basically x, besides the
            // very `xor edx, edx` instruction before the call. Thus, we patch that out, and we also do
            // `mov edx, whatever` here; afterwards, we do NOTHING else, but just return and hope that
            // edx will stick
            // 
            // * Needless to say this is **HIGHLY** amd64
            char pattern[2] = { 0x33, 0xD2 };
            DWORD dwOldProtect;
            char* p_mov_edx_val = mov_edx_val;
            if (!ep_pf)
            {
                ep_pf = memmem(_ReturnAddress(), 200, pattern, 2);
                if (ep_pf)
                {
                    // Cancel out `xor edx, edx`
                    VirtualProtect(ep_pf, 2, PAGE_EXECUTE_READWRITE, &dwOldProtect);
                    memset(ep_pf, 0x90, 2);
                    VirtualProtect(ep_pf, 2, dwOldProtect, &dwOldProtect);
                }
                VirtualProtect(p_mov_edx_val, 6, PAGE_EXECUTE_READWRITE, &dwOldProtect);
            }
            if (ep_pf)
            {
                // Craft a "function" which does `mov edx, whatever; ret` and call it
                DWORD* pVal = mov_edx_val + 1;
                *pVal = dwIMEStyle;
                void(*pf_mov_edx_val)() = p_mov_edx_val;
                pf_mov_edx_val();
            }
        }
        return hr;
    }
    return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
#pragma endregion


#pragma region "Explorer Registry Hooks"
LSTATUS explorer_RegCreateKeyExW(HKEY a1, const WCHAR* a2,  DWORD a3, WCHAR* a4, DWORD a5, REGSAM a6, struct _SECURITY_ATTRIBUTES* a7, HKEY* a8, DWORD* a9)
{
    const wchar_t* v13; // rdx
    int v14; // eax

    if (lstrcmpW(a2, L"MMStuckRects3"))
    {
        v14 = lstrcmpW(a2, L"StuckRects3");
        v13 = L"StuckRectsLegacy";
        if (v14)
        {
            v13 = a2;
        }
    }
    else
    {
        v13 = L"MMStuckRectsLegacy";
    }

    return RegCreateKeyExW(a1, v13, a3, a4, a5, a6, a7, a8, a9);
}

LSTATUS explorer_SHGetValueW(HKEY a1, const WCHAR* a2, const WCHAR* a3, DWORD* a4, void* a5, DWORD* a6)
{
    const WCHAR* v10; // rdx
    int v11; // eax

    if (lstrcmpW(a2, L"MMStuckRects3"))
    {
        v11 = lstrcmpW(a2, L"StuckRects3");
        v10 = L"StuckRectsLegacy";
        if (v11)
            v10 = a2;
    }
    else
    {
        v10 = L"MMStuckRectsLegacy";
    }

    return SHGetValueW(a1, v10, a3, a4, a5, a6);
}

LSTATUS explorer_OpenRegStream(HKEY hkey, PCWSTR pszSubkey, PCWSTR pszValue, DWORD grfMode)
{
    DWORD flOldProtect[6];

    if (!lstrcmpiW(pszValue, L"TaskbarWinXP")
        && VirtualProtect(pszValue, 0xC8ui64, 0x40u, flOldProtect))
    {
        lstrcpyW(pszValue, L"TaskbarWinEP");
        VirtualProtect(pszValue, 0xC8ui64, flOldProtect[0], flOldProtect);
    }

    return OpenRegStream(hkey, pszSubkey, pszValue, grfMode);
}

LSTATUS explorer_RegOpenKeyExW(HKEY a1, WCHAR* a2, DWORD a3, REGSAM a4, HKEY* a5)
{
    DWORD flOldProtect[6];

    if (!lstrcmpiW(a2, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify")
        && VirtualProtect(a2, 0xC8ui64, 0x40u, flOldProtect))
    {
        lstrcpyW(a2, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotSIB");
        VirtualProtect(a2, 0xC8ui64, flOldProtect[0], flOldProtect);
    }

    return RegOpenKeyExW(a1, a2, a3, a4, a5);
}

LSTATUS explorer_RegSetValueExW(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE* lpData,
    DWORD      cbData
)
{
    if (!lstrcmpW(lpValueName, L"ShowCortanaButton"))
    {
        if (cbData == sizeof(DWORD) && *(DWORD*)lpData == 1)
        {
            DWORD dwData = 2;
            return RegSetValueExW(hKey, L"TaskbarDa", Reserved, dwType, &dwData, cbData);
        }
        return RegSetValueExW(hKey, L"TaskbarDa", Reserved, dwType, lpData, cbData);
    }

    return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LSTATUS explorer_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    DWORD flOldProtect;
    BOOL bShowTaskViewButton = FALSE;
    LSTATUS lRes;

    if (!lstrcmpW(lpValue, L"ShowCortanaButton"))
    {
        lRes = RegGetValueW(hkey, lpSubKey, L"TaskbarDa", dwFlags, pdwType, pvData, pcbData);
        if (*(DWORD*)pvData == 2)
        {
            *(DWORD*)pvData = 1;
        }
    }
    /*else if (!lstrcmpW(lpValue, L"PeopleBand"))
    {
        lRes = RegGetValueW(hkey, lpSubKey, L"TaskbarMn", dwFlags, pdwType, pvData, pcbData);
    }*/
    else
    {
        lRes = RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }

    if (!lstrcmpW(lpValue, L"SearchboxTaskbarMode"))
    {
        if (*(DWORD*)pvData)
        {
            *(DWORD*)pvData = 1;
        }

        lRes = ERROR_SUCCESS;
    }

    return lRes;
}

LSTATUS twinuipcshell_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    LSTATUS lRes = RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);

    if (!bOldTaskbar && !lstrcmpW(lpValue, L"AltTabSettings"))
    {
        if (*(DWORD*)pvData)
        {
            *(DWORD*)pvData = 1;
        }

        if (hWin11AltTabInitialized)
        {
            SetEvent(hWin11AltTabInitialized);
            CloseHandle(hWin11AltTabInitialized);
            hWin11AltTabInitialized = NULL;
        }

        lRes = ERROR_SUCCESS;
    }

    return lRes;
}

BOOL CALLBACK GetMonitorByIndex(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, RECT* rc)
{
    //printf(">> %d %d %d %d\n", lprcMonitor->left, lprcMonitor->top, lprcMonitor->right, lprcMonitor->bottom);
    if (--rc->left < 0)
    {
        *rc = *lprcMonitor;
        return FALSE;
    }
    return TRUE;
}

HMONITOR explorer_MonitorFromRect(LPCRECT lprc, DWORD dwFlags)
{
    /*printf("%d %d %d %d\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

        return MonitorFromRect(lprc, dwFlags);
    //}*/
    if (bTaskbarMonitorOverride)
    {
        RECT rc;
        ZeroMemory(&rc, sizeof(RECT));
        rc.left = bTaskbarMonitorOverride - 1;
        EnumDisplayMonitors(
            NULL,
            NULL,
            GetMonitorByIndex,
            &rc
        );
        if (rc.top != rc.bottom)
        {
            return MonitorFromRect(&rc, dwFlags);
        }
    }
    return MonitorFromRect(lprc, dwFlags);
}

HRESULT (*explorer_SHCreateStreamOnModuleResourceWFunc)(
    HMODULE hModule,
    LPCWSTR pwszName,
    LPCWSTR pwszType,
    IStream** ppStream
);

HRESULT WINAPI explorer_SHCreateStreamOnModuleResourceWHook(
    HMODULE hModule,
    LPCWSTR pwszName,
    LPCWSTR pwszType,
    IStream** ppStream
)
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    if ((*((WORD*)&(pwszName)+1)))
    {
        wprintf(L"%s - %s %s\n", path, pwszName, pwszType);
    }
    else
    {
        wprintf(L"%s - %d %s\n", path, pwszName, pwszType);

        IStream* pStream = NULL;
        if (pwszName < 124)
        {
            if (S_Icon_Dark_TaskView)
            {
                pStream = SHCreateMemStream(P_Icon_Dark_TaskView, S_Icon_Dark_TaskView);
                if (pStream)
                {
                    *ppStream = pStream;
                    return S_OK;
                }
            }
        }
        else if (pwszName >= 151)
        {
            if (pwszName < 163)
            {
                if (S_Icon_Dark_Search)
                {
                    pStream = SHCreateMemStream(P_Icon_Dark_Search, S_Icon_Dark_Search);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 201)
            {
                if (S_Icon_Light_Search)
                {
                    pStream = SHCreateMemStream(P_Icon_Light_Search, S_Icon_Light_Search);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 213)
            {
                if (S_Icon_Dark_Widgets)
                {
                    printf(">>> %p %d\n", P_Icon_Dark_Widgets, S_Icon_Dark_Widgets);
                    pStream = SHCreateMemStream(P_Icon_Dark_Widgets, S_Icon_Dark_Widgets);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 251)
            {
                if (S_Icon_Light_Widgets)
                {
                    pStream = SHCreateMemStream(P_Icon_Light_Widgets, S_Icon_Light_Widgets);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }
        }
        else if (pwszName < 307)
        {
            if (S_Icon_Light_TaskView)
            {
                pStream = SHCreateMemStream(P_Icon_Light_TaskView, S_Icon_Light_TaskView);
                if (pStream)
                {
                    *ppStream = pStream;
                    return S_OK;
                }
            }
        }
    }
    return explorer_SHCreateStreamOnModuleResourceWFunc(hModule, pwszName, pwszType, ppStream);
}
#pragma endregion


DWORD InjectBasicFunctions(BOOL bIsExplorer, BOOL bInstall)
{
    //Sleep(150);

    HMODULE hShlwapi = LoadLibraryW(L"Shlwapi.dll");
    if (bInstall)
    {
        SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hShlwapi, "SHRegGetValueFromHKCUHKLM");
    }
    else
    {
        FreeLibrary(hShlwapi);
        FreeLibrary(hShlwapi);
    }

    HANDLE hShell32 = LoadLibraryW(L"shell32.dll");
    if (bInstall)
    {
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchIAT(hShell32, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hShell32, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hShell32, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
    }
    else
    {
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hShell32, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hShell32, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hShell32, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        FreeLibrary(hShell32);
        FreeLibrary(hShell32);
    }

    HANDLE hShcore = LoadLibraryW(L"shcore.dll");
    if (bInstall)
    {
        explorerframe_SHCreateWorkerWindowFunc = GetProcAddress(hShcore, (LPCSTR)188);
    }
    else
    {
        FreeLibrary(hShcore);
        FreeLibrary(hShcore);
    }

    HANDLE hExplorerFrame = LoadLibraryW(L"ExplorerFrame.dll");
    if (bInstall)
    {
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchIAT(hExplorerFrame, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowHook);  // <<<SAB>>>
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hExplorerFrame, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hExplorerFrame, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
    }
    else
    {
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hExplorerFrame, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowFunc);
        if (!bIsExplorer)
        {
            VnPatchIAT(hExplorerFrame, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hExplorerFrame, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        FreeLibrary(hExplorerFrame);
        FreeLibrary(hExplorerFrame);
    }

    HANDLE hWindowsUIFileExplorer = LoadLibraryW(L"Windows.UI.FileExplorer.dll");
    if (hWindowsUIFileExplorer)
    {
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
    }
    else
    {
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        FreeLibrary(hWindowsUIFileExplorer);
        FreeLibrary(hWindowsUIFileExplorer);
    }
}

DWORD Inject(BOOL bIsExplorer)
{
#if defined(DEBUG) | defined(_DEBUG)
    FILE* conout;
    AllocConsole();
    freopen_s(
        &conout, 
        "CONOUT$",
        "w", 
        stdout
    );
#endif

    int rv;

    LoadSettings(bIsExplorer);

#ifdef _WIN64
    if (bIsExplorer)
    {
        funchook = funchook_create();
        printf("funchook create %d\n", funchook != 0);
    }
#endif

    if (bIsExplorer)
    {
        hSwsSettingsChanged = CreateEventW(NULL, FALSE, FALSE, NULL);
        hSwsOpacityMaybeChanged = CreateEventW(NULL, FALSE, FALSE, NULL);
    }

    unsigned int numSettings = bIsExplorer ? 11 : 2;
    Setting* settings = calloc(numSettings, sizeof(Setting));
    if (settings)
    {
        unsigned int cs = 0;

        if (cs < numSettings)
        {
            settings[cs].callback = NULL;
            settings[cs].data = NULL;
            settings[cs].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
            settings[cs].hKey = NULL;
            ZeroMemory(settings[cs].name, MAX_PATH);
            settings[cs].origin = NULL;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = LoadSettings;
            settings[cs].data = bIsExplorer;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, TEXT(REGPATH));
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = LoadSettings;
            settings[cs].data = bIsExplorer;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsSettingsChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, TEXT(REGPATH) L"\\sws");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsOpacityMaybeChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\TabletTip\\1.7");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsSettingsChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = UpdateStartMenuPositioning;
            settings[cs].data = MAKELPARAM(FALSE, TRUE);
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        SettingsChangeParameters* settingsParams = calloc(1, sizeof(SettingsChangeParameters));
        if (settingsParams)
        {
            settingsParams->settings = settings;
            settingsParams->size = numSettings;
            settingsParams->hThread = CreateThread(
                0,
                0,
                MonitorSettings,
                settingsParams,
                0,
                0
            );
        }
        else
        {
            if (numSettings && settings[0].hEvent)
            {
                CloseHandle(settings[0].hEvent);
            }
            free(settings);
            settings = NULL;
        }
    }

    InjectBasicFunctions(bIsExplorer, TRUE);
    //if (!hDelayedInjectionThread)
    //{
    //    hDelayedInjectionThread = CreateThread(0, 0, InjectBasicFunctions, 0, 0, 0);
    //}

    if (!bIsExplorer)
    {
        return;
    }

#ifdef _WIN64
    if (bIsExplorer)
    {
        hWin11AltTabInitialized = CreateEventW(NULL, FALSE, FALSE, NULL);
        CreateThread(
            0,
            0,
            WindowSwitcher,
            0,
            0,
            0
        );
    }


#ifdef USE_PRIVATE_INTERFACES
    P_Icon_Dark_Search = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Search_Dark\\png\\32.png", &S_Icon_Dark_Search);
    P_Icon_Light_Search = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Search_Light\\png\\32.png", &S_Icon_Light_Search);
    P_Icon_Dark_TaskView = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\TaskView_Dark\\png\\32.png", &S_Icon_Dark_TaskView);
    P_Icon_Light_TaskView = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\TaskView_Light\\png\\32.png", &S_Icon_Light_TaskView);
    P_Icon_Dark_Widgets = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Widgets_Dark\\png\\32.png", &S_Icon_Dark_Widgets);
    P_Icon_Light_Widgets = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Widgets_Light\\png\\32.png", &S_Icon_Dark_Widgets);
#endif


    symbols_addr symbols_PTRS;
    ZeroMemory(
        &symbols_PTRS,
        sizeof(symbols_addr)
    );
    if (LoadSymbols(&symbols_PTRS, hModule))
    {
        if (bEnableSymbolDownload)
        {
            printf("Attempting to download symbol data; for now, the program may have limited functionality.\n");
            DownloadSymbolsParams* params = malloc(sizeof(DownloadSymbolsParams));
            params->hModule = hModule;
            params->bVerbose = FALSE;
            CreateThread(0, 0, DownloadSymbols, params, 0, 0);
        }
    }
    else
    {
        printf("Loaded symbols\n");
    }


    HANDLE hUser32 = LoadLibraryW(L"user32.dll");
    CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");
    GetWindowBand = GetProcAddress(hUser32, "GetWindowBand");
    SetWindowBand = GetProcAddress(hUser32, "SetWindowBand");
    SetWindowCompositionAttribute = GetProcAddress(hUser32, "SetWindowCompositionAttribute");
    printf("Setup user32 functions done\n");


    HANDLE hExplorer = GetModuleHandleW(NULL);
    SetChildWindowNoActivateFunc = GetProcAddress(GetModuleHandleW(L"user32.dll"), (LPCSTR)2005);
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "user32.dll", (LPCSTR)2005, explorer_SetChildWindowNoActivateHook);
        VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "SendMessageW", explorer_SendMessageW);
        VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "GetProcAddress", explorer_GetProcAddressHook);
        VnPatchIAT(hExplorer, "shell32.dll", "ShellExecuteW", explorer_ShellExecuteW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", explorer_RegGetValueW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegSetValueExW", explorer_RegSetValueExW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegCreateKeyExW", explorer_RegCreateKeyExW);
        VnPatchIAT(hExplorer, "API-MS-WIN-SHCORE-REGISTRY-L1-1-0.DLL", "SHGetValueW", explorer_SHGetValueW);
        VnPatchIAT(hExplorer, "user32.dll", "MonitorFromRect", explorer_MonitorFromRect);
        VnPatchIAT(hExplorer, "user32.dll", "LoadMenuW", explorer_LoadMenuW);
    }
    VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegOpenKeyExW", explorer_RegOpenKeyExW);
    VnPatchIAT(hExplorer, "shell32.dll", (LPCSTR)85, explorer_OpenRegStream);
    VnPatchIAT(hExplorer, "user32.dll", "TrackPopupMenuEx", explorer_TrackPopupMenuExHook);
    VnPatchIAT(hExplorer, "uxtheme.dll", "OpenThemeDataForDpi", explorer_OpenThemeDataForDpi);
    VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeBackground", explorer_DrawThemeBackground);
    if (bClassicThemeMitigations)
    {
        /*explorer_SetWindowThemeFunc = SetWindowTheme;
        rv = funchook_prepare(
            funchook,
            (void**)&explorer_SetWindowThemeFunc,
            explorer_SetWindowThemeHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }*/
        VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeTextEx", explorer_DrawThemeTextEx);
        VnPatchIAT(hExplorer, "uxtheme.dll", "GetThemeMargins", explorer_GetThemeMargins);
        VnPatchIAT(hExplorer, "uxtheme.dll", "GetThemeMetric", explorer_GetThemeMetric);
        //VnPatchIAT(hExplorer, "uxtheme.dll", "OpenThemeDataForDpi", explorer_OpenThemeDataForDpi);
        //VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeBackground", explorer_DrawThemeBackground);
        VnPatchIAT(hExplorer, "user32.dll", "SetWindowCompositionAttribute", explorer_SetWindowCompositionAttribute);
    }
    //VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "CreateWindowExW", explorer_CreateWindowExW);
    if (bOldTaskbar && dwIMEStyle)
    {
        VnPatchIAT(hExplorer, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", explorer_CoCreateInstanceHook);
    }


#ifdef USE_PRIVATE_INTERFACES
    HANDLE hShcore = LoadLibraryW(L"shcore.dll");
    explorer_SHCreateStreamOnModuleResourceWFunc = GetProcAddress(hShcore, (LPCSTR)109);
    VnPatchIAT(hExplorer, "shcore.dll", (LPCSTR)0x6D, explorer_SHCreateStreamOnModuleResourceWHook);
#endif

    printf("Setup explorer functions done\n");




    CreateWindowExWFunc = CreateWindowExW;
    rv = funchook_prepare(
        funchook,
        (void**)&CreateWindowExWFunc,
        CreateWindowExWHook
    );
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
    SetWindowLongPtrWFunc = SetWindowLongPtrW;
    rv = funchook_prepare(
        funchook,
        (void**)&SetWindowLongPtrWFunc,
        SetWindowLongPtrWHook
    );
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }




    HANDLE hUxtheme = LoadLibraryW(L"uxtheme.dll");
    SetPreferredAppMode = GetProcAddress(hUxtheme, (LPCSTR)0x87);
    AllowDarkModeForWindow = GetProcAddress(hUxtheme, (LPCSTR)0x85);
    ShouldAppsUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)0x84);
    GetThemeName = GetProcAddress(hUxtheme, (LPCSTR)0x4A);
    printf("Setup uxtheme functions done\n");


    HANDLE hTwinuiPcshell = LoadLibraryW(L"twinui.pcshell.dll");

    if (symbols_PTRS.twinui_pcshell_PTRS[0] && symbols_PTRS.twinui_pcshell_PTRS[0] != 0xFFFFFFFF)
    {
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc = (INT64(*)(HWND, int, HWND, int, BOOL*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[0]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[1] && symbols_PTRS.twinui_pcshell_PTRS[1] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc = (INT64(*)(void*, void*, void**))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[1]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[2] && symbols_PTRS.twinui_pcshell_PTRS[2] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc = (INT64(*)(HMENU, HMENU, HWND, unsigned int, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[2]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[3] && symbols_PTRS.twinui_pcshell_PTRS[3] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc = (void(*)(HMENU, HMENU, HWND))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[3]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[4] && symbols_PTRS.twinui_pcshell_PTRS[4] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteShutdownCommandFunc = (void(*)(void*, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[4]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[5] && symbols_PTRS.twinui_pcshell_PTRS[5] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteCommandFunc = (void(*)(void*, int))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[5]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[6] && symbols_PTRS.twinui_pcshell_PTRS[6] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[6]);
        rv = funchook_prepare(
            funchook,
            (void**)&CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc,
            CLauncherTipContextMenu_ShowLauncherTipContextMenuHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] && symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] != 0xFFFFFFFF)
    {
        winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1]);
        rv = funchook_prepare(
            funchook,
            (void**)&winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc,
            winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    VnPatchIAT(hTwinuiPcshell, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", twinuipcshell_RegGetValueW);
    printf("Setup twinui.pcshell functions done\n");


    HANDLE hStobject = LoadLibraryW(L"stobject.dll");
    VnPatchIAT(hStobject, "api-ms-win-core-registry-l1-1-0.dll", "RegGetValueW", stobject_RegGetValueW);
    VnPatchIAT(hStobject, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", stobject_CoCreateInstanceHook);
    VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenu", stobject_TrackPopupMenuHook);
    VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenuEx", stobject_TrackPopupMenuExHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchDelayIAT(hStobject, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup stobject functions done\n");



    HANDLE hBthprops = LoadLibraryW(L"bthprops.cpl");
    VnPatchIAT(hBthprops, "user32.dll", "TrackPopupMenuEx", bthprops_TrackPopupMenuExHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hBthprops, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup bthprops functions done\n");



    HANDLE hPnidui = LoadLibraryW(L"pnidui.dll");
    VnPatchIAT(hPnidui, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", pnidui_CoCreateInstanceHook);
    VnPatchIAT(hPnidui, "user32.dll", "TrackPopupMenu", pnidui_TrackPopupMenuHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hPnidui, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup pnidui functions done\n");




    HANDLE hSndvolsso = LoadLibraryW(L"sndvolsso.dll");
    VnPatchIAT(hSndvolsso, "user32.dll", "TrackPopupMenuEx", sndvolsso_TrackPopupMenuExHook);
    VnPatchIAT(hSndvolsso, "api-ms-win-core-registry-l1-1-0.dll", "RegGetValueW", sndvolsso_RegGetValueW);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hSndvolsso, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup sndvolsso functions done\n");




    rv = funchook_install(funchook, 0);
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
    printf("Installed hooks.\n");



    /*HANDLE hEvent = CreateEventEx(
        0,
        L"ShellDesktopSwitchEvent",
        CREATE_EVENT_MANUAL_RESET,
        EVENT_ALL_ACCESS
    );
    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Created ShellDesktopSwitchEvent event.\n");
        ResetEvent(hEvent);
    }*/

    if (bOldTaskbar)
    {
        CreateThread(
            0,
            0,
            PlayStartupSound,
            0,
            0,
            0
        );
        printf("Play startup sound thread...\n");
    }


    if (bOldTaskbar)
    {
        CreateThread(
            0,
            0,
            SignalShellReady,
            dwExplorerReadyDelay,
            0,
            0
        );
        printf("Signal shell ready...\n");
    }


    CreateThread(
        0,
        0,
        OpenStartOnCurentMonitorThread,
        0,
        0,
        0
    );
    printf("Open Start on monitor thread\n");


    if (bEnableArchivePlugin)
    {
        ArchiveMenuThreadParams* params = calloc(1, sizeof(ArchiveMenuThreadParams));
        params->CreateWindowInBand = CreateWindowInBand;
        params->hWnd = &archivehWnd;
        params->wndProc = CLauncherTipContextMenu_WndProc;
        CreateThread(
            0,
            0,
            ArchiveMenuThread,
            params,
            0,
            0,
            0
        );
    }



    CreateThread(NULL, 0, CheckForUpdatesThread, 0, 0, NULL);



    WCHAR wszExtraLibPath[MAX_PATH];
    if (GetWindowsDirectoryW(wszExtraLibPath, MAX_PATH))
    {
        wcscat_s(wszExtraLibPath, MAX_PATH, L"\\ep_extra.dll");
        if (FileExistsW(wszExtraLibPath))
        {
            HMODULE hExtra = LoadLibraryW(wszExtraLibPath);
            if (hExtra)
            {
                printf("[Extra] Found library: %p.\n", hExtra);
                FARPROC ep_extra_entrypoint = GetProcAddress(hExtra, "ep_extra_EntryPoint");
                if (ep_extra_entrypoint)
                {
                    printf("[Extra] Running entry point...\n");
                    ep_extra_entrypoint();
                    printf("[Extra] Finished running entry point.\n");
                }
            }
            else
            {
                printf("[Extra] LoadLibraryW failed with 0x%x.", GetLastError());
            }
        }
    }



    /*if (bHookStartMenu)
    {
        HookStartMenuParams* params2 = calloc(1, sizeof(HookStartMenuParams));
        params2->dwTimeout = 1000;
        params2->hModule = hModule;
        params2->proc = InjectStartFromExplorer;
        GetModuleFileNameW(hModule, params2->wszModulePath, MAX_PATH);
        CreateThread(0, 0, HookStartMenu, params2, 0, 0);
    }*/



    // This notifies applications when the taskbar has recomputed its layout
    /*if (SUCCEEDED(TaskbarCenter_Initialize(hExplorer)))
    {
        printf("Initialized taskbar update notification.\n");
    }
    else
    {
        printf("Failed to register taskbar update notification.\n");
    }*/




    //CreateThread(0, 0, PositionStartMenuTimeout, 0, 0, 0);

    /*else
    {
        if (bIsExplorer)
        {
            // deinject all

            rv = funchook_uninstall(funchook, 0);
            if (rv != 0)
            {
                FreeLibraryAndExitThread(hModule, rv);
                return rv;
            }

            rv = funchook_destroy(funchook);
            if (rv != 0)
            {
                FreeLibraryAndExitThread(hModule, rv);
                return rv;
            }
        }

        //SetEvent(hExitSettingsMonitor);
        //WaitForSingleObject(hSettingsMonitorThread, INFINITE);
        //CloseHandle(hExitSettingsMonitor);
        //free(settingsParams);
        //free(settings);
        //InjectBasicFunctions(FALSE, FALSE);
        FreeLibraryAndExitThread(hModule, 0);
    }*/
#endif
    return 0;
}

#ifdef _WIN64
char VisibilityChangedEventArguments_GetVisible(__int64 a1)
{
    int v1;
    char v3[8];
    ZeroMemory(v3, 8);

    v1 = (*(__int64(__fastcall**)(__int64, char*))(*(INT64*)a1 + 48))(a1, v3);
    if (v1 < 0)
        return 0;

    return v3[0];
}

DWORD StartMenu_maximumFreqApps = 6;
DWORD StartMenu_ShowAllApps = 0;

void StartMenu_LoadSettings(BOOL bRestartIfChanged)
{
    HKEY hKey = NULL;
    DWORD dwSize, dwVal;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MakeAllAppsDefault"),
            0,
            NULL,
            &StartMenu_ShowAllApps,
            &dwSize
        );
        RegCloseKey(hKey);
    }
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        dwVal = 6;
        RegQueryValueExW(
            hKey,
            TEXT("Start_MaximumFrequentApps"),
            0,
            NULL,
            &dwVal,
            &dwSize
        );
        if (bRestartIfChanged && dwVal != StartMenu_maximumFreqApps)
        {
            exit(0);
        }
        StartMenu_maximumFreqApps = dwVal;
        RegCloseKey(hKey);
    }
}

static INT64(*StartDocked_LauncherFrame_OnVisibilityChangedFunc)(void*, INT64, void*) = NULL;

static INT64(*StartDocked_LauncherFrame_ShowAllAppsFunc)(void* _this) = NULL;

INT64 StartDocked_LauncherFrame_OnVisibilityChangedHook(void* _this, INT64 a2, void* VisibilityChangedEventArguments)
{
    INT64 r = 0;
    if (StartDocked_LauncherFrame_OnVisibilityChangedFunc)
    {
        r = StartDocked_LauncherFrame_OnVisibilityChangedFunc(_this, a2, VisibilityChangedEventArguments);
    }
    if (StartMenu_ShowAllApps)
    {
        //if (VisibilityChangedEventArguments_GetVisible(VisibilityChangedEventArguments))
        {
            if (StartDocked_LauncherFrame_ShowAllAppsFunc)
            {
                StartDocked_LauncherFrame_ShowAllAppsFunc(_this);
            }
        }
    }
    return r;
}

INT64(*StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc)(void*) = NULL;

INT64 StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook(void* _this)
{
    return StartMenu_maximumFreqApps;
}

INT64(*StartDocked_StartSizingFrame_StartSizingFrameFunc)(void* _this) = NULL;

INT64 StartDocked_StartSizingFrame_StartSizingFrameHook(void* _this)
{
    INT64 rv = StartDocked_StartSizingFrame_StartSizingFrameFunc(_this);
    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        DWORD dwStatus = 0, dwSize = sizeof(DWORD);
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
        if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
            TEXT("TaskbarAl"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwStatus,
            (LPDWORD)(&dwSize)
        ) != ERROR_SUCCESS)
        {
            dwStatus = 0;
        }
        FreeLibrary(hModule);
        *(((char*)_this + 387)) = dwStatus;
    }
    return rv;
}

int WINAPI SetupMessage(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    return 0;
    LPCWSTR lpOldText = lpText;
    LPCWSTR lpOldCaption = lpCaption;
    wchar_t wszText[MAX_PATH];
    ZeroMemory(wszText, MAX_PATH * sizeof(wchar_t));
    wchar_t wszCaption[MAX_PATH];
    ZeroMemory(wszCaption, MAX_PATH * sizeof(wchar_t));
    LoadStringW(hModule, IDS_PRODUCTNAME, wszCaption, MAX_PATH);
    switch (Code)
    {
    case 1:
        LoadStringW(hModule, IDS_INSTALL_SUCCESS_TEXT, wszText, MAX_PATH);
        break;
    case -1:
        LoadStringW(hModule, IDS_INSTALL_ERROR_TEXT, wszText, MAX_PATH);
        break;
    case 2:
        LoadStringW(hModule, IDS_UNINSTALL_SUCCESS_TEXT, wszText, MAX_PATH);
        break;
    case -2:
        LoadStringW(hModule, IDS_UNINSTALL_ERROR_TEXT, wszText, MAX_PATH);
        break;
    default:
        LoadStringW(hModule, IDS_OPERATION_NONE, wszText, MAX_PATH);
        break;
    }
    int ret = MessageBoxW(hWnd, wszText, wszCaption, uType);
    lpText = lpOldText;
    lpOldCaption = lpOldCaption;
    return ret;
}

void Setup_Regsvr32(BOOL bInstall)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (!IsAppRunningAsAdminMode())
    {
        wchar_t wszPath[MAX_PATH];
        ZeroMemory(wszPath, ARRAYSIZE(wszPath));
        wchar_t wszCurrentDirectory[MAX_PATH];
        ZeroMemory(wszCurrentDirectory, ARRAYSIZE(wszCurrentDirectory));
        if (GetModuleFileNameW(NULL, wszPath, ARRAYSIZE(wszPath)) &&
            GetCurrentDirectoryW(ARRAYSIZE(wszCurrentDirectory), wszCurrentDirectory + (bInstall ? 1 : 4)))
        {
            wszCurrentDirectory[0] = L'"';
            if (!bInstall)
            {
                wszCurrentDirectory[0] = L'/';
                wszCurrentDirectory[1] = L'u';
                wszCurrentDirectory[2] = L' ';
                wszCurrentDirectory[3] = L'"';
            }
            wcscat_s(wszCurrentDirectory, ARRAYSIZE(wszCurrentDirectory), L"\\ExplorerPatcher.amd64.dll\"");
            SHELLEXECUTEINFOW sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = wszPath;
            sei.lpParameters = wszCurrentDirectory;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (!ShellExecuteExW(&sei))
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_CANCELLED)
                {
                    wchar_t wszText[MAX_PATH];
                    ZeroMemory(wszText, MAX_PATH * sizeof(wchar_t));
                    wchar_t wszCaption[MAX_PATH];
                    ZeroMemory(wszCaption, MAX_PATH * sizeof(wchar_t));
                    LoadStringW(hModule, IDS_PRODUCTNAME, wszCaption, MAX_PATH);
                    LoadStringW(hModule, IDS_INSTALL_ERROR_TEXT, wszText, MAX_PATH);
                    MessageBoxW(0, wszText, wszCaption, MB_ICONINFORMATION);
                }
            }
            exit(0);
        }
    }

    VnPatchDelayIAT(GetModuleHandle(NULL), "ext-ms-win-ntuser-dialogbox-l1-1-0.dll", "MessageBoxW", SetupMessage);
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllRegisterServer=_DllRegisterServer")
#endif
HRESULT WINAPI _DllRegisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];
    wchar_t wszInstallPath[MAX_PATH];

    Setup_Regsvr32(TRUE);

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                wszFilename,
                (wcslen(wszFilename) + 1) * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"ThreadingModel",
                0,
                REG_SZ,
                L"Apartment",
                10 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
    }
    if (!dwLastError)
    {
        PathRemoveExtensionW(wszFilename);
        PathRemoveExtensionW(wszFilename);
        wcscat_s(wszFilename, MAX_PATH, L".IA-32.dll");
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                wszFilename,
                (wcslen(wszFilename) + 1) * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"ThreadingModel",
                0,
                REG_SZ,
                L"Apartment",
                10 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            DWORD dwDriveMask = 255;
            dwLastError = RegSetValueExW(
                hKey,
                L"DriveMask",
                0,
                REG_DWORD,
                &dwDriveMask,
                sizeof(DWORD)
            );
            RegCloseKey(hKey);
        }
    }
    /*if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            DWORD dwNoInternetExplorer = 1;
            dwLastError = RegSetValueExW(
                hKey,
                L"NoInternetExplorer",
                0,
                REG_DWORD,
                &dwNoInternetExplorer,
                sizeof(DWORD)
            );
            RegCloseKey(hKey);
        }
    }*/
    Code = 1;
    if (dwLastError) Code = -Code;

    //ZZRestartExplorer(0, 0, 0, 0);

    return dwLastError == 0 ? S_OK : HRESULT_FROM_WIN32(dwLastError);
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllUnregisterServer=_DllUnregisterServer")
#endif
HRESULT WINAPI _DllUnregisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];

    Setup_Regsvr32(FALSE);

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    if (!dwLastError)
    {
        PathRemoveExtensionW(wszFilename);
        PathRemoveExtensionW(wszFilename);
        wcscat_s(wszFilename, MAX_PATH, L".IA-32.dll");
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    /*if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID)
                );
            }
        }
    }*/
    Code = 2;
    if (dwLastError) Code = -Code;

    //ZZRestartExplorer(0, 0, 0, 0);

    return dwLastError == 0 ? S_OK : HRESULT_FROM_WIN32(dwLastError);
}
#endif

#ifdef _WIN64
#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow")
#else
#pragma comment(linker, "/export:DllCanUnloadNow=__DllCanUnloadNow@0")
#endif
HRESULT WINAPI _DllCanUnloadNow()
{
    return S_FALSE;
}

void InjectStartMenu()
{
#ifdef _WIN64
    funchook = funchook_create();

    StartMenu_LoadSettings(FALSE);

    Setting* settings = calloc(3, sizeof(Setting));
    settings[0].callback = NULL;
    settings[0].data = NULL;
    settings[0].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    settings[0].hKey = NULL;
    ZeroMemory(settings[0].name, MAX_PATH);
    settings[0].origin = NULL;
    settings[1].callback = StartMenu_LoadSettings;
    settings[1].data = FALSE;
    settings[1].hEvent = NULL;
    settings[1].hKey = NULL;
    wcscpy_s(settings[1].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
    settings[1].origin = HKEY_CURRENT_USER;
    settings[2].callback = StartMenu_LoadSettings;
    settings[2].data = TRUE;
    settings[2].hEvent = NULL;
    settings[2].hKey = NULL;
    wcscpy_s(settings[2].name, MAX_PATH, TEXT(REGPATH));
    settings[2].origin = HKEY_CURRENT_USER;

    SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
    params->settings = settings;
    params->size = 3;
    CreateThread(
        0,
        0,
        MonitorSettings,
        params,
        0,
        0
    );

    int rv;

    DWORD dwVal0 = 0x62254, dwVal1 = 0x188EBC, dwVal2 = 0x187120, dwVal3 = 0x3C10, dwVal4 = 0x160AEC;

    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        DWORD dwStatus = 0, dwSize = sizeof(DWORD);
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");

        if (SHRegGetValueFromHKCUHKLMFunc)
        {

            dwSize = sizeof(DWORD);
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_0),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal0,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_1),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal1,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_2),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal2,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_3),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal3,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_4),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal4,
                (LPDWORD)(&dwSize)
            );

        }
        FreeLibrary(hModule);
    }

    LoadLibraryW(L"StartDocked.dll");
    HANDLE hStartDocked = GetModuleHandle(L"StartDocked.dll");
    if (dwVal1 && dwVal1 != 0xFFFFFFFF)
    {
        StartDocked_LauncherFrame_ShowAllAppsFunc = (INT64(*)(void*))
            ((uintptr_t)hStartDocked + dwVal1);
    }
    if (dwVal2 && dwVal2 != 0xFFFFFFFF)
    {
        StartDocked_LauncherFrame_OnVisibilityChangedFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal2);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_LauncherFrame_OnVisibilityChangedFunc,
            StartDocked_LauncherFrame_OnVisibilityChangedHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    if (dwVal3 && dwVal3 != 0xFFFFFFFF)
    {
        StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal3);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc,
            StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    if (dwVal4 && dwVal4 != 0xFFFFFFFF)
    {
        /*StartDocked_StartSizingFrame_StartSizingFrameFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal4);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_StartSizingFrame_StartSizingFrameFunc,
            StartDocked_StartSizingFrame_StartSizingFrameHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }*/
    }

    rv = funchook_install(funchook, 0);
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
#endif
}

void InjectShellExperienceHost()
{
#ifdef _WIN64
    HKEY hKey;
    if (RegOpenKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH), &hKey) != ERROR_SUCCESS)
    {
        return;
    }
    RegCloseKey(hKey);
    HMODULE hQA = LoadLibraryW(L"Windows.UI.QuickActions.dll");
    if (hQA)
    {
        PIMAGE_DOS_HEADER dosHeader = hQA;
        if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((u_char*)dosHeader + dosHeader->e_lfanew);
            if (ntHeader->Signature == IMAGE_NT_SIGNATURE)
            {
                char* pSEHPatchArea = NULL;
                char seh_pattern1[14] =
                {
                    // mov al, 1
                    0xB0, 0x01,
                    // jmp + 2
                    0xEB, 0x02,
                    // xor al, al
                    0x32, 0xC0,
                    // add rsp, 0x20
                    0x48, 0x83, 0xC4, 0x20,
                    // pop rdi
                    0x5F,
                    // pop rsi
                    0x5E,
                    // pop rbx
                    0x5B,
                    // ret
                    0xC3
                };
                char seh_off = 12;
                char seh_pattern2[5] =
                {
                    // mov r8b, 3
                    0x41, 0xB0, 0x03,
                    // mov dl, 1
                    0xB2, 0x01
                };
                BOOL bTwice = FALSE;
                PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
                for (unsigned int i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i)
                {
                    if (section->Characteristics & IMAGE_SCN_CNT_CODE)
                    {
                        if (section->SizeOfRawData && !bTwice)
                        {
                            DWORD dwOldProtect;
                            VirtualProtect(hQA + section->VirtualAddress, section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &dwOldProtect);
                            char* pCandidate = NULL;
                            while (TRUE)
                            {
                                pCandidate = memmem(
                                    !pCandidate ? hQA + section->VirtualAddress : pCandidate,
                                    !pCandidate ? section->SizeOfRawData : (uintptr_t)section->SizeOfRawData - (uintptr_t)(pCandidate - (hQA + section->VirtualAddress)),
                                    seh_pattern1,
                                    sizeof(seh_pattern1)
                                );
                                if (!pCandidate)
                                {
                                    break;
                                }
                                char* pCandidate2 = pCandidate - seh_off - sizeof(seh_pattern2);
                                if (pCandidate2 > section->VirtualAddress)
                                {
                                    if (memmem(pCandidate2, sizeof(seh_pattern2), seh_pattern2, sizeof(seh_pattern2)))
                                    {
                                        if (!pSEHPatchArea)
                                        {
                                            pSEHPatchArea = pCandidate;
                                        }
                                        else
                                        {
                                            bTwice = TRUE;
                                        }
                                    }
                                }
                                pCandidate += sizeof(seh_pattern1);
                            }
                            VirtualProtect(hQA + section->VirtualAddress, section->SizeOfRawData, dwOldProtect, &dwOldProtect);
                        }
                    }
                    section++;
                }
                if (pSEHPatchArea && !bTwice)
                {
                    DWORD dwOldProtect;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), PAGE_EXECUTE_READWRITE, &dwOldProtect);
                    pSEHPatchArea[2] = 0x90;
                    pSEHPatchArea[3] = 0x90;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), dwOldProtect, &dwOldProtect);
                }
            }
        }
    }
#endif
}

#define DLL_INJECTION_METHOD_DXGI 0
#define DLL_INJECTION_METHOD_COM 1
#define DLL_INJECTION_METHOD_START_INJECTION 2
HRESULT EntryPoint(DWORD dwMethod)
{
    if (bInstanced)
    {
        return E_NOINTERFACE;
    }

    TCHAR exePath[MAX_PATH], dllName[MAX_PATH];
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    PathStripPathW(dllName);
    BOOL bIsDllNameDXGI = !_wcsicmp(dllName, L"dxgi.dll");
    if (dwMethod == DLL_INJECTION_METHOD_DXGI && !bIsDllNameDXGI)
    {
        return E_NOINTERFACE;
    }

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION,
        FALSE,
        GetCurrentProcessId()
    );
    if (!hProcess)
    {
        return E_NOINTERFACE;
    }
    DWORD dwLength = MAX_PATH;
    QueryFullProcessImageNameW(
        hProcess,
        0,
        exePath,
        &dwLength
    );
    CloseHandle(hProcess);

    TCHAR wszExplorerExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszExplorerExpectedPath, MAX_PATH);
    wcscat_s(wszExplorerExpectedPath, MAX_PATH, L"\\explorer.exe");
    BOOL bIsThisExplorer = !_wcsicmp(exePath, wszExplorerExpectedPath);

    TCHAR wszStartExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszStartExpectedPath, MAX_PATH);
    wcscat_s(wszStartExpectedPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\StartMenuExperienceHost.exe");
    BOOL bIsThisStartMEH = !_wcsicmp(exePath, wszStartExpectedPath);

    TCHAR wszShellExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszShellExpectedPath, MAX_PATH);
    wcscat_s(wszShellExpectedPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe");
    BOOL bIsThisShellEH = !_wcsicmp(exePath, wszShellExpectedPath);

    if (dwMethod == DLL_INJECTION_METHOD_DXGI)
    {
        if (!(bIsThisExplorer || bIsThisStartMEH || bIsThisShellEH))
        {
            return E_NOINTERFACE;
        }
        TCHAR wszRealDXGIPath[MAX_PATH];
        GetSystemDirectoryW(wszRealDXGIPath, MAX_PATH);
        wcscat_s(wszRealDXGIPath, MAX_PATH, L"\\dxgi.dll");
#ifdef _WIN64
        SetupDXGIImportFunctions(LoadLibraryW(wszRealDXGIPath));
#endif
    }
    if (dwMethod == DLL_INJECTION_METHOD_COM && (bIsThisExplorer || bIsThisStartMEH || bIsThisShellEH))
    {
        return E_NOINTERFACE;
    }
    if (dwMethod == DLL_INJECTION_METHOD_START_INJECTION && !bIsThisStartMEH)
    {
        return E_NOINTERFACE;
    }

    bIsExplorerProcess = bIsThisExplorer;
    if (bIsThisExplorer)
    {
        Inject(!IsDesktopWindowAlreadyPresent());
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (bIsThisStartMEH)
    {
        InjectStartMenu();
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (bIsThisShellEH)
    {
        InjectShellExperienceHost();
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (dwMethod == DLL_INJECTION_METHOD_COM)
    {
        Inject(FALSE);
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }

    return E_NOINTERFACE;
}

#ifdef _WIN64
// for explorer.exe and ShellExperienceHost.exe
__declspec(dllexport) HRESULT DXGIDeclareAdapterRemovalSupport()
{
    EntryPoint(DLL_INJECTION_METHOD_DXGI);
    return DXGIDeclareAdapterRemovalSupportFunc();
}
// for StartMenuExperienceHost.exe via DXGI
__declspec(dllexport) HRESULT CreateDXGIFactory1(void* p1, void** p2)
{
    EntryPoint(DLL_INJECTION_METHOD_DXGI);
    return CreateDXGIFactory1Func(p1, p2);
}
// for StartMenuExperienceHost.exe via injection from explorer
HRESULT InjectStartFromExplorer()
{
    EntryPoint(DLL_INJECTION_METHOD_START_INJECTION);
    return HRESULT_FROM_WIN32(GetLastError());
}
#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject")
#else
#pragma comment(linker, "/export:DllGetClassObject=__DllGetClassObject@12")
#endif
// for everything else
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
)
{
    return EntryPoint(DLL_INJECTION_METHOD_COM);
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
