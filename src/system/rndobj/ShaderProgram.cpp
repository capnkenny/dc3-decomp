#include "rndobj/ShaderProgram.h"
#include "ShaderMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "rndobj/ShaderOptions.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/MemMgr.h"
#include "math/Utl.h"

void RndShaderProgram::SaveShaderBuffer(const char *file, RndShaderBuffer &buffer) {
    FileMkDir(FileGetPath(file));
    File *f = NewFile(file, 0x301);
    f->Write(buffer.Storage(), buffer.Size());
    delete f;
}

void RndShaderProgram::LoadShaderBuffer(
    BinStream &bs, int size, RndShaderBuffer *&buffer
) {
    MemDoTempAllocations tmp;
    buffer = NewBuffer(size);
    bs.Read(buffer->Storage(), size);
}

void RndShaderProgram::LoadShaderBuffer(const char *cc, RndShaderBuffer *&buffer) {
    FileStream stream(cc, FileStream::kReadNoArk, true);
    LoadShaderBuffer(stream, stream.Size(), buffer);
}

unsigned long gModTime;

void ShaderRecurseCB(const char *dir, const char *file) {
    FileStat stat;
    MILO_ASSERT(FileGetStat(MakeString("%s/%s", dir, file), &stat) == 0, 0x1B);
    if (stat.st_mtime > gModTime) {
        gModTime = stat.st_mtime;
    }
}

unsigned long RndShaderProgram::InitModTime() {
    gModTime = 0;
    if (TheShaderMgr.CacheShaders()) {
        FileRecursePattern(
            MakeString("%s/shaders/*.fx", FileSystemRoot()), ShaderRecurseCB, false
        );
    }
    return gModTime;
}

void RndShaderProgram::CopyErrorShader(ShaderType shader, const ShaderOptions &opts) {
    if (!MainThread()) {
        MILO_NOTIFY(
            "missing shader %s_%llx cannot be cached (not used in main thread).",
            ShaderTypeName(shader),
            opts.flags
        );
    }
    MILO_ASSERT(shader != kErrorShader && shader != kPostprocessErrorShader, 0x12F);
    ShaderType errorType = kPostprocessShader ? kPostprocessErrorShader : kErrorShader;
    u64 mask = (errorType == kErrorShader && opts.flags & 0x1000) ? 0x1000 : 0;
    mask |= TheShaderMgr.GetShaderErrorDisplay() << 0x23;
    ShaderOptions newOpts(mask);
    RndShaderProgram &program = TheShaderMgr.FindShader(errorType, newOpts);
    if (!program.Cached()) {
        if (!TheShaderMgr.CacheShaders()) {
            MILO_LOG(
                "FAILURE: Error shader cannot be cached. Unable to handle missing shaders!\n"
            );
            MILO_FAIL(
                "FAILURE: Error shader cannot be cached. Unable to handle missing shaders!\n"
            );
        }
        Cache(errorType, newOpts, nullptr, nullptr);
    }
    Copy(program);
}
