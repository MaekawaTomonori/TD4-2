#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"

class Camera;

namespace GameObjectComponent
{

	class BombMoveComponent : public IActionComponent
	, public JsonEditableBase
	{
	public:
		// カメラのポインタを受け取るコンストラクタ
		BombMoveComponent(Camera* camera);

		// 更新処理
		void Update(GameObject* owner) override;

	private:
		// カメラのポインタを保持
		Camera* camera_ = nullptr;

		// 移動速度
		float moveSpeed_ = 10.0f;

	};

} // namespace GameObjectComponent