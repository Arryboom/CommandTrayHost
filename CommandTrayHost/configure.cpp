#include "stdafx.h"
#include "configure.h"

//extern nlohmann::json* global_stat;
//extern HANDLE ghJob;

std::wstring get_utf16(const std::string& str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

std::wstring string_to_wstring(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}


// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

void code_change()
{
	std::string myString = "";
	std::wstring stemp = s2ws(myString);
	LPCWSTR result = stemp.c_str(); //bad code
}

bool initial_configure()
{
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	std::string config = isZHCN ? u8R"json({
    "configs": [
        {
            // ����8��һ��������
            "name":"cmd����", // ϵͳ���̲˵�����
            "path":"C:\\Windows\\System32", // cmd��exe����Ŀ¼
            "cmd":"cmd.exe", //cmd������뺬��.exe
            "working_directory":"", // �����еĹ���Ŀ¼��Ϊ��ʱ�Զ���path
            "addition_env_path":"",   //dll����Ŀ¼����ʱû�õ�
            "use_builtin_console":false,  //�Ƿ���CREATE_NEW_CONSOLE����ʱû�õ�
            "is_gui":false, // �Ƿ��� GUIͼ�ν������
            "enabled":true,  // �Ƿ�CommandTrayHost����ʱ���Զ���ʼ����
        },
        {
            "name":"cmd����2",
            "path":"C:\\Windows\\System32",
            "cmd":"cmd.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":false,
        },
    ],
    "global":true
})json" : u8R"json({
    "configs": [
        {
            "name":"cmd example", // Menu item name in systray
            "path":"C:\\Windows\\System32", // path which includes cmd exe
            "cmd":"cmd.exe",
            "working_directory":"", // working directory. empty is same as path
            "addition_env_path":"",   //dll search path
            "use_builtin_console":false,  //CREATE_NEW_CONSOLE
            "is_gui":false,
            "enabled":true,  // run when CommandTrayHost starts
        },
        {
            "name":"cmd example 2",
            "path":"C:\\Windows\\System32",
            "cmd":"cmd.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":false,
        },
    ],
    "global":true
})json";
	std::ofstream o("config.json");
	if (o.good()) { o << config << std::endl; return true; }
	else { return false; }
}

/*
 * return NULL: failed
 * return others: numbers of configs
 */
int configure_reader(std::string& out)
{
	if (TRUE != PathFileExists(L"config.json"))
	{
		if (!initial_configure())
		{
			return NULL;
		}
	}
	using namespace rapidjson;
	Document d;
	std::ifstream i("config.json");
	if (i.bad())
	{
		return NULL;
	}
	IStreamWrapper isw(i);
	LOGMESSAGE(L"configure_reader\n");
	if (d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(isw).HasParseError())
	{
		LOGMESSAGE(L"\nError(offset %u): %S\n",
			(unsigned)d.GetErrorOffset(),
			GetParseError_En(d.GetParseError()));
		// ...
		return NULL;
	}

	assert(d.IsObject());
	assert(!d.ObjectEmpty());
	if (!d.IsObject() || d.ObjectEmpty())
	{
		return NULL;
	}


	assert(d.HasMember("configs"));
	if (!d.HasMember("configs"))
	{
		return NULL;
	}

	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };

	assert(d["configs"].IsArray());
	if (!d["configs"].IsArray())
	{
		return NULL;
	}

	int cnt = 0;

	for (auto& m : d["configs"].GetArray())
	{
#ifdef _DEBUG
		StringBuffer sb;
		Writer<StringBuffer> writer(sb);
		m.Accept(writer);
		std::string ss = sb.GetString();
		LOGMESSAGE(L"Type of member %S is %S\n",
			ss.c_str(),
			kTypeNames[m.GetType()]);
		//LOGMESSAGE(L"Type of member %S is %S\n",
		//m.GetString(), kTypeNames[m.GetType()]);
#endif
		assert(m.IsObject());

		assert(m.HasMember("name"));
		assert(m["name"].IsString());

		assert(m.HasMember("path"));
		assert(m["path"].IsString());

		assert(m.HasMember("cmd"));
		assert(m["cmd"].IsString());

		assert(m.HasMember("working_directory"));
		assert(m["working_directory"].IsString());

		assert(m.HasMember("addition_env_path"));
		assert(m["addition_env_path"].IsString());

		assert(m.HasMember("use_builtin_console"));
		assert(m["use_builtin_console"].IsBool());

		assert(m.HasMember("is_gui"));
		assert(m["is_gui"].IsBool());

		assert(m.HasMember("enabled"));
		assert(m["enabled"].IsBool());

		if (m.IsObject() &&
			m.HasMember("name") && m["name"].IsString() &&
			m.HasMember("path") && m["path"].IsString() &&
			m.HasMember("cmd") && m["cmd"].IsString() &&
			m.HasMember("working_directory") && m["working_directory"].IsString() &&
			m.HasMember("addition_env_path") && m["addition_env_path"].IsString() &&
			m.HasMember("use_builtin_console") && m["use_builtin_console"].IsBool() &&
			m.HasMember("is_gui") && m["is_gui"].IsBool() &&
			m.HasMember("enabled") && m["enabled"].IsBool()
			)
		{
			if (m["working_directory"] == "")
			{
				m["working_directory"] = StringRef(m["path"].GetString());
			}
			cnt++;
		}
		else
		{
			return NULL;
		}
	}
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);
	d.Accept(writer);
	out = sb.GetString();
	//std::string ss = sb.GetString();
	//out = ss;
	return cnt;
}


