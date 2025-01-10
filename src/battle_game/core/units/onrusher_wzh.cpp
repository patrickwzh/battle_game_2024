#include "onrusher_wzh.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t onrusher_model_index = 0xffffffffu;
uint32_t Onrusher_turret_model_index = 0xffffffffu;
}  // namespace

Onrusher::Onrusher(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~onrusher_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /* Onrusher Body */
      onrusher_model_index = mgr->RegisterModel(
          {
              {{-0.8f, 0.8f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-0.8f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{0.8f, 0.8f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{0.8f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              // distinguish front and back
              {{0.6f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-0.6f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
          },
          {0, 1, 2, 1, 2, 3, 0, 2, 5, 2, 4, 5});
    }

    {
      /* Onrusher Turret */
      std::vector<ObjectVertex> turret_vertices;
      std::vector<uint32_t> turret_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        turret_vertices.push_back({{sin_theta * 0.5f, cos_theta * 0.5f},
                                   {0.0f, 0.0f},
                                   {0.7f, 0.7f, 0.7f, 1.0f}});
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision);
      }
      turret_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_indices.push_back(precision + 1 + 0);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 3);
      Onrusher_turret_model_index =
          mgr->RegisterModel(turret_vertices, turret_indices);
    }
  }
}

void Onrusher::Render() {
  battle_game::SetTransformation(position_, turret_rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(onrusher_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(Onrusher_turret_model_index);
}

void Onrusher::Update() {
  if (!onrush_) {
    OnrusherMove(3.0f, glm::radians(180.0f));
  } else {
    Onrush(3.0f);
  }
  UpdateOnrush();
  TurretRotate();
  Fire();
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
  }
}

float Onrusher::GetSpeedScale() const {
  if (onrush_) {
    return 10.0f;
  } else {
    return 1.0f;
  }
}

void Onrusher::UpdateOnrush() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    if (!onrush_ && input_data.key_down[GLFW_KEY_SPACE] && spirit_ >= 1.0f) {
      onrush_ = true;
      onrush_cound_down_ = kTickPerSecond / 10;
    }
    if (onrush_) {
      spirit_ -= 1.0f / 6;
      if (onrush_count_down_ == 0) {
        onrush_ = false;
      } else {
        onrush_count_down_--;
      }
    }
    if (!onrush_ && spirit_ <= 3.0f) {
      spirit_ = std::min(spirit_ + 1.0f / 90, 3.0f);
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

void Onrusher::Fire(float move_speed) {
  if (onrush_) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
        auto velocity = Rotate(glm::vec2{0.0f, move_speed * 10.0f}, turret_rotation_);
        GenerateBullet<bullet::OnrusherSelf>(
            position_, turret_rotation_, GetDamageScale(), velocity);
    }
  }
}

bool Onrusher::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.8f && position.x < 0.8f && position.y > -1.0f &&
         position.y < 1.0f && position.x + position.y < 1.6f &&
         position.y - position.x < 1.6f;
}

const char *Onrusher::UnitName() const {
  return "Onrusher";
}

const char *Onrusher::Author() const {
  return "wzh";
}
}  // namespace battle_game::unit
