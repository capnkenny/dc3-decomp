#include "rndobj/Console.h"
#include "os/System.h"
#include "obj/DataFunc.h"

static RndConsole *gConsole = nullptr;

void RndConsole::Continue() {
    if (mDebugging)
        mDebugging = nullptr;
    else
        MILO_FAIL("Can't continue unless debugging");
}

void RndConsole::Help(Symbol sym) {
    if (mDebugging) {
        mOutput->Clear();
        if (sym == "step")
            *mOutput << "Step into command\n";
        else if (sym == "next")
            *mOutput << "Step over command\n";
        else if (sym == "finish")
            *mOutput << "Finish executing current scope\n";
        else if (sym == "continue")
            *mOutput << "Continue program execution\n";
        else if (sym == "list")
            *mOutput << "List file around current line\n";
        else if (sym == "where") {
            *mOutput << "List script stack\n\n";
            *mOutput << "The current line is outdented\n";
        } else if (sym == "up")
            *mOutput << "Move up script stack\n";
        else if (sym == "down")
            *mOutput << "Move down script stack\n";
        else if (sym == "set_break") {
            *mOutput << "Set dynamic breakpoint\n\n";
            *mOutput << "set_break - current line\n";
            *mOutput << "set_break <handler> - in current object\n";
            *mOutput << "set_break <obj> <handler> - in given object\n";
            *mOutput << "set_break <class> <type> <handler> - in given class and type\n";
        } else if (sym == "breakpoints")
            *mOutput << "List dynamic breakpoints\n";
        else if (sym == "clear") {
            *mOutput << "Clear dynamic breakpoint\n\n";
            *mOutput << "clear - current line\n";
            *mOutput << "clear <n> - numbered breakpoint\n";
            *mOutput << "clear all - all breakpoints\n";
        } else if (sym == "cppbreak")
            *mOutput << "Break into the C++ debugger.\n";
        else {
            *mOutput << "Help on debug commands:\n";
            *mOutput << "help step\n";
            *mOutput << "help next\n";
            *mOutput << "help finish\n";
            *mOutput << "help continue\n";
            *mOutput << "help list\n";
            *mOutput << "help where\n";
            *mOutput << "help up\n";
            *mOutput << "help down\n";
            *mOutput << "help set_break\n";
            *mOutput << "help breakpoints\n";
            *mOutput << "help clear\n\n";
            *mOutput << "help cppbreak\n\n";
            *mOutput << "eval <$var> - return variable value\n";
            *mOutput << "eval [<prop>] - return property value\n";
            *mOutput << "set <$var> <value> - set variable value\n";
            *mOutput << "set [<prop>] <value> - set property value\n";
            *mOutput << "<func> <args> - call script function\n";
            *mOutput << "<obj> <handler> <args> - call object handler\n";
        }
    } else
        MILO_FAIL("Can't help unless debugging");
}

void RndConsole::Where() {
    if (mDebugging) {
        mOutput->Clear();
        for (DataArray **it = &gCallStackPtr[-2]; it >= gCallStack; it--) {
            if (*it != mDebugging) {
                MILO_LOG("  ");
            }
            MILO_LOG("%s:%d\n", (*it)->File(), (*it)->Line());
        }
    } else
        MILO_FAIL("Can't where unless debugging");
}

DataNode DataContinue(DataArray *) {
    gConsole->Continue();
    return 0;
}

DataNode DataWhere(DataArray *) {
    gConsole->Where();
    return 0;
}

DataNode DataCppBreak(DataArray *) {
    PlatformDebugBreak();
    return 0;
}

DataNode DataHelp(DataArray *da) {
    gConsole->Help(da->Size() > 1 ? da->Sym(1) : "");
    return 0;
}

DataNode DataNop(DataArray *) { return 0; }

RndConsole::Breakpoint::~Breakpoint() {
    if (parent) {
        DataArray *cmd = parent->Command(index);
        cmd->Node(0) = DataNop;
    }
}

