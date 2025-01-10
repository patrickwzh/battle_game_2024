#include "onrusher_wzh.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t onrusher_model_index = 0xffffffffu;
uint32_t Onrusher_turret_model_index = 0xffffffffu;
uint32_t spirit_bar_model_index = 0xffffffffu;
}  // namespace

Onrusher::Onrusher(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~onrusher_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      onrusher_model_index = mgr->RegisterModel(
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
  if (!~spirit_bar_model_index) {
    auto mgr = AssetsManager::GetInstance();
    spirit_bar_model_index = mgr->RegisterModel(
        {{{-0.5f, 0.08f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
         {{-0.5f, -0.08f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
         {{0.5f, 0.08f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
         {{0.5f, -0.08f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}},
        {0, 1, 2, 1, 2, 3});
  }
}

void Onrusher::Render() {
  battle_game::SetTransformation(position_, turret_rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(onrusher_model_index);
  
  // render spirit bar
  battle_game::SetTransformation(position_ + glm::vec2{0.0f, 0.6f}, 0.0f, {2.4f, 1.0f});
  battle_game::SetColor({0.0f, 0.0f, 0.0f, 1.0f});
  battle_game::SetTexture(0);
  battle_game::DrawModel(spirit_bar_model_index);
  float norm_spirit = spirit_ / 3.0f;
  glm::vec2 shift = {2.4f * (1 - norm_spirit) / 2, 0.0f};
  battle_game::SetTransformation(position_ + glm::vec2{0.0f, 0.6f} - shift, 0.0f, {2.4f * norm_spirit, 1.0f});
  battle_game::SetColor({1.0f, 0.5f, 0.0f, 1.0f});
  battle_game::DrawModel(spirit_bar_model_index);
  if (std::fabs(norm_spirit - fadeout_spirit_) >= 0.01f) {
    fadeout_spirit_ = norm_spirit + (fadeout_spirit_ - norm_spirit) * 0.93;
    shift = {2.4f * (norm_spirit + fadeout_spirit_ - 1) / 2, 0.0f};
    battle_game::SetTransformation(position_ + glm::vec2{0.0f, 0.6f} + shift, 0.0f, {2.4f * (norm_spirit - fadeout_spirit_), 1.0f});
    battle_game::SetColor({1.0f, 0.5f, 0.0f, 0.5f});
    battle_game::DrawModel(spirit_bar_model_index);
  } else {
    fadeout_spirit_ = norm_spirit;
  }
}

void Onrusher::Update() {
  if (!onrush_) {
    OnrusherMove(speed_, glm::radians(180.0f));
  } else {
    Onrush(speed_);
  }
  UpdateOnrush();
  UpdateParams();
  TurretRotate();
}

void Onrusher::UpdateParams() {
  speed_ = 4.0f - GetHealthScale();
  damage_scale_ = 1.0f + 0.5f * GetHealthScale();
}

void Onrusher::OnrusherMove(float move_speed, float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f};
    if (input_data.key_down[GLFW_KEY_W]) {
      offset.y += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_S]) {
      offset.y -= 1.0f;
    }
    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, turret_rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};
    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
    }
    float turret_rotation_offset = 0.0f;
    if (input_data.key_down[GLFW_KEY_A]) {
      turret_rotation_offset += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      turret_rotation_offset -= 1.0f;
    }
    turret_rotation_offset *= kSecondPerTick * rotate_angular_speed * GetSpeedScale();
    game_core_->PushEventRotateUnit(id_, turret_rotation_ + turret_rotation_offset);
  }
}

void Onrusher::Onrush(float move_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f};
    offset.y = 1.0f;
    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, turret_rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};
    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
    }
    for (auto &unit : game_core_->GetUnits()) {
      if (unit.first == id_) {
        continue;
      }
      if (unit.second->GetPlayerId() == player_id_) {
        continue;
      }
      if (unit.second->IsHit(position_) && hit_units_.find(unit.first) == hit_units_.end()) {
        game_core_->PushEventDealDamage(unit.first, id_, 20.0f);
        hit_units_.insert(unit.first);
      }
    }
  }
}

float Onrusher::GetSpeedScale() const {
  if (onrush_) {
    return 13.0f;
  } else {
    return 1.0f;
  }
}

void Onrusher::UpdateOnrush() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    if (!onrush_ && input_data.key_down[GLFW_KEY_SPACE] && spirit_ >= 1.0f && stop_ticks_ >= 10) {
      onrush_ = true;
      stop_ticks_ = 0;
      onrush_count_down_ = 3;
      hit_units_.clear();
    }
    if (onrush_) {
      spirit_ -= 1.0f / 3;
      if (onrush_count_down_ == 0) {
        onrush_ = false;
        stop_ticks_ = 0;
      } else {
        onrush_count_down_--;
      }
    }
    if (!onrush_ && spirit_ <= 3.0f) {
      spirit_ = std::min(spirit_ + 1.0f / 100, 3.0f);
    }
    if (!onrush_) {
      stop_ticks_++;
    }
  }
}

void Onrusher::TurretRotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    if (glm::length(diff) < 1e-4) {
      turret_rotation_ = turret_rotation_;
    } else {
      turret_rotation_ = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
  }
}

bool Onrusher::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.y < 0.5f + 2.0f / 0.6f * position.x &&
         position.y < 0.5f - 2.0f / 0.6f * position.x &&
         position.y > -0.5f;
}

const char *Onrusher::UnitName() const {
  return "Onrusher";
}

const char *Onrusher::Author() const {
  return "wzh";
}
}  // namespace battle_game::unit
