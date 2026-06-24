#include "PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include "engine/gameobject/component/base/ComponentFactory.h"
#include <cmath>

// ファクトリに登録
REGISTER_COMPONENT(PhysicsComponent)

namespace GameObjectComponent
{
	PhysicsComponent::PhysicsComponent(::GameObject* owner)
	{
		// メンバ変数をJSONエディタ/シリアライズ用に登録
		Register("velocity", &velocity_);
		Register("externalForce", &externalForce_);
		Register("mass", &mass_);
		Register("friction", &friction_);
		Register("forceDecay", &forceDecay_);
		Register("useGravity", &useGravity_);
		Register("isGrounded", &isGrounded_);
		Register("gravity", &gravity_);
	}

	PhysicsComponent::~PhysicsComponent()
	{
	}

	void PhysicsComponent::Update(::GameObject* owner)
	{
		float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
		if (dt <= 0.0f)
		{
			return;
		}

		// 重力の適用
		if (useGravity_)
		{
			if (!isGrounded_)
			{
				velocity_.y -= gravity_ * dt;
			}
			else
			{
				// 接地中は下向きに極小の速度（吸着バイアス）を与えることで、
				// 毎フレーム確実にコライダーが地面と交差し、接地判定が安定するようにする
				velocity_.y = -0.1f;
			}
		}

		// 外力を速度に加算し、外力自体は減衰させる (秒単位 of 60fps)
		velocity_ += externalForce_ * dt;
		externalForce_ = externalForce_ * std::pow(forceDecay_, dt * 60.0f);

		// 摩擦の適用（接地している場合）
		if (isGrounded_)
		{
			float frictionFactor = std::pow(friction_, dt * 60.0f);
			velocity_.x *= frictionFactor;
			velocity_.z *= frictionFactor;
		}

		// 座標の更新
		Vector3 currentPos = owner->GetPosition();
		currentPos += velocity_ * dt;
		owner->SetPosition(currentPos);

		// 接地フラグをリセット（この後の衝突判定で接地していれば OnStay 等で true に再設定される）
		isGrounded_ = false;
	}

	void PhysicsComponent::AddForce(const Vector3& force)
	{
		if (mass_ > 0.0f)
		{
			externalForce_ += force / mass_;
		}
	}
}
