#include "App.h"
#include "ChecksumData_xbox.h"
#include "char/Char.h"
#include "flow/Flow.h"
#include "game/Game.h"
#include "game/HamUserMgr.h"
#include "game/PartyModeMgr.h"
#include "game/PresenceMgr.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/Ham.h"
#include "hamobj/HamNavList.h"
#include "hamobj/MiniGameMgr.h"
#include "hamobj/MoveMgr.h"
#include "meta/FixedSizeSaveable.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/ContextChecker.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/SaveLoadManager.h"
#include "midi/MidiParser.h"
#include "movie/Movie.h"
#include "movie/Splash.h"
#include "net/DingoSvr.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/FileCache.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/Rnd.h"
#include "stl/_algobase.h"
#include "synth/Synth.h"
#include "synth/SynthSample.h"
#include "ui/UI.h"
#include "utl/Cheats.h"
#include "utl/Loader.h"
#include "utl/Magnu.h"
#include "utl/MemTracker.h"
#include "utl/Option.h"
#include "world/World.h"
#include "xdk/nui/nuiskeleton.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbox.h"
#include <cctype>
#include <cstring>

App *gApp;
ModalCallbackFunc *gRealCallback;
namespace {
    FileCache *gPersistentCache;
    bool gListenForKinectGuide;
}

bool EndsWith(const char *c1, const char *c2) {
    int len1 = strlen(c1);
    int len2 = strlen(c2);
    return strstr(c1, c2) == c1 + len1 - len2;
}

void DebugModal(Debug::ModalType &ty, FixedString &str, bool b3) {
    if (ty == Debug::kModalFail) {
        gRealCallback(ty, str, b3);
    } else {
        if (ty != Debug::kModalWarn) {
            static DataNode &n = DataVariable("notify_level");
            switch (n.Int()) {
            case 2: {
                gRealCallback(ty, str, b3);
                return;
            }
            case 1: {
                Hmx::Object *cheatDisplay =
                    ObjectDir::Main()->Find<Hmx::Object>("cheat_display", false);
                if (cheatDisplay) {
                    static Message show("show", 0);
                    show[0] = str.c_str();
                    cheatDisplay->Handle(show, false);
                }
                return;
            }
            }
        }
        MILO_LOG("%s\n", str.c_str());
    }
}

Symbol RemoveDigitSuffix(const Symbol &s1) {
    char buffer[0x40];
    buffer[0] = '\0';
    memset(&buffer[1], 0, 0x40 - 1);
    const char *strBegin = s1.Str();
    int len = strlen(strBegin);
    MILO_ASSERT(len > 0, 0x2AB);
    const char *res = std::find_if(strBegin, strBegin + len, isdigit);
    if (res - strBegin != 0) {
        memmove(buffer, strBegin, res - strBegin);
    }
    return buffer;
}

bool IsUselessLoad(const char *);

bool XShowNuiCallback(DWORD &id) {
    bool ret;

    MILO_ASSERT(TheGestureMgr, 0x87);

    Skeleton *skel = TheGestureMgr->GetActiveSkeleton();

    if (!HamNavList::sLastSelectInControllerMode && skel && skel->IsTracked()) {
        ret = true;
        id = skel->TrackingID();
    } else {
        ret = false;
    }

    return ret;
}

DWORD KinectGuideThread(LPVOID) {
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingDisable()), "NuiSkeletonTrackingDisable failed"
    );
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(nullptr, 0)),
        "NuiSkeletonTrackingEnable failed"
    );
    HANDLE kinect_listener = XNotifyCreateListener(1);
    MILO_ASSERT(kinect_listener, 0xA2);
    while (gListenForKinectGuide) {
        DWORD dwId;
        ULONG param;
        while (XNotifyGetNext(kinect_listener, 0, &dwId, &param)) {
            if (dwId == 0x6001A) {
                XShowNuiGuideUI(param);
            }
        }
    }
    CloseHandle(kinect_listener);
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingDisable()), "NuiSkeletonTrackingDisable failed"
    );
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(SkeletonUpdate::NewSkeletonEvent(), 2)),
        "NuiSkeletonTrackingEnable failed"
    );
    return 0;
}

