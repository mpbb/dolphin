#pragma once

#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Core/HW/Memmap.h"
#include "Core/State.h"
#include "Core/System.h"

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

  void apply_offset(float x, float z) noexcept
  {
    x_ += x;
    z_ += z;
  }

public:
  float x_, y_, z_;

public:
  static const Ball CUE;
  static const Ball ONE;
  static const Ball TWO;
  static const Ball THREE;
  static const Ball FOUR;
  static const Ball FIVE;
  static const Ball SIX;
  static const Ball SEVEN;
  static const Ball EIGHT;
  static const Ball NINE;
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

  const Ball& GetBall(size_t ball_number) { return balls_.at(ball_number); }

  const std::vector<Ball>& GetBalls() { return balls_; }

private:
  WiiPlayRandomFloat rng_;
  std::vector<Ball> balls_;
};

class PerfectBreak
{
public:
  static void init(Core::System& system);
  static void update(Core::System& system);
  static std::vector<u8> state;
  static uint32_t count;
  static uint32_t best_seed;
  static int best_score;
};

std::vector<u8> PerfectBreak::state;
uint32_t PerfectBreak::count = 0;
uint32_t PerfectBreak::best_seed = 0;
int PerfectBreak::best_score = 0;

void PerfectBreak::init(Core::System& system)
{
  INFO_LOG_FMT(MASTER_LOG, "PerfectBreak Init!");
  State::Load(9);
  system.GetPowerPC().GetDebugInterface().SetBreakpoint(0x802c4afc);
  State::SaveToBuffer(PerfectBreak::state);
}

void PerfectBreak::update(Core::System &system)
{
  auto pc = system.GetPPCState().pc;
  Core::SetIsThrottlerTempDisabled(true);
  Memory::MemoryManager& memory = system.GetMemory();
  if (pc == 0x802c4afc)
  {

    int sunk_1 = memory.Read_U8(0x91b4cf7b);
    int sunk_2 = memory.Read_U8(0x91b4d6ab);
    int sunk_3 = memory.Read_U8(0x91b4dddb);
    int sunk_4 = memory.Read_U8(0x91b4e50b);
    int sunk_5 = memory.Read_U8(0x91b4ec3b);
    int sunk_6 = memory.Read_U8(0x91b4f36b);
    int sunk_7 = memory.Read_U8(0x91b4fa9b);
    int sunk_8 = memory.Read_U8(0x91b501cb);
    int sunk_9 = memory.Read_U8(0x91b508fb);

    int score = sunk_1 + sunk_2 + sunk_3 + sunk_4 + sunk_5 + sunk_6 + sunk_7 + sunk_8 + sunk_9;

    if (score > best_score)
    {
      best_score = score;
      best_seed = count;
      State::Save(10);
    }

    INFO_LOG_FMT(MASTER_LOG, "{} {} {} {} {} {} {} {} {} - {} {}", sunk_1, sunk_2, sunk_3, sunk_4, sunk_5, sunk_6, sunk_7, sunk_8, sunk_9, score, count);


    State::LoadFromBuffer(PerfectBreak::state);

    count += 1;

    WiiPlayRandomRack rack(count);

    const Ball& b1 = rack.GetBall(1);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b1.x_), 0x91B4D07C);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b1.y_), 0x91B4D080);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b1.z_), 0x91B4D084);

    const Ball& b2 = rack.GetBall(2);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b2.x_), 0x91B4D7AC);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b2.y_), 0x91B4D7B0);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b2.z_), 0x91B4D7B4);

    const Ball& b3 = rack.GetBall(3);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b3.x_), 0x91B4DEDC);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b3.y_), 0x91B4DEE0);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b3.z_), 0x91B4DEE4);

    const Ball& b4 = rack.GetBall(4);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b4.x_), 0x91B4E60C);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b4.y_), 0x91B4E610);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b4.z_), 0x91B4E614);

    const Ball& b5 = rack.GetBall(5);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b5.x_), 0x91B4ED3C);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b5.y_), 0x91B4ED40);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b5.z_), 0x91B4ED44);

    const Ball& b6 = rack.GetBall(6);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b6.x_), 0x91B4F46C);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b6.y_), 0x91B4F470);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b6.z_), 0x91B4F474);

    const Ball& b7 = rack.GetBall(7);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b7.x_), 0x91B4FB9C);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b7.y_), 0x91B4FBA0);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b7.z_), 0x91B4FBA4);

    const Ball& b8 = rack.GetBall(8);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b8.x_), 0x91B502CC);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b8.y_), 0x91B502D0);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b8.z_), 0x91B502D4);

    const Ball& b9 = rack.GetBall(9);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b9.x_), 0x91B509FC);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b9.y_), 0x91B50A00);
    memory.Write_U32(std::bit_cast<uint32_t, float>(b9.z_), 0x91B50A04);

    system.GetCPU().EnableStepping(false);
  }
}
