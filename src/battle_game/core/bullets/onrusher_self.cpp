#include "battle_game/core/bullets/onrusher_self.h"

#include "battle_game/core/game_core.h"
#include "battle_game/core/particles/particles.h"

namespace {
uint32_t onrusher_self_model_index = 0xffffffffu;
} // namespace

namespace battle_game::bullet {
OnrusherSelf::OnrusherSelf(GameCore *core,
                       uint32_t id,
                       uint32_t unit_id,
                       uint32_t player_id,
                       glm::vec2 position,
                       float rotation,
                       float damage_scale,
                       glm::vec2 velocity)
    : Bullet(core, id, unit_id, player_id, position, rotation, damage_scale),
      velocity_(velocity) {
  if (!~onrusher_self_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /* Onrusher Body */
      onrusher_self_model_index = mgr->RegisterModel(
        {
            // Define the vertices of the new isosceles triangle
            // The sharp point at the top
            { {0.0f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
            // Left base corner
            { {-0.3f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
            // Right base corner
            { {0.3f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
            // Back edges, used for differentiating front and back
            { {0.0f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
            { {-0.3f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
            { {0.3f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} }
        },
        {
            0, 1, 2, 0, 2, 3, 0, 3, 4, 3, 5, 4
        });
    }
  }
}

void OnrusherSelf::Render() {
  // SetTransformation(position_, rotation_, glm::vec2{0.1f});
  SetTransformation(position_, rotation_);
  SetTexture(0);
  SetColor(game_core_->GetPlayerColor(player_id_));
  // SetTexture(BATTLE_GAME_ASSETS_DIR "textures/particle3.png");
  DrawModel(onrusher_self_model_index);
}

void OnrusherSelf::Update() {
  position_ += velocity_ * kSecondPerTick;
  bool should_die = false;
  if (game_core_->IsBlockedByObstacles(position_)) {
    should_die = true;
  }
  if (life_count_down_ == 0) {
    should_die = true;
  } else {
    life_count_down_--;
  }

  auto &units = game_core_->GetUnits();
  for (auto &unit : units) {
    if (unit.first == unit_id_) {
      continue;
    }
    if (unit.second->IsHit(position_) && hit_units_.find(unit.first) == hit_units_.end()) {
      game_core_->PushEventDealDamage(unit.first, id_, damage_scale_ * 12.0f);
      hit_units_.insert(unit.first);
      // should_die = true;
    }
  }

  if (should_die) {
    game_core_->PushEventRemoveBullet(id_);
  }
}

OnrusherSelf::~OnrusherSelf() {
//   for (int i = 0; i < 5; i++) {
//     game_core_->PushEventGenerateParticle<particle::Smoke>(
//         position_, rotation_, game_core_->RandomInCircle() * 2.0f, 0.2f,
//         glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, 3.0f);
//   }
}
}  // namespace battle_game::bullet