static const int arkNums[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

App::App(int argc, char **argv) {
    Timer init_time;
    init_time.Start();
    EnableKeyCheats(false);
    SetFileChecksumData();
    SystemPreInit(argc, argv, "config/ham_preinit_keep.dta");
    if (TheArchive) {
        TheArchive->SetArchivePermission(0xB, arkNums);
    }
    TheRnd.PreInit();
    static DataNode &notify_level = DataVariable("notify_level");
    if (UsingCD()) {
        notify_level = 1;
    } else {
        notify_level = 1;
    }
    gRealCallback = TheDebug.SetModalCallback(DebugModal);
    SynthPreInit();
    Movie::Init();
    TheRnd.SetClearColor(Hmx::Color(0, 0, 0));
    Splash splash;
    bool fast = OptionBool("fast", false);
    if (fast || !UsingCD()) {
        splash.SetWaitForSplash(false);
    }
    if (fast) {
        SynthSample::Disable();
    }
    PlatformRegion region = ThePlatformMgr.GetRegion();
    if (ULSystemLocale() == XC_LOCALE_JAPAN) {
        splash.AddScreen("ui/splash/jpn/esrb_keep.milo", 4800);
    } else if (region == kRegionNA) {
        splash.AddScreen("ui/splash/eng/esrb_keep.milo", 4800);
    }
    splash.AddScreen("ui/splash/harmonix_keep.milo", 3000);
    splash.PrepareNext();
    splash.BeginSplasher();
    float ms = init_time.SplitMs();
    LiveCameraInput::PreInit();
    LiveCameraInput::Init();
    gListenForKinectGuide = true;
    HANDLE hThread = CreateThread(nullptr, 0, KinectGuideThread, nullptr, 4, nullptr);
    XSetThreadProcessor(hThread, 1);
    ResumeThread(hThread);
    splash.PrepareRemaining();
    SystemInit("config/ham_keep.dta");
    MagnuInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    splash.Suspend();
    TheRnd.Init();
    TheServer.Init();
    TheRockCentral.Init();
    splash.Resume();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MILO_LOG("HMX Red Build!\n");
    FixedSizeSaveable::Init(0x5c, 0x1662);
    HamUserMgrInit(false);
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    SynthInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    FlowInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    {
        ObjDirPtr<ObjectDir> dPtr;
        dPtr.LoadFile("sfx/audio_mixer.milo", false, true, kLoadFront, false);
        ObjDirPtr<ObjectDir> dPtr2;
        dPtr2.LoadFile(
            SystemConfig("sound", "banks", "common")->Str(1),
            false,
            true,
            kLoadFront,
            false
        );
        TheSynth->SetDir(dPtr2);
        if (TheSplasher) {
            TheSplasher->Poll();
        }
    }
    SaveLoadManager::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    CharInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MidiParser::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    WorldInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    HamInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    TheHamSongMgr.Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MetaPanel::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    GameInit();
    DirLoader::SetPathEvalFunc(IsUselessLoad);
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    ContextCheckerInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    PlatformMgr::sXShowCallback = XShowNuiCallback;
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    AccomplishmentManager::Init(SystemConfig("accomplishment_info"));
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MetagameRank::Init();
    DataArray *cacheCfg = SystemConfig("persistent_filecache");
    if (cacheCfg) {
        gPersistentCache = new FileCache(cacheCfg->Int(1), kLoadFront, false, true);
        gPersistentCache->StartSet(0);
        for (int i = 2; i < cacheCfg->Size(); i++) {
            gPersistentCache->Add(cacheCfg->Str(i), 1, "");
        }
        gPersistentCache->EndSet();
        gPersistentCache->PollUntilLoaded();
    }
    static DataNode &extra_songs = DataVariable("extra_songs");
    if (UsingCD()) {
        extra_songs = 0;
    } else {
        extra_songs = 1;
    }
    TheUI->Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    GestureMgr::DebugInit();
    ThePresenceMgr.Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MoveMgr::Init(nullptr);
    MiniGameMgr::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    PartyModeMgr::Init();
    TheUI->GotoFirstScreen();
    float f15 = init_time.SplitMs();
    if (TheArchive && TheArchive->DebugArkOrder()) {
        MILO_LOG("Startup Time: %f %f\n", ms, f15 - ms);
    }
    splash.EndSplasher();
    EnableKeyCheats(true);
    AutoGlitchReport::EnableCallback();
    ThePlatformMgr.SetBackgroundDownloadPriority(true);
    gListenForKinectGuide = false;
    WaitForSingleObject(hThread, -1);
    CloseHandle(hThread);
    MemTrackEnable(true);
}

void App::Run() { RunWithoutDebugging(); }

void App::CaptureHiRes() {
    bool paused = AllPaused();

    if (paused)
        TheGame->SetTimePaused(true);

    DrawRegular();

    int tiles = TheHiResScreen.GetTiling() * TheHiResScreen.GetTiling();

    for (int i = 0; i <= tiles; i++) {
        DrawRegular();
        TheHiResScreen.Accumulate();
    }

    TheHiResScreen.Finish();

    if (paused)
        TheGame->SetTimePaused(false);
}

void App::DrawRegular() {
    TheRnd.BeginDrawing();
    TheUI->Draw();
    TheRnd.EndDrawing();
}

App::~App() { TheDebug.Exit(0, true); }
