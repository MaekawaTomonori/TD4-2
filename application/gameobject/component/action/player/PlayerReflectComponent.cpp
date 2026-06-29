#include "PlayerReflectComponent.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "application/collision/CollisionLayer.h"
#include "engine/gameobject/base/GameObject.h"

void GameObjectComponent::PlayerReflectComponent::Update(GameObject* owner)
{
	if (!collider_)
	{
		// 初回Update時にコライダーコンポーネントのポインタを取得
		collider_ = owner->GetComponent<SphereColliderComponent>().get();
	}

	// コライダーコンポーネントが取得できていない場合は処理を中断
	if (!collider_)
	{
		return;
	}

	// 入力があったらコライダーをアクティブにする
	if (owner->GetComponent<PlayerInputComponent>()->IsReflecting())
	{
		collider_->SetCollisionLayer(CollisionLayer::PlayerBullet);
	}
	else
	{
		collider_->SetCollisionLayer(CollisionLayer::None);
	}
}
