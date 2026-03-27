#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/NetworkSocket_Win.h"
#include "os/PlatformMgr.h"
#include "utl/JobMgr.h"
#include "utl/Locale.h"
#include "xdk/XAPILIB.h"
#include "xdk/XMP.h"
#include "xdk/XNET.h"
#include "xdk/XONLINE.h"
#include "xdk/NUI.h"
#include "xdk/XBC.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"
#include "xdk/xbc/xbc.h"
#include "xdk/xjson/xjson.h"
#include "xdk/xmp/xmp.h"

namespace {
    enum ServiceIdState {
    };

    class FriendEnumRequest {
    public:
        int unk0;
    };

    DWORD gSmartGlassClientIDs[4];
    XUID mXuidCache[4];
    unsigned long mResult;
    unsigned long mUserID;
    unsigned long mPathLen;
    unsigned long mListSize;
    unsigned long mFileReadBuffer[1024];
    wchar_t mStrServerPath[512];
    XSTORAGE_ENUMERATE_RESULTS *mStorageList;
    XOVERLAPPED *mServiceIDOverlapped2;
    XOVERLAPPED *mServiceIDOverlapped;
    ServiceIdState mServiceIdState;
    float mRetryTime;
    std::vector<Friend *> *mFriendsList;
    WCHAR (*mStrStorageFiles)[256];
    Hmx::Object *mFriendsCallback;
    void *mFriendsAsync;
    void *mFriendsBuffer;
    void *mFriendsEnum;
    HANDLE mListener;
    int mSigninSameGuest;
    // ...
    // struct _XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS `anonymous namespace'::mResults
    int gNumSmartGlassClients;
    int gNumSmartGlassSendsInProgress;
    std::list<FriendEnumRequest *> mFriendEnumRequests;
    Timer mTime;
    std::map<String, unsigned int> mServiceIdMap;

    int GetPadNumFromXuid(XUID xuid) {
        for (int i = 0; i < 4; i++) {
            XUSER_SIGNIN_INFO info;
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            info.xuid = xuid;
            XUserGetSigninInfo(i, 1, &info);
            if (info.xuid == 0) {
                return i;
            }
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            info.xuid = xuid;
            XUserGetSigninInfo(i, 2, &info);
            if (info.xuid == 0) {
                return i;
            }
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            XUserGetXUID(i, &info.xuid);
            if (info.xuid == 0) {
                return i;
            }
        }
        return -1;
    }

    bool XPrivilegeCheck(XPRIVILEGE_TYPE t1, XPRIVILEGE_TYPE t2, XUID xuid) {
        BOOL result = false;
        XUserCheckPrivilege(0xFF, t1, &result);
        if (!result) {
            XUserCheckPrivilege(0xFF, t2, &result);
            if (!result) {
                return false;
            }
            for (int i = 0; i < 4; i++) {
                if (XUserCheckPrivilege(i, t1, &result) == 0 && !result
                    && XUserAreUsersFriends(i, &xuid, 1, &result, nullptr) == 0
                    && !result) {
                    return false;
                }
            }
        }
        return true;
    }

    void DtaToJsonHelper(HJSONWRITER *, const DataArray *);
    DataArrayPtr JsonToDta(HJSONREADER *, bool);
    HJSONWRITER *DtaToJson(const DataArray *);

    void XbcSendMsg(DWORD id, const DataArray *a) {
        HJSONWRITER *writer = DtaToJson(a);
        if (id == 0) {
            // i hate this
            for (DWORD *it = gSmartGlassClientIDs; it < &gSmartGlassClientIDs[4]; it++) {
                if (*it) {
                    XbcSendJSON(XBC_DELIVERY_RELIABLE, *it, writer, nullptr);
                    gNumSmartGlassSendsInProgress++;
                }
            }
        } else {
            XbcSendJSON(XBC_DELIVERY_RELIABLE, id, writer, nullptr);
            gNumSmartGlassSendsInProgress++;
        }
        XJSONCloseWriter(writer);
    }

    void XbcRecieveMsg(DWORD id, HJSONREADER *reader) {
        DataArrayPtr dta = JsonToDta(reader, true);
        SmartGlassMsg msg(id, dta);
        ThePlatformMgr.Handle(msg, true);
    }

