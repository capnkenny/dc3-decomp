#include "net_ham/DataMinerJobs.h"
#include "game/GamePanel.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveDir.h"
#include "hamobj/PoseFatalities.h"
#include "hamobj/PracticeSection.h"
#include "hamobj/ScoreUtl.h"
#include "math/Utl.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/DataPointMgr.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

GameEndedDataPointJob::GameEndedDataPointJob(
    Hmx::Object *callback, EndGameResult const &result
)
    : RCJob("dataminer/game_ended", callback) {
    static Symbol mode("mode");
    static Symbol song("song");
    static Symbol reason("reason");
    static Symbol song_position("song_position");
    static Symbol playlist_perform("playlist_perform");
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    static Symbol perform_legacy("perform_legacy");
    static Symbol practice("practice");
    static Symbol showdown("showdown");
    static Symbol score("score");
    static Symbol num_stars("num_stars");
    static Symbol perf_no_flashcard("perf_no_flashcard");
    static Symbol perf_current_stars("perf_current_stars");
    static Symbol photos_disabled("photos_disabled");
    static Symbol freestyle_disabled("freestyle_disabled");
    static Symbol num_bid_vcmds("num_bid_vcmds");
    static Symbol num_shell_vcmds("num_shell_vcmds");
    static Symbol custom_session("custom_session");
    static Symbol challenge("challenge");

    Symbol lastPlayed = MetaPerformer::Current()->LastPlayedMode();
    float f29 = 0;
    Symbol gameDataSong = TheGameData->GetSong();

    if (TheMaster && TheMaster->IsLoaded()) {
        float streamMs = TheMaster->StreamMs();
        float durMs = TheMaster->SongDurationMs();
        f29 = Clamp(0.0f, 1.0f, streamMs / durMs);
    }

    DataPoint pt;
    pt.AddPair(mode, lastPlayed);
    pt.AddPair(song, gameDataSong);
    pt.AddPair(reason, (int)result);
    pt.AddPair(song_position, f29);

    if (lastPlayed == showdown || lastPlayed == playlist_perform || lastPlayed == perform
        || lastPlayed == dance_battle || lastPlayed == perform_legacy
        || lastPlayed == challenge) {
        int numStars = 0;
        const DataNode *prop = TheGamePanel->Property(num_stars, false);
        if (prop) {
            numStars = prop->Float();
        }
        pt.AddPair(perf_current_stars, numStars);
        pt.AddPair(
            perf_no_flashcard, MetaPerformer::Current()->CompletedSongWithNoFlashcards()
        );
    } else {
        MetaPerformer *p = MetaPerformer::Current();
        if (p->GetMoveScore(0).size() != 0) {
            MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
            MILO_ASSERT(moves, 0x5A);
            PracticeSection *section = nullptr;
            for (ObjDirItr<PracticeSection> it(moves, true); it != nullptr; ++it) {
                section = &*it;
                if (it->GetDifficulty() == TheGameData->Player(0)->GetDifficulty()) {
                    break;
                }
            }
            MILO_ASSERT(section, 0x64);
            int numSteps = section->Steps().size();
            int numMoveScores = p->GetMoveScore(0).size();
            if (numSteps < numMoveScores) {
                String str = MakeString("(%d/%d)", numMoveScores, numSteps);
                pt.AddPair(custom_session, str);
            }
        }
    }
    Symbol playerCrew = gNullStr;
    Symbol playerChar = gNullStr;
    for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        playerCrew = hpd->Crew();
        playerChar = hpd->Char();
        char idx[8];
        itoa(i, idx, 10);
        String crewStr("crew");
        crewStr += idx;
        String charStr("character");
        charStr += idx;
        String diffStr("diff");
        diffStr += idx;
        String scoreStr("perf_current_score");
        scoreStr += idx;
        String str1f0;
        String ratingStr("move_ratings=");
        ratingStr += idx;
        bool b21 = true;
        if (lastPlayed == perform || lastPlayed == dance_battle
            || lastPlayed == perform_legacy) {
            str1f0 = "perf_move_ratings";
            str1f0 += idx;
        } else if (lastPlayed == practice) {
            str1f0 = "pract_move_ratings";
            str1f0 += idx;
        } else {
            b21 = false;
        }
        if (b21 && CompileMoveRatings(ratingStr, i, lastPlayed == practice)) {
            pt.AddPair(str1f0.c_str(), ratingStr);
        }
        static Symbol score("score");
        pt.AddPair(crewStr.c_str(), playerCrew);
        pt.AddPair(charStr.c_str(), playerChar);
        HamPlayerData *curPlayer = TheGameData->Player(i);
        pt.AddPair(diffStr.c_str(), curPlayer->GetDifficulty());
        pt.AddPair(scoreStr.c_str(), curPlayer->Provider()->Property(score)->Int());
        int padnum = curPlayer->PadNum();
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(padnum);
        if (profile && profile->HasValidSaveData()) {
        }
    }
}

