#include <LogVisor/LogVisor.hpp>
#include <boo/boo.hpp>
#include <Specter/Specter.hpp>

namespace RUDE
{

struct Application : boo::IApplicationCallback
{
    boo::IWindow* m_mainWindow;
    int appMain(boo::IApplication* app)
    {
        m_mainWindow = app->newWindow(_S("RUDE"));
        return 0;
    }
    void appQuitting(boo::IApplication*)
    {
    }
    void appFilesOpen(boo::IApplication*, const std::vector<boo::SystemString>&)
    {

    }
};

}

#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    LogVisor::RegisterConsoleLogger();
    RUDE::Application appCb;
    int ret = ApplicationRun(boo::IApplication::EPlatformType::Auto,
        appCb, _S("rude"), _S("RUDE"), argc, argv);
    printf("IM DYING!!\n");
    return ret;
}

#if _WIN32
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int)
{
    int argc = 0;
    const boo::SystemChar** argv = (const wchar_t**)(CommandLineToArgvW(lpCmdLine, &argc));
    static boo::SystemChar selfPath[1024];
    GetModuleFileNameW(nullptr, selfPath, 1024);
    static const boo::SystemChar* booArgv[32] = {};
    booArgv[0] = selfPath;
    for (int i=0 ; i<argc ; ++i)
        booArgv[i+1] = argv[i];

    LogVisor::CreateWin32Console();
    return wmain(argc+1, booArgv);
}
#endif
