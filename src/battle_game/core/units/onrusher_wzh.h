#pragma once
#include "battle_game/core/unit.h"

namespace battle_game::unit {
class Onrusher : public Unit {
 public:
  Onrusher(GameCore *game_core, uint32_t id, uint32_t player_id);
  void Render() override;
  void Update() override;
  [[nodiscard]] bool IsHit(glm::vec2 position) const override;

 protected:
  void OnrusherMove(float move_speed, float rotate_angular_speed);
  void Onrush(float move_speed);
  float GetSpeedScale() const override;
  void UpdateOnrush();
  void UpdateParams();
  void TurretRotate();
  void Fire(float move_speed);
  [[nodiscard]] const char *UnitName() const override;
  [[nodiscard]] const char *Author() const override;

  float turret_rotation_{0.0f}, spirit_{1.5f}, speed_{3.0f}, damage_scale_{1.0f}, fadeout_spirit_{1.5f};
  bool onrush_{false};
  uint32_t onrush_count_down_{0};
  uint32_t mine_count_down_{0};
  uint32_t stop_ticks_{0};
  std::set<uint32_t> hit_units_;
};
}  // namespace battle_game::unit
