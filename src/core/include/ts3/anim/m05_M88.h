#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m88 {
// TS3W.exe 0x0057CE30: Finalize weighted SRTs or copy bind pose, then clear weights; preserves incidental EAX.
uintptr_t Finalize(b20::Pose*);
// TS3W.exe 0x0057D060: Form a shortest-arc quaternion, retaining EA's antiparallel-axis selection and EAX.
uintptr_t Rotation(const Float4Value*,const Float4Value*,Float4Value*);
}
