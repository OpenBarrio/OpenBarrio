#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m104 {
// TS3W.exe 0x00585D80: Sample twelve-byte compressed channels into a pose; vector W repeats X and quaternion zero pairs become identity.
bool Sample12(const void* clip,b20::Pose* pose,float time,const int32_t* binding,uint8_t finish);
// TS3W.exe 0x00586300: Sample twenty-byte scaled compressed channels with the same interpolation quirks.
bool Sample20(const void* clip,b20::Pose* pose,float time,const int32_t* binding,uint8_t finish);
}