    void XbcCallback(long err, XBC_EVENT_PARAMS *params, void *) {
        if (err != 0) {
            MILO_NOTIFY("SmartGlass: Error in cb: 0x%08x", err);
        } else if (params->nUserIndex >= 4) {
            MILO_NOTIFY(
                "SmartGlass: Error in cb: user index %d (event: %d)",
                params->nUserIndex,
                params->Type
            );
        } else {
            switch (params->Type) {
            case XBC_EVENT_CLIENT_CONNECTED: {
                gSmartGlassClientIDs[params->nUserIndex] = params->nClientId;
                gNumSmartGlassClients++;
                MILO_ASSERT(gNumSmartGlassClients <= XBC_MAX_CLIENTS, 0x20C);
                break;
            }
            case XBC_EVENT_CLIENT_DISCONNECTED: {
                gSmartGlassClientIDs[params->nUserIndex] = 0;
                gNumSmartGlassClients--;
                MILO_ASSERT(gNumSmartGlassClients >= 0, 0x214);
                break;
            }
            case XBC_EVENT_JSON_SEND_COMPLETE: {
                gNumSmartGlassSendsInProgress--;
                break;
            }
            case XBC_EVENT_JSON_RECEIVE_COMPLETE: {
                XbcRecieveMsg(params->nClientId, params->hReader);
                break;
            }
            default:
                break;
            }
        }
    }

    void SmartGlassInit() {
        for (int i = 0; i < 4; i++) {
            gSmartGlassClientIDs[i] = 0;
        }
        if (XbcInitialize(XbcCallback, nullptr) < 0) {
            MILO_FAIL("Failed to initialize Xbox SmartGlass library.\n");
        }
    }

    void SmartGlassPoll() {
        HRESULT res = XbcDoWork();
        if (res != 0) {
            MILO_NOTIFY("SmartGlass: error: %d\n", res);
        }
    }

}

Hmx::Object *PlatformMgr::spShowControllerObject = nullptr;
unsigned long PlatformMgr::sdwShowControllerTrackingID = 0;
int PlatformMgr::snShowControllerPadNum = 0;

PlatformMgr::PlatformMgr() : mSigninMask(0) {
    mScreenSaver = true;
    mSigninChangeMask = 0;
    mGuideShowing = false;
    mConfirmCancelSwapped = false;
    mConnected = false;
    mRegion = kRegionNone;
    mDiskError = kNoDiskError;
    unk69 = false;
    mSigninSameGuest = 0;
    mFriendsEnum = nullptr;
    mFriendsBuffer = nullptr;
    mFriendsCallback = nullptr;
    mFriendsAsync = nullptr;
    mFriendsList = nullptr;
    mListener = nullptr;
    mJobMgr = new JobMgr(this);
    for (int i = 0; i < 4; i++) {
        mXuidCache[i] = 0;
    }
    mServiceIDOverlapped = nullptr;
    mServiceIDOverlapped2 = nullptr;
    mStorageList = nullptr;
    mPathLen = 0x200;
    mServiceIdState = (ServiceIdState)0;
    mListSize = 0;
    mUserID = -1;
    mResult = 0;
    mOverlapped.hEvent = nullptr;
}

PlatformMgr::~PlatformMgr() {
    DWORD ret = CloseHandle(mListener);
    MILO_ASSERT(ret == ERROR_SUCCESS, 999);
    ret = XOnlineCleanup();
    MILO_ASSERT(ret == ERROR_SUCCESS, 0x3EA);
}

void PlatformMgr::PreInit() { XMPOverrideBackgroundMusic(); }

void PlatformMgr::Init() {
    SetName("platform_mgr", ObjectDir::Main());
    WinSockSocket::Init();
    DWORD ret = XOnlineStartup();
    MILO_ASSERT(ret == ERROR_SUCCESS, 0x419);
    mListener = XNotifyCreateListener(0xA7);
    MILO_ASSERT(mListener, 0x41C);
    UpdateSigninState();
    SmartGlassInit();
    mTime.Start();
    mRetryTime = mTime.Ms() + 300000.0f;
}

