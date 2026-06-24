#include "TestScene.h"
#include <numbers>
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/graphics/3d/Object3dCommon.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "scene/manager/SceneManager.h"
#include "manager/editor/GameObjectEditor.h"
#include "externals/imgui/imgui.h"
#include "engine/gameobject/component/collision/CollisionManager.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "application/collision/CollisionLayer.h"
#include "base/Logger.h"
#include "application/gameobject/component/action/StatusComponent.h"
#include "application/gameobject/component/action/PhysicsComponent.h"


using namespace GameObjectComponent;

void TestScene::Initialize()
{
	// カメラの設定
	sceneManager_->GetCameraManager()->GetActiveCamera()->SetTranslate({ 0.0f, 10.0f, 30.0f });
	sceneManager_->GetCameraManager()->GetActiveCamera()->SetRotate({ 0.1f, 0.0f, 0.0f });

	// ライトの調整
	DirectionalLight dirLight = sceneManager_->GetLightManager()->GetDirectionalLight();
	dirLight.direction = kLightDirection;
	dirLight.intensity = kLightIntensity;
	sceneManager_->GetLightManager()->SetDirectionalLight(dirLight);

	// デフォルトライトマネージャーの設定（Object3d描画用）
	sceneManager_->GetObject3dCommon()->SetDefaultLightManager(sceneManager_->GetLightManager());

	// デバッグカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(sceneManager_->GetCameraManager()->GetActiveCamera());
	debugCamera_->Start({ 0.0f, 10.0f, -30.0f }, { 0.2f, 0.0f, 0.0f });

	// ゲームオブジェクトマネージャーの初期化
	GameObjectManager::GetInstance()->Initialize();

	// コリジョンマネージャーの初期化
	CollisionManager::GetInstance()->Initialize();

	// GameObjectEditorの初期化
	GameObjectEditor::GetInstance()->Initialize();

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Collision Layer Test", [this]() { this->DrawImGui(); }, DebugUIArea::Console);
#endif

	// 1. テスト用キューブオブジェクトの作成
	cubeObject_ = std::make_unique<GameObject>("TestCube");
	cubeObject_->SetName("TestCube");
	cubeObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	cubeObject_->SetModel("cube");
	cubeObject_->SetPosition({ 0.0f, 2.0f, 0.0f });
	cubeObject_->SetScale({ 2.0f, 2.0f, 2.0f });
	
	// アクション・物理・ステータスコンポーネントの追加
	cubeObject_->AddComponent("Status", std::make_unique<StatusComponent>(cubeObject_.get()));
	cubeObject_->AddComponent("Physics", std::make_unique<PhysicsComponent>(cubeObject_.get()));

	// AABBコライダーの追加
	cubeObject_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(cubeObject_.get()));
	if (auto collider = cubeObject_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Player);
		collider->SetCollisionMask(CollisionLayer::Enemy | CollisionLayer::Stage | CollisionLayer::Terrain);
		
		// 衝突時の共通押し戻し・接地処理
		auto handleCubeCollision = [this](const CollisionInfo& info) {
			if (!info.otherCollider) return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Terrain)) return;
			if (!cubeObject_) return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = cubeObject_->GetPosition();
			pos += info.normal * info.depth;
			cubeObject_->SetPosition(pos);

			// 接地判定と速度リセット
			auto physics = cubeObject_->GetComponent<PhysicsComponent>();
			if (!physics) return;

			if (info.normal.y > 0.0f)
			{
				physics->SetGrounded(true);
				Vector3 vel = physics->GetVelocity();
				if (vel.y < 0.0f)
				{
					vel.y = 0.0f;
					physics->SetVelocity(vel);
				}
			}
		};

		collider->SetOnEnter([this, handleCubeCollision](const CollisionInfo& info) {
			handleCubeCollision(info);

			// 相手がEnemyの場合にHPを減らす
			if (!info.otherCollider) return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Enemy)) return;

			auto status = cubeObject_->GetComponent<StatusComponent>();
			if (!status) return;

			int32_t prevHp = status->GetHp();
			if (status->ApplyDamage(10))
			{
				Logger::Log("TestCube Damaged! HP: " + std::to_string(prevHp) + " -> " + std::to_string(status->GetHp()) + "\n");
			}
		});
		collider->SetOnStay([handleCubeCollision](const CollisionInfo& info) {
			handleCubeCollision(info);
		});
		collider->SetOnExit([](const CollisionInfo& info) {
		});
	}
	GameObjectManager::GetInstance()->Register(cubeObject_.get());

	// 2. テスト用ターゲットオブジェクトの作成
	targetObject_ = std::make_unique<GameObject>("TestTarget");
	targetObject_->SetName("TestTarget");
	targetObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	targetObject_->SetModel("cube");
	targetObject_->SetPosition({ 5.0f, 2.0f, 0.0f });
	targetObject_->SetScale({ 2.0f, 2.0f, 2.0f });
	targetObject_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 分かりやすく赤色にする

	// アクション・物理・ステータスコンポーネントの追加
	targetObject_->AddComponent("Status", std::make_unique<StatusComponent>(targetObject_.get()));
	targetObject_->AddComponent("Physics", std::make_unique<PhysicsComponent>(targetObject_.get()));

	// AABBコライダーの追加
	targetObject_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(targetObject_.get()));
	if (auto collider = targetObject_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Enemy);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Terrain);
		
		auto handleTargetCollision = [this](const CollisionInfo& info) {
			if (!info.otherCollider) return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Terrain)) return;
			if (!targetObject_) return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = targetObject_->GetPosition();
			pos += info.normal * info.depth;
			targetObject_->SetPosition(pos);

			// 接地判定と速度リセット
			auto physics = targetObject_->GetComponent<PhysicsComponent>();
			if (!physics) return;

			if (info.normal.y > 0.0f)
			{
				physics->SetGrounded(true);
				Vector3 vel = physics->GetVelocity();
				if (vel.y < 0.0f)
				{
					vel.y = 0.0f;
					physics->SetVelocity(vel);
				}
			}
		};

		collider->SetOnEnter([handleTargetCollision](const CollisionInfo& info) {
			handleTargetCollision(info);
		});
		collider->SetOnStay([handleTargetCollision](const CollisionInfo& info) {
			handleTargetCollision(info);
		});
		collider->SetOnExit([](const CollisionInfo& info) {
		});
	}
	GameObjectManager::GetInstance()->Register(targetObject_.get());

	// 3. 地面キューブオブジェクトの作成
	groundObject_ = std::make_unique<GameObject>("GroundCube");
	groundObject_->SetName("GroundCube");
	groundObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	groundObject_->SetModel("cube");
	groundObject_->SetPosition({ 0.0f, -0.5f, 0.0f });
	groundObject_->SetScale({ 50.0f, 1.0f, 50.0f });
	if (auto* obj3d = groundObject_->GetObject3d())
	{
		obj3d->SetCastShadow(false);
	}

	// 地面のAABBコライダーの追加
	groundObject_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(groundObject_.get()));
	if (auto collider = groundObject_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Terrain);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Enemy);
	}
	GameObjectManager::GetInstance()->Register(groundObject_.get());

	StartState(SceneState::Playing);
}

