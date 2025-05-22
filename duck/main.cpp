#include "pch.h"

#include "duckDemo.h"
#include "exceptions.h"

using namespace std;
using namespace mini;
using namespace gk2;
void CreateConsole()
{
    AllocConsole();

    // Redirect standard streams to the console
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::ios::sync_with_stdio();

    // Optional: set console title
    SetConsoleTitleA("Debug Console");
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
    CreateConsole();
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    auto exitCode = EXIT_FAILURE;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    try
    {
        DuckDemo app(hInstance);
        exitCode = app.Run();
    }
    catch (Exception& e)
    {
        MessageBoxW(nullptr, e.getMessage().c_str(), L"Błąd", MB_OK);
        exitCode = e.getExitCode();
    }
    catch (exception& e)
    {
        string s(e.what());
        MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK);
    }
    catch (const char* str)
    {
        string s(str);
        MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK);
    }
    catch (const wchar_t* str)
    {
        MessageBoxW(nullptr, str, L"Błąd", MB_OK);
    }
    catch (...)
    {
        MessageBoxW(nullptr, L"Nieznany Błąd", L"Błąd", MB_OK);
    }
    return exitCode;
}