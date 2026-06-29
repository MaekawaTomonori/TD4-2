#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"

class Camera;

namespace GameObjectComponent
{
	class PlayerInputComponent;
	class PhysicsComponent;

	class PlayerMoveComponent : public IActionComponent, public JsonEditableBase
	{
	public:
		// カメラのポインタを受け取るコンストラクタ
		PlayerMoveComponent(Camera* camera);

		void Update(GameObject* owner) override;

	private:
		// 物理挙動コンポーネントのポインタを保持
		PhysicsComponent* physics_ = nullptr;
		// 入力コンポーネントのポインタを保持
		PlayerInputComponent* input_ = nullptr;
		// カメラのポインタを保持
		Camera* camera_ = nullptr;

		// 移動速度
		float moveSpeed_ = 10.0f;
	};
} // namespace GameObjectComponent