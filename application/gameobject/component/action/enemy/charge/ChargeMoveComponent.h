#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"

namespace GameObjectComponent
{

  /// <summary>
  /// チャージ移動コンポーネント
  /// </summary>
  class ChargeMoveComponent : public IActionComponent
  {
  public:

	  // コンストラクタ
	  ChargeMoveComponent(GameObject* _player);

	  // デストラクタ
	  ~ChargeMoveComponent() override = default;
	  
	  /// <summary>
	  /// 毎フレームの更新処理
	  /// </summary>
	  /// <param name="owner">所有者</param>
      void Update(GameObject* owner) override;

  private:

	  // プレイヤーのポインタ
	  GameObject* player_ = nullptr; 

	  // 移動速度
	  float moveSpeed_ = 5.0f;


  };


} // namespace GameObjectComponent