/*
 * return NULL : failed
 * return 1 : sucess
 */
int init_global(nlohmann::json& js, HANDLE& ghJob)
{
	std::string js_string;
	int cmd_cnt = configure_reader(js_string);
	assert(cmd_cnt > 0);
	LOGMESSAGE(L"cmd_cnt:%d \n%S\n", cmd_cnt, js_string.c_str());
	if (cmd_cnt == 0)
	{
		::MessageBox(0, L"Load configure failed!", L"Error", MB_OK);
		return NULL;
	}
	//using json = nlohmann::json;
	//assert(js == nullptr);
	if (js == nullptr)
	{
		LOGMESSAGE(L"nlohmann::json& js not initialize\n");
	}

	// I don't know where is js now? data? bss? heap? stack?
	js = nlohmann::json::parse(js_string);
	for (auto& i : js["configs"])
	{
		i["running"] = false;
		i["handle"] = 0;
		i["pid"] = -1;
		i["show"] = false;
		std::wstring cmd = utf8_to_wstring(i["cmd"]), path = utf8_to_wstring(i["path"]);
		TCHAR commandLine[MAX_PATH * 128]; // �������Ҫ���ǿ�д���ַ�����������const�ġ�
		if (NULL != PathCombine(commandLine, path.c_str(), cmd.c_str()))
		{
			PTSTR pIdx = StrStr(commandLine, L".exe");
			if (pIdx)
			{
				*(pIdx + 4) = 0;
			}
			bool file_exist = PathFileExists(commandLine);
			if (!file_exist)
			{
				i["enabled"] = false;
				LOGMESSAGE(L"File not exist! %S %s\n", i["name"], commandLine);
			}
		}
	}

	if (ghJob != NULL)
	{
		return 1;
	}

	ghJob = CreateJobObject(NULL, NULL); // GLOBAL
	if (ghJob == NULL)
	{
		::MessageBox(0, L"Could not create job object", L"Error", MB_OK);
		return NULL;
	}
	else
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

		// Configure all child processes associated with the job to terminate when the
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		{
			::MessageBox(0, L"Could not SetInformationJobObject", L"Error", MB_OK);
			return NULL;
		}
	}
	return 1;
}

void start_all(nlohmann::json& js, HANDLE ghJob)
{
	int cmd_idx = 0;
	for (auto& i : js["configs"])
	{
		bool is_enabled = i["enabled"];
		if (is_enabled)
		{
			create_process(js, cmd_idx, ghJob);
		}
		cmd_idx++;
	}
}