bool PlatformMgr::IsEthernetCableConnected() { return XNetGetEthernetLinkStatus() != 0; }

void PlatformMgr::UpdateSigninState() {
    XUID oldCache[4] = { mXuidCache[0], mXuidCache[1], mXuidCache[2], mXuidCache[3] };
    mSigninSameGuest = 0;
    mSigninChangeMask = 0;
    mSigninMask = 0;
    for (int i = 0; i < 4; i++) {
        if (XUserGetSigninState(i) == eXUserSigninState_NotSignedIn) {
            mXuidCache[i] = 0;
        } else {
            XUSER_SIGNIN_INFO info;
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            mSigninMask |= 1 << i;
            XUserGetSigninInfo(i, 2, &info);
            XUserGetXUID(i, &info.xuid);
            mXuidCache[i] = 0;
        }
        if (oldCache[i] != mXuidCache[i]) {
            mSigninChangeMask |= 1 << i;
            if (((mXuidCache[i] ^ oldCache[i]) & 0xff3fffffffffffff) == 0) {
                mSigninSameGuest |= 1 << i;
            }
        }
    }
}

bool PlatformMgr::HasCreatedContentPrivilege() const {
    bool ret = true;
    for (int i = 0; i < 4; i++) {
        BOOL result = false;
        bool created =
            XUserCheckPrivilege(i, XPRIVILEGE_USER_CREATED_CONTENT, &result) == 0
            && result == 0;
        bool createdFriends =
            XUserCheckPrivilege(i, XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY, &result)
                == 0
            && result == 0;
        bool bAnd = !created || !createdFriends;
        ret &= bAnd;
    }
    return ret;
}

bool PlatformMgr::HasKinectSharePrvilege() const {
    int bptr = 0;
    if (XUserCheckPrivilege(0xFF, XPRIVILEGE_SHARE_CONTENT_OUTSIDE_LIVE, &bptr) == 0) {
        return true;
    } else if (bptr == 0) {
        bool ret = bptr;
        return ret;
    }
}

bool PlatformMgr::IsSmartGlassConnected() { return gNumSmartGlassClients > 0; }

void PlatformMgr::SetPadContext(int padNum, int id, int val) const {
    if (padNum != -1 && ThePlatformMgr.IsSignedIn(padNum)) {
        XUserSetContext(padNum, id, val);
    }
}

void PlatformMgr::SetPadPresence(int padNum, int val) const {
    if (padNum != -1 && ThePlatformMgr.IsSignedIn(padNum)) {
        XUserSetContext(padNum, 0x8001, val);
    }
}

void PlatformMgr::ShowFriendsUI(int padNum) {
    DWORD trackingID;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            XShowNuiFriendsUI(trackingID, padNum);
        } else {
            XShowFriendsUI(padNum);
        }
    }
}

void PlatformMgr::SetBackgroundDownloadPriority(bool alwaysAllow) {
    XBackgroundDownloadSetMode(
        alwaysAllow ? XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW
                    : XBACKGROUND_DOWNLOAD_MODE_AUTO
    );
}

void PlatformMgr::SignInUsers(int count, DWORD flags) {
    MILO_ASSERT(count == 1 || count == 2 || count == 4, 0x64E);
    DWORD trackingID;
    if (sXShowCallback(trackingID)) {
        XShowNuiSigninUI(trackingID, flags);
    } else {
        XShowSigninUI(count, flags);
    }
}

bool PlatformMgr::PollXSocialCapabilities() {
    if (mOverlapped.hEvent && mOverlapped.InternalLow != ERROR_IO_PENDING) {
        CloseHandle(mOverlapped.hEvent);
        mOverlapped.hEvent = nullptr;
        BOOL result = false;
        mHasXSocialPhotoPost = (unsigned char)unk4c & 1;
        mHasXSocialLinkPost = ((unsigned char)unk4c >> 1) & 1;
        if (XUserCheckPrivilege(0xFF, XPRIVILEGE_SOCIAL_NETWORK_SHARING, &result)
            || !result) {
            mHasXSocialPhotoPost = false;
            mHasXSocialLinkPost = false;
        }
        MILO_LOG(
            "PollXSocialCapabilities() - can post Photo:%s can post Link:%s\n",
            mHasXSocialPhotoPost ? "YES" : "NO",
            mHasXSocialLinkPost ? "YES" : "NO"
        );
        return true;
    } else {
        return false;
    }
}

