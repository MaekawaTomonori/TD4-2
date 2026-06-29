#include "ChargeMoveComponent.h"

#include "engine/gameobject/base/GameObject.h"

GameObjectComponent::ChargeMoveComponent::ChargeMoveComponent(GameObject* _player)
	: player_(_player)
{
}

void GameObjectComponent::ChargeMoveComponent::Update(GameObject* owner)
{
	// プレイヤーが存在しない場合は処理を中断
	if (!player_)
	{
		return;
	}

	// プレイヤーの位置を取得
	Vector3 playerPosition = player_->GetPosition();




}
