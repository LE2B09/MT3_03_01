#include <Novice.h>
#include <imgui.h>
#include "MathFunction.h"
#include "Vector4.h"

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

// Matrix4x4とVector4の乗算
Vector4 Multiply(const Vector4& v, const Matrix4x4& m) {
	Vector4 result;
	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2];
	result.w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3];
	return result;
}

// 3D座標を2Dスクリーン座標に変換する関数
Vector3 ProjectToScreen(const Vector3& point, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	Vector4 clipSpacePoint = Multiply({ point.x, point.y, point.z, 1.0f }, viewProjectionMatrix);
	Vector4 ndcSpacePoint = { clipSpacePoint.x / clipSpacePoint.w, clipSpacePoint.y / clipSpacePoint.w, clipSpacePoint.z / clipSpacePoint.w, 1.0f };
	Vector4 screenSpacePoint = Multiply(ndcSpacePoint, viewportMatrix);
	return { screenSpacePoint.x, screenSpacePoint.y, screenSpacePoint.z };
}

const char kWindowTitle[] = "提出用課題";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int prevMouseX = 0;
	int prevMouseY = 0;
	bool isDragging = false;

	MathFunction mathFunc;

	Segment segment{ .origin{-0.7f, 0.3f, 0.0f}, .diff{2.0f, -0.5f, 0.0f} };

	Vector3 translate{};
	Vector3 rotate{};

	Vector3 translates[3] = {
		{0.2f, 1.0f, 0.0f},		// 肩の位置
		{0.4f, 0.0f, 0.0f},		// 肘の位置
		{0.3f, 0.0f, 0.0f},		// 手の位置
	};
	Vector3 rotates[3] = {
		{0.0f, 0.0f, -6.8f},	// 肩の回転
		{0.0f, 0.0f, -1.4f},	// 肘の回転
		{0.0f, 0.0f, 0.0f},		// 手の回転
	};
	Vector3 scales[3] = {
		{1.0f, 1.0f, 1.0f},		// 肩のスケール
		{1.0f, 1.0f, 1.0f},		// 肘のスケール
		{1.0f, 1.0f, 1.0f},		// 手のスケール
	};
	Vector3 cameraTranslate = { 0.0f, 1.9f, -6.49f };
	Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// マウス入力を取得
		POINT mousePosition;
		GetCursorPos(&mousePosition);

		// マウスドラッグによる回転制御
		if (Novice::IsPressMouse(1)) {
			if (!isDragging) {
				isDragging = true;
				prevMouseX = mousePosition.x;
				prevMouseY = mousePosition.y;
			}
			else {
				int deltaX = mousePosition.x - prevMouseX;
				int deltaY = mousePosition.y - prevMouseY;
				rotate.y += deltaX * 0.01f; // 水平方向の回転
				rotate.x += deltaY * 0.01f; // 垂直方向の回転
				prevMouseX = mousePosition.x;
				prevMouseY = mousePosition.y;
			}
		}
		else {
			isDragging = false;
		}

		// マウスホイールで前後移動
		int wheel = Novice::GetWheel();
		if (wheel != 0) {
			cameraTranslate.z += wheel * 0.01f; // ホイールの回転方向に応じて前後移動
		}

		ImGui::Begin("Window");
		ImGui::DragFloat3("Shoulder Translate", &translates[0].x, 0.01f);
		ImGui::DragFloat3("Shoulder Rotate", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("Shoulder Scale", &scales[0].x, 0.01f);
		ImGui::DragFloat3("Elbow Translate", &translates[1].x, 0.01f);
		ImGui::DragFloat3("Elbow Rotate", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("Elbow Scale", &scales[1].x, 0.01f);
		ImGui::DragFloat3("Hand Translate", &translates[2].x, 0.01f);
		ImGui::DragFloat3("Hand Rotate", &rotates[2].x, 0.01f);
		ImGui::DragFloat3("Hand Scale", &scales[2].x, 0.01f);
		ImGui::End();

		// 各種行列の計算
		Matrix4x4 worldMatrix = mathFunc.MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = mathFunc.MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewWorldMatrix = mathFunc.Inverse(worldMatrix);
		Matrix4x4 viewCameraMatrix = mathFunc.Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = mathFunc.MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = mathFunc.Multiply(viewWorldMatrix, mathFunc.Multiply(viewCameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = mathFunc.MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		Matrix4x4 shoulderMatrix = mathFunc.MakeAffineMatrix(scales[0], rotates[0], translates[0]);
		Matrix4x4 elbowMatrix = mathFunc.MakeAffineMatrix(scales[1], rotates[1], translates[1]);
		Matrix4x4 handMatrix = mathFunc.MakeAffineMatrix(scales[2], rotates[2], translates[2]);

		Matrix4x4 elbowWorldMatrix = mathFunc.Multiply(shoulderMatrix, elbowMatrix);
		Matrix4x4 handWorldMatrix = mathFunc.Multiply(elbowWorldMatrix, handMatrix);

		Vector3 shoulderPos = { shoulderMatrix.m[3][0], shoulderMatrix.m[3][1], shoulderMatrix.m[3][2] };
		Vector3 elbowPos = { elbowWorldMatrix.m[3][0], elbowWorldMatrix.m[3][1], elbowWorldMatrix.m[3][2] };
		Vector3 handPos = { handWorldMatrix.m[3][0], handWorldMatrix.m[3][1], handWorldMatrix.m[3][2] };

		// 描画処理
		mathFunc.DrawGrid(viewProjectionMatrix, viewportMatrix);

		Vector3 shoulderPosScreen = ProjectToScreen(shoulderPos, viewProjectionMatrix, viewportMatrix);
		Vector3 elbowPosScreen = ProjectToScreen(elbowPos, viewProjectionMatrix, viewportMatrix);
		Vector3 handPosScreen = ProjectToScreen(handPos, viewProjectionMatrix, viewportMatrix);

		mathFunc.DrawSphere(Sphere{ shoulderPos, 0.1f }, viewProjectionMatrix, viewportMatrix, 0xFF0000FF); // 赤
		mathFunc.DrawSphere(Sphere{ elbowPos, 0.1f }, viewProjectionMatrix, viewportMatrix, 0x00FF00FF); // 緑
		mathFunc.DrawSphere(Sphere{ handPos, 0.1f }, viewProjectionMatrix, viewportMatrix, 0x0000FFFF); // 青

		Novice::DrawLine(int(shoulderPosScreen.x), int(shoulderPosScreen.y), int(elbowPosScreen.x), int(elbowPosScreen.y), 0xFFFFFFFF);
		Novice::DrawLine(int(elbowPosScreen.x), int(elbowPosScreen.y), int(handPosScreen.x), int(handPosScreen.y), 0xFFFFFFFF);

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