int ShowControllerRequiredUIThreaded() {
    return XShowNuiControllerRequiredUI(
        PlatformMgr::sdwShowControllerTrackingID, PlatformMgr::snShowControllerPadNum
    );
}

bool PlatformMgr::ShowPartyUI(int padNum) {
    DWORD trackingID;
    DWORD ret = 1;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiPartyUI(trackingID, padNum);
        } else {
            ret = XShowPartyUI(padNum);
        }
    }

    return ret == 0;
}

bool PlatformMgr::ShowFitnessBodyProfileUI(int padNum) {
    DWORD trackingID;
    DWORD ret = 1;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiFitnessBodyProfileUI(trackingID, padNum);
        } else {
            ret = XShowFitnessBodyProfileUI(padNum);
        }
    }

    return ret == 0;
}

void PlatformMgr::EnableXMP() { XMPRestoreBackgroundMusic(); }
void PlatformMgr::DisableXMP() { XMPOverrideBackgroundMusic(); }

void PlatformMgr::SetScreenSaver(bool screensaver) {
    mScreenSaver = screensaver;
    XEnableScreenSaver(screensaver);
}

bool PlatformMgr::IsSignedIntoLive(int padNum) const {
    MILO_ASSERT(padNum >= 0, 0x671);

    if (!IsSignedIn(padNum)) {
        return false;
    } else {
        return (XUserGetSigninState(padNum) == eXUserSigninState_SignedInToLive);
    }
}

bool PlatformMgr::IsPadAGuest(int padNum) const {
    XUSER_SIGNIN_INFO signinInfo;

    DWORD ret = XUserGetSigninInfo(padNum, 0, &signinInfo);

    if (ret == ERROR_NO_SUCH_USER) {
        return IsSignedIn(padNum);
    } else {
        MILO_ASSERT(ret != ERROR_SUCCESS, 0x929);

        return signinInfo.dwInfoFlags >> 1 & 1;
    }
}

void PlatformMgr::ShowOfferUI(int padNum) {
    DWORD trackingID;
    DWORD ret;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiMarketplaceUI(
                trackingID,
                padNum,
                XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST_BACKGROUND,
                0,
                -1
            );
        } else {
            ret = XShowMarketplaceUI(
                padNum, XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST_BACKGROUND, 0, -1
            );
        }

        if (ret != ERROR_SUCCESS) {
            MILO_NOTIFY("XShowMarketplaceUI failed (0x%x)", ret);
        }
    }
}

DWORD PlatformMgr::ShowDeviceSelectorUI(
    DWORD userIndex,
    DWORD contentType,
    DWORD contentFlags,
    ULARGE_INTEGER bytesRequested,
    DWORD *deviceID,
    XOVERLAPPED *overlapped
) {
    DWORD trackingID;
    DWORD ret;

    if (sXShowCallback(trackingID)) {
        ret = XShowNuiDeviceSelectorUI(
            trackingID,
            userIndex,
            contentType,
            contentFlags,
            bytesRequested,
            deviceID,
            overlapped
        );
    } else {
        ret = XShowDeviceSelectorUI(
            userIndex, contentType, contentFlags, bytesRequested, deviceID, overlapped
        );
    }

    return ret;
}

void PlatformMgr::RegionInit() {
    if (XGetGameRegion() != 0xFF) {
        SetRegion(kRegionEurope);
    } else {
        SetRegion(kRegionNA);
    }
}

const char *PlatformMgr::GetName(int padnum) const {
    if (IsSignedIn(padnum)) {
        char name[16];
        int ret = XUserGetName(padnum, name, 16);
        if (ret == 0) {
            return MakeString(name);
        }
    }
    static Symbol player("player");
    return MakeString("%s %i", Localize(player, nullptr, TheLocale), padnum + 1);
}