bool GameEndedDataPointJob::CompileMoveRatings(
    String &str, int playerIndex, bool b3
) const {
    MILO_ASSERT(playerIndex >= 0 && playerIndex < MAX_NUM_PLAYERS, 0x120);
    bool ret = false;
    const std::vector<HamMoveScore> &moveScores =
        MetaPerformer::Current()->GetMoveScore(playerIndex);
    if (moveScores.size() != 0) {
        char buffer[56];
        bool b4 = true;
        for (int i = 0; i < moveScores.size(); i++) {
            if (!moveScores[i].unk0->IsRest()) {
                ret = true;
                String moveName = moveScores[i].unk0->Name();
                if (SearchReplace(moveName.c_str(), "&", "%26", buffer)) {
                    moveName = buffer;
                }
                if (!b4) {
                    str += "|";
                }
                if (!b3) {
                    str += MakeString(
                        "%s:%.2f%%20(%s)",
                        moveName,
                        moveScores[i].unk8,
                        RatingState(moveScores[i].unk4).Str() + 5
                    );
                } else if (moveScores[i].unk4 < 0) {
                    const char *speed;
                    if (moveScores[i].unk4 == -4) {
                        speed = "fast";
                    } else if (moveScores[i].unk4 == -3) {
                        speed = "pass";
                    } else {
                        speed = "fail";
                    }
                    str += MakeString(
                        "%s:%s%%20(slowmo:%d)", moveName, speed, moveScores[i].unkc ? 1 : 0
                    );
                } else {
                    const char *rating;
                    if (moveScores[i].unk4 == 0) {
                        rating = "perfect";
                    } else if (moveScores[i].unk4 == 1) {
                        rating = "awesome";
                    } else if (moveScores[i].unk4 == 3) {
                        rating = "ok";
                    } else {
                        rating = "bad";
                    }
                    str += MakeString(
                        "%s:%.2f%%20(%s)%%20(slowmo:%d)",
                        moveName,
                        moveScores[i].unk8,
                        rating,
                        moveScores[i].unkc ? 1 : 0
                    );
                }
                b4 = false;
            }
        }
    }
    return ret;
}

char const *GameEndedDataPointJob::GetXUIDStrFromProfile(HamProfile *profile) {
    int padNum = profile->GetPadNum();
    XUID xuid = 0;
    DWORD result = XUserGetXUID(padNum, &xuid);
    if (result != 0) {
        MILO_NOTIFY("XUserGetXUID returned %u", result);
    }
    return MakeString("%016I64X", xuid);
}

OmgScoresJob::OmgScoresJob(Hmx::Object *callback, int p1Score, int p2Score)
    : RCJob("dataminer/omg_scores/", callback) {
    static Symbol player1_score("player1_score");
    static Symbol player2_score("player2_score");
    DataPoint dataP;
    dataP.AddPair(player1_score, p1Score);
    dataP.AddPair(player2_score, p2Score);
    SetDataPoint(dataP);
}

PlayerDroppedInJob::PlayerDroppedInJob(Hmx::Object *callback, int playerIdx)
    : RCJob("dataminer/player_dropped_in/", callback) {
    static Symbol player_idx("player_idx");
    DataPoint dataP;
    dataP.AddPair(player_idx, playerIdx);
    SetDataPoint(dataP);
}

PlayerDroppedOutJob::PlayerDroppedOutJob(Hmx::Object *callback, int playerIdx)
    : RCJob("dataminer/player_dropped_out/", callback) {
    static Symbol player_idx("player_idx");
    DataPoint dataP;
    dataP.AddPair(player_idx, playerIdx);
    SetDataPoint(dataP);
}

ControllerModeJob::ControllerModeJob(Hmx::Object *callback, int enterCount, int exitCount)
    : RCJob("dataminer/controller_mode/", callback) {
    static Symbol enter_count("enter_count");
    static Symbol exit_count("exit_count");
    DataPoint dataP;
    dataP.AddPair(enter_count, enterCount);
    dataP.AddPair(exit_count, exitCount);
    SetDataPoint(dataP);
}

PlaylistChangedJob::PlaylistChangedJob(Hmx::Object *callback, Symbol name, int numSongs)
    : RCJob("dataminer/playlist_changed/", callback) {
    static Symbol playlist_name("playlist_name");
    static Symbol num_songs("num_songs");
    DataPoint dataP;
    dataP.AddPair(playlist_name, name);
    dataP.AddPair(num_songs, numSongs);
    SetDataPoint(dataP);
}

ScreenResJob::ScreenResJob(Hmx::Object *callback, _XVIDEO_MODE *videoMode)
    : RCJob("dataminer/screen_resolution/", callback) {
    DataPoint dataP;
    static Symbol dwDisplayWidth("dwDisplayWidth");
    dataP.AddPair(dwDisplayWidth, videoMode->dwDisplayWidth);
    static Symbol dwDisplayHeight("dwDisplayHeight");
    dataP.AddPair(dwDisplayHeight, videoMode->dwDisplayHeight);
    static Symbol fIsInterlaced("fIsInterlaced");
    dataP.AddPair(fIsInterlaced, videoMode->fIsInterlaced);
    static Symbol fIsWideScreen("fIsWidescreen");
    dataP.AddPair(fIsWideScreen, videoMode->fIsWideScreen);
    static Symbol fIsHiDef("fIsHiDef");
    dataP.AddPair(fIsHiDef, videoMode->fIsHiDef);
    static Symbol RefreshRate("RefreshRate");
    dataP.AddPair(RefreshRate, videoMode->RefreshRate);
    static Symbol VideoStandard("VideoStandard");
    dataP.AddPair(VideoStandard, videoMode->VideoStandard);
    SetDataPoint(dataP);
}
