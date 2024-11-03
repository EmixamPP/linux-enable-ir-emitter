#include "../src/camera/camerainstruction.hpp"

#include <gtest/gtest.h>

constexpr CameraInstruction::Unit UNIT{66};
constexpr CameraInstruction::Unit SELECTOR{99};
static const CameraInstruction::Control INIT{1, 3, 1, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control NO_MAX{};
static const CameraInstruction::Control MAX_UINT8{255, 255, 255, 255, 255, 255, 255, 255, 255};
static const CameraInstruction::Control MAX{1, 3, 3, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control NO_MIN{};
static const CameraInstruction::Control MIN{1, 3, 0, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control NEW_CUR{1, 3, 2, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control BAD_NEW_CUR_0{1, 3, 2, 0, 0, 0, 0, 0};
static const CameraInstruction::Control BAD_NEW_CUR_1{1, 3, 4, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control BAD_NEW_CUR_2{1, 2, 2, 0, 0, 0, 0, 0, 0};
static const CameraInstruction::Control BAD_NEW_CUR_3{1, 3, 2, 1, 0, 0, 0, 0, 0};
constexpr auto disable = true;
constexpr auto not_disable = false;

TEST(CameraInstructionTest, set_cur) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, not_disable};
  ASSERT_TRUE(inst.set_cur(NEW_CUR));
  ASSERT_EQ(inst.cur(), NEW_CUR);
}

TEST(CameraInstructionTest, set_cur_fail) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, not_disable};
  ASSERT_FALSE(inst.set_cur(BAD_NEW_CUR_0));
  ASSERT_FALSE(inst.set_cur(BAD_NEW_CUR_1));
  ASSERT_FALSE(inst.set_cur(BAD_NEW_CUR_2));
  ASSERT_FALSE(inst.set_cur(BAD_NEW_CUR_3));
}

TEST(CameraInstructionTest, set_cur_error) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, disable};
  ASSERT_THROW(inst.set_cur(NEW_CUR), CameraInstructionException);
}

TEST(CameraInstructionTest, set_cur_max) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, not_disable};
  ASSERT_TRUE(inst.set_max_cur());
  ASSERT_EQ(inst.cur(), MAX);
}

TEST(CameraInstructionTest, set_cur_no_max) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, NO_MAX, MIN, not_disable};
  ASSERT_TRUE(inst.set_max_cur());
  ASSERT_EQ(inst.cur(), MAX_UINT8);
}

TEST(CameraInstructionTest, set_cur_max_error) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, disable};
  ASSERT_THROW(inst.set_max_cur(), CameraInstructionException);
}

TEST(CameraInstructionTest, set_cur_min) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, not_disable};
  ASSERT_TRUE(inst.set_min_cur());
  ASSERT_EQ(inst.cur(), MIN);
}

TEST(CameraInstructionTest, set_cur_min_fail) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, NO_MIN, not_disable};
  ASSERT_FALSE(inst.set_min_cur());
}

TEST(CameraInstructionTest, set_cur_min_error) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, disable};
  ASSERT_THROW(inst.set_min_cur(), CameraInstructionException);
}

TEST(CameraInstructionTest, reset) {
  CameraInstruction inst{UNIT, SELECTOR, NEW_CUR, INIT, MAX, MIN, not_disable};
  ASSERT_EQ(inst.cur(), NEW_CUR);  // ensure no bad state before actual test
  inst.reset();
  ASSERT_EQ(inst.cur(), INIT);
}

TEST(CameraInstructionTest, reset_error) {
  CameraInstruction inst{UNIT, SELECTOR, NEW_CUR, INIT, MAX, MIN, disable};
  ASSERT_THROW(inst.reset(), CameraInstructionException);
}

TEST(CameraInstructionTest, next) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, not_disable};
  ASSERT_EQ(inst.cur(), INIT);  // ensure no bad state before actual test
  ASSERT_EQ(inst.max(), MAX);   // ensure no bad state before actual test

  ASSERT_TRUE(inst.next());
  ASSERT_EQ(inst.cur(), NEW_CUR);

  ASSERT_TRUE(inst.next());
  ASSERT_EQ(inst.cur(), MAX);

  ASSERT_FALSE(inst.next());
}

TEST(CameraInstructionTest, next_error) {
  CameraInstruction inst{UNIT, SELECTOR, INIT, INIT, MAX, MIN, disable};
  ASSERT_THROW(inst.next(), CameraInstructionException);
}