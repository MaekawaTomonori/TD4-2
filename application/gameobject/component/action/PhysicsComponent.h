#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

namespace GameObjectComponent
{
	/**
	 * @brief キャラクターの物理挙動（移動、重力、外力）を管理するコンポーネント
	 */
	class PhysicsComponent : public IActionComponent, public JsonEditableBase
	{
	public:
		/**
		 * @brief コンストラクタ
		 * @param owner このコンポーネントを所有するGameObject
		 */
		PhysicsComponent(::GameObject* owner);

		/**
		 * @brief デストラクタ
		 */
		~PhysicsComponent() override;

		/**
		 * @brief 毎フレームの更新処理
		 * @param owner このコンポーネントを所有するGameObject
		 */
		void Update(::GameObject* owner) override;

		/**
		 * @brief 外部からの衝撃を加える (ノックバック等)
		 * @param force 加える力ベクトル
		 */
		void AddForce(const Vector3& force);

	public: // ゲッター・セッター
		const Vector3& GetVelocity() const { return velocity_; }
		void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

		const Vector3& GetExternalForce() const { return externalForce_; }
		void SetExternalForce(const Vector3& force) { externalForce_ = force; }

		float GetMass() const { return mass_; }
		void SetMass(float mass) { mass_ = mass; }

		float GetFriction() const { return friction_; }
		void SetFriction(float friction) { friction_ = friction; }

		float GetForceDecay() const { return forceDecay_; }
		void SetForceDecay(float decay) { forceDecay_ = decay; }

		bool GetUseGravity() const { return useGravity_; }
		void SetUseGravity(bool use) { useGravity_ = use; }

		bool IsGrounded() const { return isGrounded_; }
		void SetGrounded(bool grounded) { isGrounded_ = grounded; }

		float GetGravity() const { return gravity_; }
		void SetGravity(float gravity) { gravity_ = gravity; }

	private:
		// 現在速度
		Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
		// 瞬間的に加わる外力（被弾時の衝撃など）
		Vector3 externalForce_ = { 0.0f, 0.0f, 0.0f };

		// 質量
		float mass_ = 1.0f;
		// 摩擦係数 (接地時の減衰率、1フレームあたり)
		float friction_ = 0.9f;
		// 外力の減衰率 (1フレームあたり)
		float forceDecay_ = 0.85f;

		// 重力を適用するか
		bool useGravity_ = true;
		// 接地しているか
		bool isGrounded_ = false;
		// 重力加速度 (m/s^2)
		float gravity_ = 9.8f;
	};
}