std::vector<HMENU> get_command_submenu(nlohmann::json& js)
{
	LOGMESSAGE(L"get_command_submenu json %S\n", js.dump().c_str());
	//return {};

	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"��ʾ",
		L"����" ,
		L"����",
		L"����",
		L"��������"
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Show",
		L"Hide" ,
		L"Enable" ,
		L"Disable",
		L"Restart Command"
	};
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	std::vector<HMENU> vctHmenu;
	hSubMenu = CreatePopupMenu();
	vctHmenu.push_back(hSubMenu);
	int i = 0;
	for (auto& itm : js["configs"])
	{
		hSubMenu = CreatePopupMenu();

		bool is_enabled = static_cast<bool>(itm["enabled"]);
		bool is_running = static_cast<bool>(itm["running"]);
		bool is_show = static_cast<bool>(itm["show"]);

		int64_t handle = itm["handle"];
		if (is_running)
		{
			DWORD lpExitCode;
			BOOL retValue = GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &lpExitCode);
			if (retValue != 0 && lpExitCode != STILL_ACTIVE)
			{
				itm["running"] = false;
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["show"] = false;
				itm["enabled"] = false;

				is_running = false;
				is_show = false;
				is_enabled = false;
			}
		}

		UINT uSubFlags = is_running ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 0,
			utf8_to_wstring(itm["path"]).c_str());
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 1,
			utf8_to_wstring(itm["cmd"]).c_str());
		AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);

		const int info_items_cnt = 2;
		uSubFlags = is_enabled ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		for (int j = 0; j < 3; j++)
		{
			int menu_name_item;// = j + (j == 0 && is_running) + (j == 1 && is_show) + (j == 2 ? 0 : 2);
			if (j == 0)
			{
				if (is_show) { menu_name_item = 1; }
				else { menu_name_item = 0; }
			}
			else if (j == 1)
			{
				if (is_enabled) { menu_name_item = 3; }
				else { menu_name_item = 2; }
			}
			else
			{
				menu_name_item = 4;
			}
			LPCTSTR lpText = isZHCN ? MENUS_LEVEL2_CN[menu_name_item] : MENUS_LEVEL2_EN[menu_name_item];
			if (j != 1)
			{
				AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					lpText);
			}
			else
			{
				AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					lpText);
			}
		}
		UINT uFlags = is_enabled ? (MF_STRING | MF_CHECKED | MF_POPUP) : (MF_STRING | MF_POPUP);
		AppendMenu(vctHmenu[0], uFlags, (UINT_PTR)hSubMenu, utf8_to_wstring(itm["name"]).c_str());
		vctHmenu.push_back(hSubMenu);
		i++;
	}
	return vctHmenu;
}

#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define TA_SUCCESS_16 3

BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam)
{
	DWORD dwID;

	GetWindowThreadProcessId(hwnd, &dwID);

	if (dwID == (DWORD)lParam)
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}

	return TRUE;
}

/*----------------------------------------------------------------
DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )

Purpose:
Shut down a 32-Bit Process (or 16-bit process under Windows 95)

Parameters:
dwPID
Process ID of the process to shut down.

dwTimeout
Wait time in milliseconds before shutting down the process.

Return Value:
TA_FAILED - If the shutdown failed.
TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
TA_SUCCESS_KILL - if the process was shut down with
TerminateProcess().
NOTE:  See header for these defines.
----------------------------------------------------------------*/
DWORD WINAPI TerminateApp(DWORD dwPID, DWORD dwTimeout)
{
	HANDLE hProc;
	DWORD dwRet;

	// If we can't open the process with PROCESS_TERMINATE rights,
	// then we give up immediately.
	hProc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE,
		dwPID);

	if (hProc == NULL)
	{
		return TA_FAILED;
	}

	// TerminateAppEnum() posts WM_CLOSE to all windows whose PID
	// matches your process's.
	EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)dwPID);

	// Wait on the handle. If it signals, great. If it times out,
	// then you kill it.
	if (WaitForSingleObject(hProc, dwTimeout) != WAIT_OBJECT_0)
		dwRet = (TerminateProcess(hProc, 0) ? TA_SUCCESS_KILL : TA_FAILED);
	else
		dwRet = TA_SUCCESS_CLEAN;

	CloseHandle(hProc);

	return dwRet;
}

