#pragma once

#include "Common/ChunkFile.h"
#include "Common/Logging/Log.h"

#include "Core/Core.h"
#include "Core/HW/Memmap.h"
#include "Core/State.h"
#include "Core/System.h"

#include <bit>
#include <cstdio>
#include <vector>

// We are using C++ 20 features
// Make sure the compiler supports them

// Floating point arithmetic must be IEC 559 (IEEE 754)
// Assert that the system supports it
static_assert(std::numeric_limits<float>::is_iec559);

class Ball
{
public:
  explicit Ball(float x, float y, float z) noexcept : x_(x), y_(y), z_(z) {}

  explicit Ball(uint32_t iec559_x, uint32_t iec559_y, uint32_t iec559_z) noexcept
      : x_(std::bit_cast<float, uint32_t>(iec559_x)), y_(std::bit_cast<float, uint32_t>(iec559_y)),
        z_(std::bit_cast<float, uint32_t>(iec559_z))
  {
  }

  bool operator==(const Ball&) const = default;

  float x_float() const { return x_; }
  float y_float() const { return y_; }
  float z_float() const { return z_; }

  uint32_t x_iec559() const { return std::bit_cast<uint32_t, float>(x_); }
  uint32_t y_iec559() const { return std::bit_cast<uint32_t, float>(y_); }
  uint32_t z_iec559() const { return std::bit_cast<uint32_t, float>(z_); }

  void apply_offset(float x, float z) noexcept
  {
    x_ += x;
    z_ += z;
  }

private:
  float x_, y_, z_;

public:
  static const Ball CUE, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE;
};

const Ball Ball::CUE(0xc2480000u, 0x40383958u, 0x00000000u);
const Ball Ball::ONE(0x42480000u, 0x40383958u, 0x00000000u);
const Ball Ball::TWO(0x42920e56u, 0x40383958u, 0x00000000u);
const Ball Ball::THREE(0x42760e56u, 0x40383958u, 0x40b83958u);
const Ball Ball::FOUR(0x42760e56u, 0x40383958u, 0xc0b83958u);
const Ball Ball::FIVE(0x425f072bu, 0x40383958u, 0x40383958u);
const Ball Ball::SIX(0x425f072bu, 0x40383958u, 0xc0383958u);
const Ball Ball::SEVEN(0x42868ac0u, 0x40383958u, 0x40383958u);
const Ball Ball::EIGHT(0x42868ac0u, 0x40383958u, 0xc0383958u);
const Ball Ball::NINE(0x42760e56u, 0x40383958u, 0x00000000u);

class WiiPlayRandomFloat
{
public:
  explicit WiiPlayRandomFloat(uint32_t seed) noexcept : seed_(seed) {}

  float random() noexcept
  {
    seed_ = seed_ * 0x10DCD + 1;
    return static_cast<float>((seed_ >> 16) & 0xffff) / static_cast<float>(0x10000);
  }

private:
  uint32_t seed_;
};

class WiiPlayRandomRack
{
public:
  WiiPlayRandomRack(uint32_t seed) noexcept
      : rng_(seed), balls_{Ball::CUE,  Ball::ONE, Ball::TWO,   Ball::THREE, Ball::FOUR,
                           Ball::FIVE, Ball::SIX, Ball::SEVEN, Ball::EIGHT, Ball::NINE}
  {
    for (Ball& ball : balls_)
    {
      if (ball == Ball::CUE)
        continue;

      float offset_x = (rng_.random() * 0.28785f) - 0.143925f;
      float offset_y = (rng_.random() * 0.28785f) - 0.143925f;

      ball.apply_offset(offset_x, offset_y);
    }
  }

  const Ball& GetBall(size_t ball_number) const { return balls_.at(ball_number); }

  const std::vector<Ball>& GetBalls() const { return balls_; }

private:
  WiiPlayRandomFloat rng_;
  std::vector<Ball> balls_;
};

struct BreakState
{
  struct
  {
    struct
    {
      float x, y, z;
    } pos;
    uint32_t sunk;
  } CUE, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE;
};

template <size_t size>
struct MemBuf
{
  uint8_t bytes[size];
};

class RackAttack
{
public:
  static void Init();
  static void Update();

private:
  static Core::System& system;

