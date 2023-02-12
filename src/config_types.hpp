#pragma once
#include "helpers.hpp"
#include <array>

// Options for how the stick will move the mouse, scroll wheel, etc.
enum class MovementMode {
  REWIND, // do a motion then unwind the cursor position when the stick is released
  STUTTER, // move, then unwind after `stutter_step` movement, then repeat, until the stick is released.
  SIMPLE, // don't unwind at all, just move the cursor
  CHASE, // move, then activate keyboard command, continuously. This doesn't work that great, honestly.
  SCROLL // translate stick movements into vertical and horizontal scroll wheel events.
};

// A curve used to interpolate stick positions.
// There's not much point increasing the size (resolution) of the curve. The joysticks aren't sensitive enough to benefit.
using StickCurve = std::array<int16_t, 10>;

constexpr auto INVERT_AXIS = -1;
constexpr auto NO_INVERT_AXIS = 1;
constexpr auto NULL_AXIS = 0;

// Create a new StickMode and add it to `stickModes` in main to create a new operation mode for a stick.
struct StickMode {
  const MovementMode move; // which movement mode?
  const StickCurve& curve; // how fast and on what curve to move the mouse or wheel?
  const uint32_t indicator = rgb(255, 0, 0); // what color should the indicator be?
  const bool activeButtons[3] = {false, false, false}; // which buttons to press? left, middle, right

  const int activeKey = 0; // which key to hold down during mouse motion
  
  const int8_t horDir = INVERT_AXIS; // set positive (1) or inverted motion (-1), or 0 for null axis.
  const int8_t vertDir = INVERT_AXIS; // same as horDir.

  const int chaseKey = 0; // key to press after each motion step to "chase"
  const int chaseMods = 0; // modifiers to press along with the chase key.

  const int motionThreshold = 1; // how many pixels of movement are required to trigger mouse motion?


  constexpr bool hasButtons() const {
    return activeButtons[0] || activeButtons[1] || activeButtons[2];
  }

  constexpr bool hasKey() const {
    return activeKey;
  }
};


// build a curve of the form `maxSpeed * x^(e*expCoef)`.
// this is a basic "expo" function like found on RC controller sticks.
// set expCoef to 0.4 to get a more-or-less linear curve.
constexpr StickCurve make_curve(float maxSpeed, float expCoef = 0.4f) {
  StickCurve retval = {};
  float step = 1.0f / (retval.size() - 1);
  for (unsigned int i = 0; i < retval.size(); i++) {
    retval[i] = exp_entry(expCoef, i * step, maxSpeed);
  }
  return retval;
}


// This is not very interesting, just c++ nonsense to make a compile-time array.
template <int STICK_A_MODES, int STICK_B_MODES>
struct ModeMap {
  const std::array<StickMode, STICK_A_MODES + STICK_B_MODES> modes;
  static constexpr int aModesCount = STICK_A_MODES, bModesCount = STICK_B_MODES;

  constexpr ModeMap(std::array<StickMode, STICK_A_MODES> const& aModes, std::array<StickMode, STICK_B_MODES> const& bModes) : modes(detail::concatArray(aModes, bModes)) {}

  constexpr int count(int stick) const {
    if (stick == 0) return STICK_A_MODES;
    return STICK_B_MODES;
  }

  constexpr StickMode const& getMode(int stick, int modeIndex) const {
    if (stick == 0) {
      return modes[modeIndex];
    }
    else {
      return modes[STICK_A_MODES + modeIndex];
    }
  }
};

template <typename T1, typename T2>
constexpr auto declare_mode_map(T1 const& aModes, T2 const& bModes) {
    return ModeMap<aModes.size(), bModes.size()>(aModes, bModes);
}