void check_and_kill(HANDLE hProcess, DWORD pid, PCSTR name)
{
	assert(GetProcessId(hProcess) == pid);
	if (GetProcessId(hProcess) == pid)
	{
		if (TA_FAILED == TerminateApp(pid, 200))
		{
			LOGMESSAGE(L"TerminateApp %S pid: %d failed!!  File = %S Line = %d Func=%S Date=%S Time=%S\n",
				name, pid,
				__FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
		else
		{
			LOGMESSAGE(L"TerminateApp %S pid: %d successed.  File = %S Line = %d Func=%S Date=%S Time=%S\n",
				name, pid,
				__FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
		}
	}
}


/* nlohmann::json& js		global internel state
 * int cmd_idx				index
 * const HANDLE& ghJob		global job object handle
 * LPCTSTR cmd				command to run
 * LPCTSTR path				command working directory
 * LPVOID env				Environment
 */
void create_process(
	nlohmann::json& js, // we may update js
	int cmd_idx,
	const HANDLE& ghJob
)
{
	//��������wstring��ס����Ȼ���������ʧ
	std::wstring cmd_wstring = utf8_to_wstring(js["configs"][cmd_idx]["cmd"]);
	std::wstring path_wstring = utf8_to_wstring(js["configs"][cmd_idx]["path"]);
	LPCTSTR cmd = cmd_wstring.c_str();
	LPCTSTR path = path_wstring.c_str();

	LPVOID env = NULL;

	LOGMESSAGE(L"%d %d\n", wcslen(cmd), wcslen(path));

	bool is_running = js["configs"][cmd_idx]["running"];
	if (is_running)
	{
		int64_t handle = js["configs"][cmd_idx]["handle"];
		int64_t pid = js["configs"][cmd_idx]["pid"];
		std::string name = js["configs"][cmd_idx]["name"];

		LOGMESSAGE(L"create_process process running, now kill it\n");

		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), name.c_str());
	}
	js["configs"][cmd_idx]["handle"] = 0;
	js["configs"][cmd_idx]["pid"] = -1;
	js["configs"][cmd_idx]["running"] = false;

	LOGMESSAGE(L"%d %d\n", wcslen(cmd), wcslen(path));

	std::wstring name = utf8_to_wstring(js["configs"][cmd_idx]["name"]);
	TCHAR nameStr[256];
	wcscpy_s(nameStr, name.c_str());

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.lpTitle = nameStr;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	TCHAR commandLine[MAX_PATH * 128]; // �������Ҫ���ǿ�д���ַ�����������const�ġ�
	if (NULL == PathCombine(commandLine, path, cmd))
		//if (0 != wcscpy_s(commandLine, MAX_PATH * 128, cmd))
	{
		//assert(false);
		LOGMESSAGE(L"Copy cmd failed\n");
		MessageBox(0, L"wcscpy_s Failed", L"Error", MB_OK);
	}

	LOGMESSAGE(L"cmd_idx:%d\n path: %s\n cmd: %s\n", cmd_idx, path, commandLine);

	// https://stackoverflow.com/questions/53208/how-do-i-automatically-destroy-child-processes-in-windows
	// Launch child process - example is notepad.exe
	if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, path, &si, &pi))
	{
		if (ghJob)
		{
			if (0 == AssignProcessToJobObject(ghJob, pi.hProcess))
			{
				MessageBox(0, L"Could not AssignProcessToObject", L"Error", MB_OK);
			}
			else
			{
				js["configs"][cmd_idx]["handle"] = reinterpret_cast<int64_t>(pi.hProcess);
				js["configs"][cmd_idx]["pid"] = static_cast<int64_t>(pi.dwProcessId);
				js["configs"][cmd_idx]["running"] = true;
			}
		}
		// Can we free handles now? Not sure about this.
		//CloseHandle(pi.hProcess); // Now I save all hProcess, so we don't need to close it now.
		CloseHandle(pi.hThread);
	}
	else
	{
		LOGMESSAGE(L"CreateProcess Failed. %d\n", GetLastError());
		MessageBox(0, L"CreateProcess Failed.", L"Msg", MB_ICONERROR);
	}
}

void disable_enable_menu(nlohmann::json& js, int cmd_idx, HANDLE ghJob)
{
	bool is_enabled = js["configs"][cmd_idx]["enabled"];
	if (is_enabled) {
		bool is_running = js["configs"][cmd_idx]["running"];
		if (is_running)
		{
			int64_t handle = js["configs"][cmd_idx]["handle"];
			int64_t pid = js["configs"][cmd_idx]["pid"];
			std::string name = js["configs"][cmd_idx]["name"];

			LOGMESSAGE(L"disable_enable_menu disable_menu process running, now kill it\n");

			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), name.c_str());
		}
		js["configs"][cmd_idx]["handle"] = 0;
		js["configs"][cmd_idx]["pid"] = -1;
		js["configs"][cmd_idx]["running"] = false;
		js["configs"][cmd_idx]["show"] = false;
		js["configs"][cmd_idx]["enabled"] = false;
	}
	else
	{
		js["configs"][cmd_idx]["enabled"] = true;
		create_process(js, cmd_idx, ghJob);
	}
}

