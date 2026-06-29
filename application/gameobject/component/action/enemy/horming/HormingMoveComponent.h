#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"

namespace GameObjectComponent {

class HormingMoveComponent : public IActionComponent {

public:

	void Update(GameObject* owner) override;
};
}