void RndConsole::Breakpoints() {
    mOutput->Clear();
    int idx = 1;
    for (std::list<Breakpoint>::iterator it = mBreakpoints.begin();
         it != mBreakpoints.end();
         ++it) {
        DataArray *cmd = it->parent->Command(it->index);
        MILO_LOG("%d. %s:%d\n", idx++, cmd->File(), cmd->Line());
    }
}

DataNode DataBreakpoints(DataArray *) {
    gConsole->Breakpoints();
    return 0;
}

void RndConsole::List() {
    if (mDebugging) {
        mOutput->Clear();
        File *f = NewFile(mDebugging->File(), 2);
        if (f) {
            int i = 1;
            int numlines = mOutput->NumLines() / 2;
            int i4 = mDebugging->Line() - numlines;
            int totalLines = numlines + mDebugging->Line();
            do {
                char buf;
                int read = f->Read(&buf, 1);
                if (i > i4) {
                    *mOutput << buf;
                }
                if (buf == '\n' || read == 0) {
                    i++;
                    if (i > i4 && i < totalLines) {
                        *mOutput << MakeString(
                            "%3d%c", i, i == mDebugging->Line() ? '>' : ':'
                        );
                    }
                }
            } while (i < totalLines);
            delete f;
        }
    } else
        MILO_FAIL("Can't list unless debugging");
}

DataNode DataList(DataArray *) {
    gConsole->List();
    return 0;
}

DataNode DataBreak(DataArray *da) {
    gConsole->Break(da);
    return 0;
}

DataNode DataUp(DataArray *) {
    gConsole->MoveLevel(-1);
    return 0;
}

DataNode DataDown(DataArray *) {
    gConsole->MoveLevel(1);
    return 0;
}

BEGIN_HANDLERS(RndConsole)
    HANDLE_MESSAGE(KeyboardKeyMsg)
END_HANDLERS

DataNode DataStep(DataArray *) {
    gConsole->Step(0);
    return 0;
}

DataNode DataNext(DataArray *) {
    gConsole->Step(-1);
    return 0;
}

DataNode DataFinish(DataArray *) {
    gConsole->Step(-2);
    return 0;
}

DataNode DataSetBreak(DataArray *da) {
    gConsole->SetBreak(da);
    return 0;
}

DataNode DataClear(DataArray *da) {
    int clearInt;
    if (da->Size() < 2)
        clearInt = 0;
    else if (da->Type(1) == kDataSymbol && da->Sym(1) == "all")
        clearInt = -1;
    else
        clearInt = da->Int(1);
    gConsole->Clear(clearInt);
    return 0;
}

RndConsole::RndConsole()
    : mShowing(0), mBuffer(), mTabLen(0), mCursor(0), mPumpMsgs(0), mDebugging(0),
      mLevel(0) {
    mBufPtr = mBuffer.begin();
    gConsole = this;
    mOutput = RndOverlay::Find("output", true);
    mInput = RndOverlay::Find("input", true);
    DataArray *rndCfg = SystemConfig("rnd");
    rndCfg->FindData("console_buffer", mMaxBuffer, true);
    DataRegisterFunc("break", DataBreak);
    DataRegisterFunc("continue", DataContinue);
    DataRegisterFunc("step", DataStep);
    DataRegisterFunc("next", DataNext);
    DataRegisterFunc("finish", DataFinish);
    DataRegisterFunc("list", DataList);
    DataRegisterFunc("where", DataWhere);
    DataRegisterFunc("set_break", DataSetBreak);
    DataRegisterFunc("clear", DataClear);
    DataRegisterFunc("breakpoints", DataBreakpoints);
    DataRegisterFunc("up", DataUp);
    DataRegisterFunc("down", DataDown);
    DataRegisterFunc("cppbreak", DataCppBreak);
    DataRegisterFunc("help", DataHelp);
}

RndConsole::~RndConsole() { TheDebug.SetReflect(nullptr); }
