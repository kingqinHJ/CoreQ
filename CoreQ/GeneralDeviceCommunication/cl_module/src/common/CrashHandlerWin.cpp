#include <windows.h>
#include <DbgHelp.h>
#include <shlwapi.h>
#include <string>
#include <tchar.h>
#include <codecvt>

#include <QFileInfo>

using namespace std;

#include <Psapi.h>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"psapi.lib")

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException);

namespace Internal {
class CrashHandler
{
public:
    CrashHandler() {
        char szFileName[MAX_PATH + 1] = { 0 };
        ::GetModuleFileNameA(NULL, szFileName, MAX_PATH);
        QFileInfo fi(szFileName);
        app_name = fi.baseName();

#ifdef QT_NO_DEBUG
        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
        QString msg = QString("CrashHandler intinalized: %1").arg(app_name);
        OutputDebugString(reinterpret_cast<const wchar_t *>(msg.utf16()));
#endif
    }

    QString app_name;
};
CrashHandler s_instance;
}

static BOOL EnsureFilePath(const TCHAR *szPath)
{
    int len = wcslen(szPath);
    int pos = 0;
    if (len <= 0) return FALSE;
    TCHAR szTemp[2048] = { 0 };
    for (; pos < len; pos++)
    {
        if (szPath[pos] == _T('\\') || szPath[pos] == _T('/'))
        {
            wcsncpy_s(szTemp, szPath, pos);
            szTemp[pos] = _T('\0');
            if (!PathFileExists(szTemp)) {
                if (!CreateDirectory(szTemp, NULL))
                    return FALSE;
            }
        }
    }
    return TRUE;
}

static void run(LPCTSTR cmd, LPCTSTR deffolder /*= NULL*/)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // CDuiString szcmd;
    // szcmd.Format(_T("\"%s\""), cmd);
    // CDuiString szDefFolder = GetFilePath(cmd);

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        (LPTSTR)cmd,		    // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        (LPCTSTR)deffolder, // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
        )
    {
        return ;
    }

    // Wait until child process exits.
    //   WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)  
{  
    // 创建Dump文件
    //
    HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Dump信息
    //
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;

    // 写入Dump文件内容
    //
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

    CloseHandle(hDumpFile);
}  

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(NULL, tszModule, MAX_PATH);

    std::wstring szApp = std::wstring(tszModule);

    PathRemoveFileSpec(tszModule);
    wstring szAppDir = std::wstring(tszModule);

    SYSTEMTIME stLocTime;
    GetLocalTime(&stLocTime);

    TCHAR dumpFile[MAX_PATH] = { 0 };
    swprintf_s(dumpFile, L"%s_%04d%02d%02d-%02d%02d%02d.dmp",
               Internal::s_instance.app_name.utf16(),
               stLocTime.wYear, stLocTime.wMonth, stLocTime.wDay,
               stLocTime.wHour, stLocTime.wMinute, stLocTime.wSecond);

    OutputDebugString(dumpFile);

    EnsureFilePath(dumpFile);
    CreateDumpFile(dumpFile, pException);

//    if (PathFileExists(dumpFile))
//    {
//        wstring szReporterExe = szAppDir + _T("\\CrashReport.exe");

//#if defined(BUILD_3U)
//        wstring szUrl = L"http://log.3u.com/log/info/uploadLogFile.go";
//#else
//        wstring szUrl = L"http://app3.i4.cn/log/info/uploadLogFile.go";
//#endif
//        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//        wstring szVersion = converter.from_bytes(VERSION_STR);

//        //最后一个参数 弃用
//        TCHAR cmd[512] = { 0 };
//        swprintf_s(cmd, _T("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" 0"), szReporterExe.c_str(),
//            dumpFile, szApp.c_str(), szUrl.c_str(), szVersion.c_str(), _T("11"));

//        OutputDebugString(cmd);

//        bool report = qgetenv("DisabledCrashReport") != "1";
//        if (report) {
//            run(cmd, szAppDir.c_str());
//            ExitProcess(1);
//        }
//    }

    return EXCEPTION_EXECUTE_HANDLER;
}
