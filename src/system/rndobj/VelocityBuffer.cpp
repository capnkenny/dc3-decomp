#include "rndobj/VelocityBuffer.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndVelocityBuffer RndVelocityBuffer::sSingleton;

bool RndXfmCache::GetXfms(
    const RndMesh *__restrict mesh,
    unsigned int ui2,
    unsigned int ui3,
    const float *&floats
) const {
    if (ui2 + ui3 <= unk1b580) {
        const RndMesh *ui2_mesh = unk0[ui2];
        const RndMesh *__restrict what = unk0[ui2 + ui3 - 1];
        if (ui2_mesh == mesh && what == mesh) {
            floats = unk1f40[ui2];
            return true;
        }
    }
    floats = nullptr;
    return false;
}

RndVelocityBuffer::RndVelocityBuffer()
    : unk36be8(0), unk36c6c(0), mFrame(0), mVelocityTex(nullptr), mMat(nullptr),
      unk36c7c(nullptr) {
    memset(&unk8, 0, 0xa4);
}

void RndVelocityBuffer::CacheCameraSettings(RndCam *camera) {
    MILO_ASSERT(camera, 0x88);
    Transform tfa0;
    Hmx::Matrix4 me0;
    camera->GetViewProjectXfms(tfa0, me0);
    unk8 = tfa0 * me0;
    camera->GetDepthRangeValues(unk48);
    camera->GetCamFrustum(unk58, unk68);
    mCam = camera;
}

bool RndVelocityBuffer::AdvanceFrame(RndCam *cam) {
    unk36c80 = false;
    unk36c6c ^= 1;
    mFrame++;
    if (cam != unk36c7c) {
        unk36c7c = cam;
        mFrame = 0;
    }
    return mFrame >= 2;
}

void RndVelocityBuffer::AllocateData(
    unsigned int ui1, unsigned int ui2, unsigned int ui3
) {
    MILO_ASSERT(mVelocityTex == NULL, 0x45);
    mVelocityTex = Hmx::Object::New<RndTex>();
    mVelocityTex->SetBitmap(ui1 / 2, ui2 / 2, ui3, RndTex::kRendered, false, nullptr);
    MILO_ASSERT(mMat == NULL, 0x4A);
    mMat = Hmx::Object::New<RndMat>();
    mMat->SetPerPixelLit(false);
    mMat->SetBlend(BaseMaterial::kBlendSrc);
    mMat->SetZMode(kZModeDisable);
    CreateAndSetMetaMat(mMat);
}

void RndVelocityBuffer::DrawMesh(RndMesh *mesh) const {
    MILO_ASSERT(mesh, 0x123);
    MILO_ASSERT(mesh->Showing(), 0x124);
    MILO_ASSERT(TheRnd.DrawMode() == Rnd::kDrawVelocity, 0x125);
    if (mesh->Mat() && mesh->Mat()->GetZMode() != kZModeTransparent) {
        int i10 = Max(1, mesh->NumBones());
        const float *fptr;
        if (mXfmCaches[unk36c6c].GetXfms(mesh, 0, i10, fptr)) {
        }
    }

    // clang-format off
//   if ((*(int *)(mesh + 0x128) != 0) && (*(int *)(*(int *)(mesh + 0x128) + 0xb8) != 2)) {
//     mesh[0x17c] = (RndMesh)0x1;
//     uVar1 = this->field12_0x36c6c;
//     local_50[0] = (*(int *)(mesh + 0x154) - *(int *)(mesh + 0x150)) / 0x54;
//     lVar8 = (longlong)local_50[0];
//     pRVar2 = *(RndMesh **)(mesh + (uVar1 + 0x5d) * 4);
//     this_00 = this->mXfmCaches + uVar1;
//     lVar10 = 1;
//     if (1 < local_50[0]) {
//       lVar10 = lVar8;
//     }
//     lVar7 = lVar10;
//     pRVar6 = mesh;
//     bVar5 = RndXfmCache::GetXfms
//                       (this->mXfmCaches + (uVar1 ^ 1),(uint *)mesh,
//                        *(RndMesh **)(mesh + ((uVar1 ^ 1) + 0x5d) * 4),(uint)lVar10,(uint)local_60,
//                        (float **)this_00);
//     iVar9 = (int)lVar8;
//     if (bVar5) {
//       bVar5 = RndXfmCache::GetXfms
//                         (this_00,(uint *)pRVar6,pRVar2,(uint)lVar7,(uint)&local_58,(float **)this_00
//                         );
//       if (bVar5) {
//         if (iVar9 < 0x29) {
//           RndShaderMgr::SetMeshInfo((RndShaderMgr *)TheShaderMgr,iVar9,false);
//           RndShader::SelectConfig(this->mMat,0x18,false);
//           (**(code **)(*(int *)TheShaderMgr + 0x20))(TheShaderMgr,9,local_60[0],lVar10 * 3);
//           (**(code **)(*(int *)TheShaderMgr + 0x20))(TheShaderMgr,0x81,local_58._0_4_,lVar10 * 3);
//           (**(code **)(*(int *)TheShaderMgr + 0x18))
//                     (TheShaderMgr,0,
//                      ((CONCAT44(uVar1,uVar1) ^ 0x100000001) & 0x3ffffff) * 0x40 + lVar3 + 0x36bec);
//           (**(code **)(*(int *)TheShaderMgr + 0x18))
//                     (TheShaderMgr,4,((ulonglong)uVar1 & 0x3ffffff) * 0x40 + lVar3 + 0x36bec);
//           (**(code **)(*(int *)TheShaderMgr + 0x40))(TheShaderMgr,8,lVar3 + 0x48);
//           (**(code **)(**(int **)(mesh + 0x148) + 0x40))
//                     (*(int **)(mesh + 0x148),0,0xffffffffffffffff);
//           TheNgStats->mMotionBlurs = TheNgStats->mMotionBlurs + 1;
//         }
//         else {
//           if ((DAT_830a8bd8 & 1) == 0) {
//             DAT_830a8bd8 = DAT_830a8bd8 | 1;
//             local_58 = 0;
//             _DAT_830a8bd0 = 0x830a8bd0830a8bd0;
//             atexit(`public:_void___cdecl_RndVelocityBuffer::DrawMesh(class_RndMesh*)const_'::__l26::
//                    `dynamic_atexit_destructor_for_'_dw'');
//           }
//           pcVar4 = PathName((Object *)(mesh + *(int *)(*(int *)(mesh + 4) + 4) + 4));
//           local_58 = CONCAT44(pcVar4,local_58._4_4_);
//           local_60[0] = *(char **)(mesh + *(int *)(*(int *)(mesh + 4) + 4) + 0x24);
//           pcVar4 = MakeString<>("%s (%s): Has too many bones to apply object motion blur (%d bones o f max %d)"
//                                 ,local_60,(char **)&local_58,local_50,(int *)&DAT_8209f810);
//           bVar5 = _anon_774E6181::AddToStrings(pcVar4,(list<> *)&DAT_830a8bd0);
//           if (bVar5) {
//             Debug::Notify(&TheDebug,pcVar4);
//           }
//         }
//       }
//     }
//   }
    // clang-format on
}

void RndVelocityBuffer::FreeData() {
    RELEASE(mVelocityTex);
    RELEASE(mMat);
}

void RndVelocityBuffer::ResetFrame() { mFrame = 0; }