  static int frame_number;
  static int frames_idle;
  static uint32_t seed;
  static int best_score;
  static int best_seed;
  static BreakState last_state;
  static std::vector<u8> save;
  static MemBuf<0x6600> mem;
};


Core::System& RackAttack::system = Core::System::GetInstance();

int RackAttack::frame_number;
int RackAttack::frames_idle;
uint32_t RackAttack::seed;
BreakState RackAttack::last_state;
std::vector<u8> RackAttack::save;
MemBuf<0x6600> RackAttack::mem;
int RackAttack::best_score;
int RackAttack::best_seed;

void RackAttack::Init()
{
  NOTICE_LOG_FMT(MASTER_LOG, "RackAttack Init!");

  seed = 0;
  frame_number = 0;
  frames_idle = 0;
  best_score = 0;
  best_seed = seed;

  State::Load(3);

  Memory::MemoryManager& memory = system.GetMemory();

  memory.CopyFromEmu(&mem, 0x91b4bf80, sizeof(mem));
}

void RackAttack::Update()
{
  BreakState state;

  Memory::MemoryManager& memory = system.GetMemory();

  //PowerPC::MMU::

  memory.CopyFromEmu(&state.CUE.pos, 0x91B4C8BC, sizeof(state.CUE.pos));
  memory.CopyFromEmu(&state.ONE.pos, 0x91B4D07C, sizeof(state.ONE.pos));
  memory.CopyFromEmu(&state.TWO.pos, 0x91B4D7AC, sizeof(state.TWO.pos));
  memory.CopyFromEmu(&state.THREE.pos, 0x91B4DEDC, sizeof(state.THREE.pos));
  memory.CopyFromEmu(&state.FOUR.pos, 0x91B4E60C, sizeof(state.FOUR.pos));
  memory.CopyFromEmu(&state.FIVE.pos, 0x91B4ED3C, sizeof(state.FIVE.pos));
  memory.CopyFromEmu(&state.SIX.pos, 0x91B4F46C, sizeof(state.SIX.pos));
  memory.CopyFromEmu(&state.SEVEN.pos, 0x91B4FB9C, sizeof(state.SEVEN.pos));
  memory.CopyFromEmu(&state.EIGHT.pos, 0x91B502CC, sizeof(state.EIGHT.pos));
  memory.CopyFromEmu(&state.NINE.pos, 0x91B509FC, sizeof(state.NINE.pos));

  memory.CopyFromEmu(&state.CUE.sunk, 0x91B4C7BB, sizeof(state.CUE.sunk));
  memory.CopyFromEmu(&state.ONE.sunk, 0x91B4CF7B, sizeof(state.ONE.sunk));
  memory.CopyFromEmu(&state.TWO.sunk, 0x91B4D6AB, sizeof(state.TWO.sunk));
  memory.CopyFromEmu(&state.THREE.sunk, 0x91B4DDDB, sizeof(state.THREE.sunk));
  memory.CopyFromEmu(&state.FOUR.sunk, 0x91B4E50B, sizeof(state.FOUR.sunk));
  memory.CopyFromEmu(&state.FIVE.sunk, 0x91B4EC3B, sizeof(state.FIVE.sunk));
  memory.CopyFromEmu(&state.SIX.sunk, 0x91B4F36B, sizeof(state.SIX.sunk));
  memory.CopyFromEmu(&state.SEVEN.sunk, 0x91B4FA9B, sizeof(state.SEVEN.sunk));
  memory.CopyFromEmu(&state.EIGHT.sunk, 0x91B501CB, sizeof(state.EIGHT.sunk));
  memory.CopyFromEmu(&state.NINE.sunk, 0x91B508FB, sizeof(state.NINE.sunk));

  state.CUE.pos.y = 0.0f;
  state.ONE.pos.y = 0.0f;
  state.TWO.pos.y = 0.0f;
  state.THREE.pos.y = 0.0f;
  state.FOUR.pos.y = 0.0f;
  state.FIVE.pos.y = 0.0f;
  state.SIX.pos.y = 0.0f;
  state.SEVEN.pos.y = 0.0f;
  state.EIGHT.pos.y = 0.0f;
  state.NINE.pos.y = 0.0f;

  if (0 == memcmp(&last_state, &state, sizeof(BreakState)))
  {
    frames_idle += 1;
    //NOTICE_LOG_FMT(MASTER_LOG, "IDLE FRAME {}", frames_idle);
  }
  else
  {
    frames_idle = 0;
  }

  last_state = state;

  if (frames_idle > 3)
  {
          frames_idle = -1;

          int score = state.ONE.sunk + state.TWO.sunk + state.THREE.sunk + state.FOUR.sunk +
                      state.FIVE.sunk + state.SIX.sunk + state.SEVEN.sunk + state.EIGHT.sunk +
                      state.NINE.sunk;

          if (score > best_score)
          {
            best_score = score;
            State::Save(10);
          }

          OSD::AddMessage(fmt::format("SCORE: {}, SEED: {:#010x} | BEST: {}", score, seed, best_score));

          NOTICE_LOG_FMT(MASTER_LOG, "{} {} {} {} {} {} {} {} {} - {} {}", state.ONE.sunk,
                         state.TWO.sunk, state.THREE.sunk, state.FOUR.sunk, state.FIVE.sunk,
                         state.SIX.sunk, state.SEVEN.sunk, state.EIGHT.sunk, state.NINE.sunk, score,
                         seed);

          memory.CopyToEmu(0x91b4bf80, &mem, sizeof(mem));

          WiiPlayRandomRack rack(seed++);

          const Ball& b1 = rack.GetBall(1);
          memory.Write_U32(b1.x_iec559(), 0x91B4D07C);
          memory.Write_U32(b1.y_iec559(), 0x91B4D080);
          memory.Write_U32(b1.z_iec559(), 0x91B4D084);

          const Ball& b2 = rack.GetBall(2);
          memory.Write_U32(b2.x_iec559(), 0x91B4D7AC);
          memory.Write_U32(b2.y_iec559(), 0x91B4D7B0);
          memory.Write_U32(b2.z_iec559(), 0x91B4D7B4);

          const Ball& b3 = rack.GetBall(3);
          memory.Write_U32(b3.x_iec559(), 0x91B4DEDC);
          memory.Write_U32(b3.y_iec559(), 0x91B4DEE0);
          memory.Write_U32(b3.z_iec559(), 0x91B4DEE4);

          const Ball& b4 = rack.GetBall(4);
          memory.Write_U32(b4.x_iec559(), 0x91B4E60C);
          memory.Write_U32(b4.y_iec559(), 0x91B4E610);
          memory.Write_U32(b4.z_iec559(), 0x91B4E614);

          const Ball& b5 = rack.GetBall(5);
          memory.Write_U32(b5.x_iec559(), 0x91B4ED3C);
          memory.Write_U32(b5.y_iec559(), 0x91B4ED40);
          memory.Write_U32(b5.z_iec559(), 0x91B4ED44);

          const Ball& b6 = rack.GetBall(6);
          memory.Write_U32(b6.x_iec559(), 0x91B4F46C);
          memory.Write_U32(b6.y_iec559(), 0x91B4F470);
          memory.Write_U32(b6.z_iec559(), 0x91B4F474);

          const Ball& b7 = rack.GetBall(7);
          memory.Write_U32(b7.x_iec559(), 0x91B4FB9C);
          memory.Write_U32(b7.y_iec559(), 0x91B4FBA0);
          memory.Write_U32(b7.z_iec559(), 0x91B4FBA4);

          const Ball& b8 = rack.GetBall(8);
          memory.Write_U32(b8.x_iec559(), 0x91B502CC);
          memory.Write_U32(b8.y_iec559(), 0x91B502D0);
          memory.Write_U32(b8.z_iec559(), 0x91B502D4);

          const Ball& b9 = rack.GetBall(9);
          memory.Write_U32(b9.x_iec559(), 0x91B509FC);
          memory.Write_U32(b9.y_iec559(), 0x91B50A00);
          memory.Write_U32(b9.z_iec559(), 0x91B50A04);
  }

  //int score = sunk_1 + sunk_2 + sunk_3 + sunk_4 + sunk_5 + sunk_6 + sunk_7 + sunk_8 + sunk_9;

  //notice_log_fmt(master_log, "{} {} {} {} {} {} {} {} {} - {}", sunk_1, sunk_2, sunk_3, sunk_4,
  //               sunk_5, sunk_6, sunk_7, sunk_8, sunk_9, score);
}
