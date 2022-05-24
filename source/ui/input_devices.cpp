//
// Created by Caio W on 16/2/22.
//

#include "input_devices.hpp"

cr::keyboard::keyboard() : _keys() {}

cr::keyboard::state
cr::keyboard::state_of(cr::keyboard::key_code key) const noexcept {
  return _keys[static_cast<size_t>(key)].press_state;
}

int cr::keyboard::mods_of(cr::keyboard::key_code key) const noexcept {
  return _keys[static_cast<size_t>(key)].mods;
}

std::array<cr::keyboard::key_state,
           static_cast<size_t>(cr::keyboard::key_code::max_key)> &
cr::keyboard::keys() {
  return _keys;
}

bool cr::keyboard::is_key_down(cr::keyboard::key_code key) const noexcept {
  return _keys[static_cast<int>(key)].press_state == state::held ||
         _keys[static_cast<int>(key)].press_state == state::repeat ||
         _keys[static_cast<int>(key)].press_state == state::pressed;
}

bool cr::keyboard::is_key_pressed(cr::keyboard::key_code key) const noexcept {
  if (static_cast<int>(key) >= _keys.size())
    return false;

  return _keys[static_cast<int>(key)].press_state == state::pressed;
}

bool cr::keyboard::is_key_up(cr::keyboard::key_code key) const noexcept {
  return _keys[static_cast<int>(key)].press_state == state::released;
}

bool cr::keyboard::is_key_held(cr::keyboard::key_code key) const noexcept {
  return _keys[static_cast<int>(key)].press_state == state::held ||
         _keys[static_cast<int>(key)].press_state == state::repeat;
}

cr::mouse::mouse() {
  for (auto &button : _buttons)
    button = button_state::up;
}

void cr::mouse::set_position(const glm::ivec2 &pos) { _position = pos; }

std::array<cr::mouse::button_state, 3> &cr::mouse::buttons() {
  return _buttons;
}

cr::mouse::button_state
cr::mouse::state_of(cr::mouse::button target) const noexcept {
  return _buttons[static_cast<int>(target)];
}

glm::ivec2 cr::mouse::position() const noexcept { return _position; }

void cr::mouse::hide() { _hidden = true; }

void cr::mouse::show() { _hidden = false; }

bool cr::mouse::hidden() const noexcept { return _hidden; }
