#include "PlayerMoveComponent.h"
#include "PlayerInputComponent.h"
#include "../common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "base/Camera.h"

GameObjectComponent::PlayerMoveComponent::PlayerMoveComponent(Camera* camera)
	: camera_(camera)
{
	// メンバ変数をJSONエディタ/シリアライズ用に登録
	Register("moveSpeed", &moveSpeed_);
}

void GameObjectComponent::PlayerMoveComponent::Update(GameObject* owner)
{
	// 初回Update時に必要なコンポーネントのポインタを取得
	if (!input_)
	{
		input_ = owner->GetComponent<PlayerInputComponent>().get();
	}
	if (!physics_)
	{
		physics_ = owner->GetComponent<PhysicsComponent>().get();
	}

	// 入力コンポーネントと物理コンポーネントが取得できていない場合は処理を中断
	if (!input_ || !physics_)
	{
		return;
	}

	// 入力に基づいて移動方向を取得
	const Vector3& moveDirection = input_->GetMoveDirection();

	// カメラのY軸回転を取得
	float yaw = camera_->GetRotate().y;

	// カメラの基準方向を計算（Y軸回転のみを考慮）
	Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
	Vector3 right = {std::cos(yaw), 0.0f, -std::sin(yaw)};

	// 入力方向をカメラ基準に変換
	Vector3 transformedDirection = (moveDirection.x * right) + (moveDirection.z * forward);
	transformedDirection.NormalizeSelf(); // 正規化して方向ベクトルにする

	// 移動速度を適用
	transformedDirection *= moveSpeed_;

	// 物理挙動コンポーネントに速度を設定
	physics_->SetMovementVelocity(transformedDirection);
}