// https://stackoverflow.com/questions/3269390/how-to-get-hwnd-of-window-opened-by-shellexecuteex-hprocess
struct ProcessWindowsInfo
{
	DWORD ProcessID;
	std::vector<HWND> Windows;

	ProcessWindowsInfo(DWORD const AProcessID)
		: ProcessID(AProcessID)
	{
	}
};

BOOL __stdcall EnumProcessWindowsProc(HWND hwnd, LPARAM lParam)
{
	ProcessWindowsInfo *Info = reinterpret_cast<ProcessWindowsInfo*>(lParam);
	DWORD WindowProcessID;

	GetWindowThreadProcessId(hwnd, &WindowProcessID);

	if (WindowProcessID == Info->ProcessID)
		Info->Windows.push_back(hwnd);

	return true;
}

void hide_all(nlohmann::json& js)
{
	for (auto& itm : js["configs"])
	{
		bool is_show = itm["show"];
		int64_t handle_int64 = itm["handle"];
		HANDLE hProcess = (HANDLE)handle_int64;
		WaitForInputIdle(hProcess, INFINITE);

		ProcessWindowsInfo Info(GetProcessId(hProcess));

		EnumWindows((WNDENUMPROC)EnumProcessWindowsProc,
			reinterpret_cast<LPARAM>(&Info));

		size_t num_of_windows = Info.Windows.size();
		LOGMESSAGE(L"show_terminal size: %d\n", num_of_windows);
		if (num_of_windows > 0)
		{
			if (is_show)
			{
				ShowWindow(Info.Windows[0], SW_HIDE);
				itm["show"] = false;
			}
		}
	}

}

void show_hide_toggle(nlohmann::json& js, int cmd_idx)
{
	bool is_show = js["configs"][cmd_idx]["show"];
	int64_t handle_int64 = js["configs"][cmd_idx]["handle"];
	HANDLE hProcess = (HANDLE)handle_int64;
	WaitForInputIdle(hProcess, INFINITE);

	ProcessWindowsInfo Info(GetProcessId(hProcess));

	EnumWindows((WNDENUMPROC)EnumProcessWindowsProc,
		reinterpret_cast<LPARAM>(&Info));

	size_t num_of_windows = Info.Windows.size();
	LOGMESSAGE(L"show_terminal size: %d\n", num_of_windows);
	if (num_of_windows > 0)
	{
		if (is_show)
		{
			ShowWindow(Info.Windows[0], SW_HIDE);
			js["configs"][cmd_idx]["show"] = false;
		}
		else
		{
			ShowWindow(Info.Windows[0], SW_SHOW);
			SetForegroundWindow(Info.Windows[0]);
			js["configs"][cmd_idx]["show"] = true;
		}
	}

}

// https://stackoverflow.com/questions/15913202/add-application-to-startup-registry
BOOL IsMyProgramRegisteredForStartup(PCWSTR pszAppName)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwRegType = REG_SZ;
	TCHAR szPathToExe[MAX_PATH] = {};
	DWORD dwSize = sizeof(szPathToExe);

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		lResult = RegGetValue(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe, &dwSize);
		fSuccess = (lResult == 0);
	}

	if (fSuccess)
	{
		fSuccess = (wcslen(szPathToExe) > 0) ? TRUE : FALSE;
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL RegisterMyProgramForStartup(PCWSTR pszAppName, PCWSTR pathToExe, PCWSTR args)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwSize;

	const size_t count = MAX_PATH * 2;
	TCHAR szValue[count] = {};


	wcscpy_s(szValue, count, L"\"");
	wcscat_s(szValue, count, pathToExe);
	wcscat_s(szValue, count, L"\" ");

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		wcscat_s(szValue, count, args);
	}

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		dwSize = static_cast<DWORD>((wcslen(szValue) + 1) * 2);
		lResult = RegSetValueEx(hKey, pszAppName, 0, REG_SZ, reinterpret_cast<BYTE*>(szValue), dwSize);
		fSuccess = (lResult == 0);
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL DisableStartUp()
{
	if (ERROR_SUCCESS == RegDeleteKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", CommandTrayHost))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL EnableStartup()
{
	TCHAR szPathToExe[MAX_PATH];
	GetModuleFileName(NULL, szPathToExe, MAX_PATH);
	return RegisterMyProgramForStartup(CommandTrayHost, szPathToExe, L"");
}
