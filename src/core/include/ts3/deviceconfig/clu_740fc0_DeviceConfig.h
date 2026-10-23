#pragma once
#include <cstdint>
namespace ts3::deviceconfig {
// TS3W.exe 0x007D04A0: ReadTelemetry bridge.
int32_t ReadTelemetry();
// TS3W.exe 0x007D0510: WriteTelemetry bridge.
uint8_t WriteTelemetry(int32_t data);
// TS3W.exe 0x00740FC0: AuthenticateDisc bridge.
int32_t AuthenticateDisc();
// TS3W.exe 0x00740FD0: SetRegistryTelemetry bridge.
uint8_t SetRegistryTelemetry(uint8_t enabled);
}
