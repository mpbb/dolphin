// Copyright 2024 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include "Common/SettingsHandler.h"

namespace
{
// The encrypted bytes corresponding to the following settings, in order:
//   "key" = "val"
Common::SettingsHandler::Buffer BUFFER_A{0x91, 0x91, 0x90, 0xEE, 0xD1, 0x2F, 0xF0, 0x34, 0x79};

// The encrypted bytes corresponding to the following settings, in order:
//   "key1" = "val1"
//   "key2" = "val2"
//   "foo" = "bar"
Common::SettingsHandler::Buffer BUFFER_B{
    0x91, 0x91, 0x90, 0xE2, 0x9A, 0x38, 0xFD, 0x55, 0x42, 0xEA, 0xC4, 0xF6, 0x5E, 0xF,  0xDF, 0xE7,
    0xC3, 0x0A, 0xBB, 0x9C, 0x50, 0xB1, 0x10, 0x82, 0xB4, 0x8A, 0x0D, 0xBE, 0xCD, 0x72, 0xF4};
}  // namespace

TEST(SettingsHandlerTest, EncryptSingleSetting)
{
  Common::SettingsHandler handler;
  handler.AddSetting("key", "val");
  Common::SettingsHandler::Buffer buffer = handler.GetBytes();

  EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), BUFFER_A.begin(), BUFFER_A.end()));
}

TEST(SettingsHandlerTest, DecryptSingleSetting)
{
  Common::SettingsHandler::Buffer buffer = BUFFER_A;
  Common::SettingsHandler handler(std::move(buffer));
  EXPECT_EQ(handler.GetValue("key"), "val");
}

TEST(SettingsHandlerTest, EncryptMultipleSettings)
{
  Common::SettingsHandler handler;
  handler.AddSetting("key1", "val1");
  handler.AddSetting("key2", "val2");
  handler.AddSetting("foo", "bar");
  Common::SettingsHandler::Buffer buffer = handler.GetBytes();

  EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), BUFFER_B.begin(), BUFFER_B.end()));
}

TEST(SettingsHandlerTest, DecryptMultipleSettings)
{
  Common::SettingsHandler::Buffer buffer = BUFFER_B;
  Common::SettingsHandler handler(std::move(buffer));
  EXPECT_EQ(handler.GetValue("key1"), "val1");
  EXPECT_EQ(handler.GetValue("key2"), "val2");
  EXPECT_EQ(handler.GetValue("foo"), "bar");
}

TEST(SettingsHandlerTest, SetBytesOverwritesExistingBuffer)
{
  Common::SettingsHandler::Buffer buffer = BUFFER_A;
  Common::SettingsHandler handler(std::move(buffer));
  ASSERT_EQ(handler.GetValue("key"), "val");
  ASSERT_EQ(handler.GetValue("foo"), "");

  Common::SettingsHandler::Buffer buffer2 = BUFFER_B;
  handler.SetBytes(std::move(buffer2));
  EXPECT_EQ(handler.GetValue("foo"), "bar");
  EXPECT_EQ(handler.GetValue("key"), "");
}

TEST(SettingsHandlerTest, GetValueOnSameInstance)
{
  Common::SettingsHandler handler;
  handler.AddSetting("key", "val");
  EXPECT_EQ(handler.GetValue("key"), "");

  Common::SettingsHandler::Buffer buffer = handler.GetBytes();
  handler.SetBytes(std::move(buffer));
  EXPECT_EQ(handler.GetValue("key"), "val");
}

TEST(SettingsHandlerTest, GetValueAfterReset)
{
  Common::SettingsHandler::Buffer buffer = BUFFER_A;
  Common::SettingsHandler handler(std::move(buffer));
  ASSERT_EQ(handler.GetValue("key"), "val");

  handler.Reset();
  EXPECT_EQ(handler.GetValue("key"), "");
}

// TODO: Add test coverage of the edge case fixed in
// https://github.com/dolphin-emu/dolphin/pull/8704.
