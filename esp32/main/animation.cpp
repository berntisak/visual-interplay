#include "animation.h"

namespace vi_anim {

  void Character::set_position(float v, uint8_t b) {

    position = {v, b};

  }

  void Character::set_size(float v, uint8_t b) {

    size = {v, b};

  }

  Character::Character() {

  }

}