void TestScene::Finalize()
{
	// 登録されたオブジェクトの登録解除とクリア
	GameObjectManager::GetInstance()->Finalize();
	CollisionManager::GetInstance()->Finalize();
	if (GameObjectEditor::HasInstance())
	{
		GameObjectEditor::GetInstance()->Finalize();
	}
#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
	cubeObject_.reset();
	groundObject_.reset();
	targetObject_.reset();
	debugCamera_.reset();
}

void TestScene::OnUpdatePlaying()
{
	if (debugCamera_)
	{
		debugCamera_->Update();
	}

	// コリジョンマネージャーの前フレーム位置更新
	CollisionManager::GetInstance()->UpdatePreviousPositions();

	// ゲームオブジェクトマネージャーの更新
	GameObjectManager::GetInstance()->Update();

	// 衝突判定の実行
	CollisionManager::GetInstance()->CheckCollisions();
}

void TestScene::Draw3D()
{
	// ゲームオブジェクトの3D描画
	GameObjectManager::GetInstance()->Draw3D(sceneManager_->GetCameraManager());
}

void TestScene::Draw2D()
{
	// ゲームオブジェクトの2D描画
	GameObjectManager::GetInstance()->Draw2D();
}

#ifdef USE_IMGUI
void TestScene::DrawImGui()
{
	if (cubeObject_ && targetObject_)
	{
		// 1. 位置の調整
		Vector3 pos = cubeObject_->GetPosition();
		ImGui::Text("Cube Position:");
		if (ImGui::SliderFloat("X", &pos.x, -10.0f, 10.0f))
		{
			cubeObject_->SetPosition(pos);
		}
		
		ImGui::Separator();
		
		// 2. レイヤーとマスクの調整
		if (auto collider = cubeObject_->GetComponent<AABBColliderComponent>())
		{
			ImGui::Text("Cube Collision Settings:");
			
			// レイヤー選択
			int currentLayerIdx = 0;
			ColliderLayer layer = collider->GetCollisionLayer();
			if (layer == CollisionLayer::Player) currentLayerIdx = 0;
			else if (layer == CollisionLayer::PlayerBullet) currentLayerIdx = 1;
			else if (layer == CollisionLayer::Enemy) currentLayerIdx = 2;
			else if (layer == CollisionLayer::None) currentLayerIdx = 3;
			
			const char* layerNames[] = { "Player", "PlayerBullet", "Enemy", "None" };
			if (ImGui::Combo("Layer", &currentLayerIdx, layerNames, IM_ARRAYSIZE(layerNames)))
			{
				if (currentLayerIdx == 0) collider->SetCollisionLayer(CollisionLayer::Player);
				else if (currentLayerIdx == 1) collider->SetCollisionLayer(CollisionLayer::PlayerBullet);
				else if (currentLayerIdx == 2) collider->SetCollisionLayer(CollisionLayer::Enemy);
				else if (currentLayerIdx == 3) collider->SetCollisionLayer(CollisionLayer::None);
			}
			
			// マスク（衝突対象）のトグル
			uint32_t mask = collider->GetCollisionMask();
			bool collideWithPlayer = (mask & CollisionLayer::Player) != 0;
			bool collideWithEnemy = (mask & CollisionLayer::Enemy) != 0;
			
			ImGui::Text("Collides With:");
			if (ImGui::Checkbox("Player (Layer)", &collideWithPlayer))
			{
				if (collideWithPlayer) mask |= CollisionLayer::Player;
				else mask &= ~CollisionLayer::Player;
				collider->SetCollisionMask(mask);
			}
			if (ImGui::Checkbox("Enemy (Layer)", &collideWithEnemy))
			{
				if (collideWithEnemy) mask |= CollisionLayer::Enemy;
				else mask &= ~CollisionLayer::Enemy;
				collider->SetCollisionMask(mask);
			}
		}
		
		ImGui::Separator();
		ImGui::Text("Target (Red Cube) Info:");
		ImGui::Text("Position: X=5.0, Y=2.0, Z=0.0");
		ImGui::Text("Layer: Enemy");
		ImGui::Text("Mask: Player (Only collides with Player)");
		
		// 判定のヒント表示
		ImGui::Separator();
		ImGui::TextWrapped("Tip: Move the Cube X position towards 5.0 to collide with the Red Cube. Change Cube's layer/mask above to see how filtering works. Check Output Log for collision events.");
	}
}
#endif

void TestScene::DrawShadow()
{
	// ゲームオブジェクトのシャドウ描画
	GameObjectManager::GetInstance()->DrawShadow(sceneManager_->GetCameraManager()->GetActiveCamera());
}

void TestScene::DrawGBuffer()
{
	// ゲームオブジェクトのGBuffer描画
	GameObjectManager::GetInstance()->DrawGBuffer(sceneManager_->GetCameraManager());
}
