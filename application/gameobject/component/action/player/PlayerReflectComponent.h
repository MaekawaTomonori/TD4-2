#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"

class ICollisionComponent;

namespace GameObjectComponent
{
	/// @brief プレイヤーの反射処理を制御するコンポーネント
	class PlayerReflectComponent : public IActionComponent
		, public JsonEditableBase
	{
	public:
		void Update(GameObject* owner) override;

	private:
		// 反射するときに使うコライダーコンポーネントのポインタを保持
		ICollisionComponent* collider_ = nullptr;
	};
} // namespace GameObjectComponent
