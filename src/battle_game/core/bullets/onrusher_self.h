#pragma once
#include "battle_game/core/bullet.h"

namespace battle_game::bullet {
class OnrusherSelf : public Bullet {
 public:
  OnrusherSelf(GameCore *core,
             uint32_t id,
             uint32_t unit_id,
             uint32_t player_id,
             glm::vec2 position,
             float rotation,
             float damage_scale,
             glm::vec2 velocity);
  ~OnrusherSelf() override;
  void Render() override;
  void Update() override;

 private:
  glm::vec2 velocity_{};
  uint32_t life_count_down_{6};
};
}  // namespace battle_game::bullet
