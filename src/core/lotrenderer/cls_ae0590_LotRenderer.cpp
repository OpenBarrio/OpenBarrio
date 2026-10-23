// Unit cls_ae0590_LotRenderer - see cls_ae0590_LotRenderer.h for the contract.
// All six entries are __thiscall; cross-unit helpers go through the ts3/layout.h
// bridge; layouts widen their pointers for the native build.
#include "ts3/lotrenderer/cls_ae0590_LotRenderer.h"
#include <cmath>
#include <limits>
#include <cstring>

namespace ts3 {
namespace lotrenderer {

// 0x00AE0590: pure leaf.  asm: mov eax,3; cmp eax,[ecx+0x24]; sbb eax,eax; neg eax;
// ret.  Returns (uint32)[this+0x24] > 3u as full EAX (0 or 1).  Read-only and free of
// game calls / game addresses, so the body is width-identical (no #if _M_IX86 needed).
uint32_t LotRenderer7AE0590::IsEnabledAE0590() {
  return (mode24 > 3u) ? 1u : 0u;
}

namespace {
float QuietGridValue(float value) {
  // FLD/FSTP in AFBB40 quiets signaling NaNs; AFBCB0's MOVSS does not.
  std::uint32_t bits;
  std::memcpy(&bits, &value, sizeof(bits));
  if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
  std::memcpy(&value, &bits, sizeof(value));
  return value;
}
std::uint32_t* LowerBound(std::uint32_t* first, std::uint32_t* last, std::uint32_t key) {
  if (first == last) return first;
  auto count = last - first;
  while (count > 0) {
    auto half = count / 2;
    if (first[half] >= key) count = half;
    else { first += half + 1; count -= half + 1; }
  }
  return first;
}
std::intptr_t Insert(LotKeyVector& vector, std::uint32_t* position, std::uint32_t& key) {
  if (position == vector.end || key < *position) {
    if (vector.end == vector.capacity || position != vector.end)
      return CallGameMethod<std::intptr_t, 0x008621C0>(&vector, position, &key);
    auto destination = vector.end++;
    if (destination) *destination = key;
  }
  return reinterpret_cast<std::intptr_t>(position);
}
int SignedByte(std::uint8_t value) { return value < 128 ? value : int(value) - 256; }
bool Present(const LotRendererState& state, std::uint32_t col, std::uint32_t row, std::int32_t& byte) {
  const std::uint32_t bit = row + state.height * col;
  // SAR rather than an unsigned shift, including wrapped indices.
  byte = static_cast<std::int32_t>(bit) >> 3;
  return (state.mask[byte] & (1u << (bit & 7))) != 0;
}
bool FindWall(LotRendererState& state, const WallPair& key) {
  void* iterator;
  auto found = CallGameMethod<void**, 0x00C4CF00>(&state.scratch, &iterator, &key);
  return *found != state.scratch.sentinel;
}
std::uint8_t RoundByte(float value) {
  const double rounded = std::nearbyint(static_cast<double>(value));
  const auto integer = !std::isfinite(rounded) || rounded >= 2147483648.0 || rounded < -2147483648.0
      ? std::numeric_limits<std::int32_t>::min() : static_cast<std::int32_t>(rounded);
  return static_cast<std::uint8_t>(integer);
}
}

// 0x00AFBB40, thiscall, ret 4. EAX is the last mask byte index or vector-helper result.
std::intptr_t LotRenderer_BuildLotKeysAFBB40(void* self, const LotKeyInput* input) {
  auto& state = *static_cast<LotRendererState*>(self);
  std::intptr_t result = 0;
  for (auto entry = input->begin; entry != input->end; ++entry) {
    std::uint32_t key = entry->col + state.width * entry->row;
    state.grid[key].first = QuietGridValue(entry->value);
    Insert(state.keysB, LowerBound(state.keysB.begin, state.keysB.end, key), key);
    std::int32_t byte;
    const bool present = Present(state, entry->col, entry->row, byte);
    result = byte;
    if (present) {
      auto position = CallGameFunction<std::uint32_t*, 0x006D8BD0>(state.keysA.begin, state.keysA.end, &key, state.keysA.reserved & 255u);
      result = Insert(state.keysA, position, key);
    }
  }
  return result;
}

// 0x00AFBCB0, thiscall, ret 12. The Vec2 second component preserves the input bits.
std::intptr_t LotRenderer_InsertOneKeyAFBCB0(void* self, int col, int row, float value) {
  auto& state = *static_cast<LotRendererState*>(self);
  std::uint32_t key = static_cast<std::uint32_t>(col) + state.width * static_cast<std::uint32_t>(row);
  state.grid[key].second = value;
  auto position = CallGameFunction<std::uint32_t*, 0x006D8BD0>(state.keysB.begin, state.keysB.end, &key, state.keysB.reserved & 255u);
  Insert(state.keysB, position, key);
  std::int32_t byte;
  if (!Present(state, static_cast<std::uint32_t>(col), static_cast<std::uint32_t>(row), byte)) return byte;
  struct Result { std::uint32_t* position; std::uint8_t inserted; } output;
  return CallGameMethod<std::intptr_t, 0x00B1E750>(&state.keysA, &output, &key);
}

// 0x00AFCA80, thiscall, ret 8. Signed coordinate bytes are OR-packed, not masked.
uint8_t LotRenderer_UpdateWallTypeAFCA80(void* self, uint8_t col, uint8_t row) {
  auto& state = *static_cast<LotRendererState*>(self);
  for (std::uint8_t plane = 0; plane < 2; ++plane) {
    WallCoordinate coordinate{col, row, plane};
    const auto key = std::uint32_t(SignedByte(col)) | ((std::uint32_t(SignedByte(row)) | (std::uint32_t(plane) << 10)) << 10);
    auto node = state.buckets[key % state.bucketCount];
    while (node && node->key != key) node = node->next;
    if (!node) node = state.buckets[state.bucketCount];
    if (node != state.buckets[state.bucketCount]) {
      const int previousType = node->type;
      const int type = CallGameMethod<std::int16_t, 0x00AA2870>(state.owner->service, &coordinate);
      if (previousType != type) CallGameMethod<void, 0x00AFC070>(self, &coordinate, type);
    }
  }
  return 2;
}

// 0x00C552C0, thiscall, ret 4. Failure gates return full EAX, despite testing only AL.
int LotRenderer_ProcessWallCellC552C0(void* self, void* argument) {
  auto& state = *static_cast<LotRendererState*>(self);
  const auto input = static_cast<const WallPair*>(argument);
  WallPair key = *input;
  void* matchedNode = argument;
  auto found = CallGameMethod<void**, 0x00C4CF00>(&state.scratch, &matchedNode, &key);
  if (*found == state.scratch.sentinel) key = {input->to, input->from};
  auto wall = CallGameMethod<void*, 0x00C512D0>(&state.scratch, &key);
  int result = CallGameFunction<int, 0x00C5BBC0>(wall);
  if (!(result & 255)) return result;
  result = CallGameFunction<int, 0x00C5C0D0>(wall);
  if (!(result & 255)) return result;
  const auto flag = CallGameFunction<std::uint8_t, 0x00C5C320>(wall);
  auto type = [&] {
    if (CallGameFunction<std::uint8_t, 0x00C5BF70>(wall)) return 4;
    auto value = CallGameFunction<int, 0x00C5BAD0>(wall);
    return CallGameFunction<int, 0x00C4B460>(value);
  };
  CallGameMethod<void, 0x00C54620>(self, input, type());
  // The lookup reused the argument slot; AL replaces its iterator node's low byte.
  const auto packedFlag = (reinterpret_cast<std::uintptr_t>(matchedNode) & ~std::uintptr_t{255}) | flag;
  return CallGameMethod<int, 0x00C544A0>(self, input, type(), packedFlag);
}

// 0x00C563B0, thiscall, ret 4. First pass uses 1/2 floors; the second always uses two.
int LotRenderer_EnumerateWallKeysC563B0(void* self, const WallCoordinate* coordinate) {
  auto& state = *static_cast<LotRendererState*>(self);
  const int floors = (SignedByte(coordinate->level) > state.threshold) + 1;
  struct Position { float x, y; std::int32_t level; } position{-1.0f, -1.0f, 0};
  CallGameFunction<int, 0x00ADD600>(coordinate, &position);
  // Constants read from 0x010779E4 / 0x01077A04 / 0x0107819C / 0x010781BC.
  constexpr float dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  constexpr float dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  constexpr float ax[4] = {-1, 1, -1, 1}, ay[4] = {0, 0, 0, 0};
  constexpr float bx[4] = {0, 0, 0, 0}, by[4] = {-1, -1, 1, 1};
  auto process = [&](WallPair key) {
    if (!FindWall(state, key)) {
      key = {key.to, key.from};
      if (!FindWall(state, key)) return;
    }
    LotRenderer_ProcessWallCellC552C0(self, &key);
  };
  for (int floor = 0; floor < floors; ++floor) {
    const auto level = static_cast<std::uint8_t>(position.level - floor);
    for (int i = 0; i < 8; ++i)
      process({{RoundByte(position.x), RoundByte(position.y), level}, {RoundByte(position.x + dx[i]), RoundByte(position.y + dy[i]), level}});
  }
  for (int floor = 0; floor < 2; ++floor) {
    const auto level = static_cast<std::uint8_t>(position.level - floor);
    for (int i = 0; i < 4; ++i)
      process({{RoundByte(position.x + bx[i]), RoundByte(position.y + by[i]), level}, {RoundByte(position.x + ax[i]), RoundByte(position.y + ay[i]), level}});
  }
  return 2;
}

}  // namespace lotrenderer
}  // namespace ts3
