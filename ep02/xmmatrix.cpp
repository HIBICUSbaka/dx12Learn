#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// Overload the  "<<" operators so that we can use cout to 
// output XMVECTOR and XMMATRIX objects.
ostream& XM_CALLCONV operator << (ostream& os, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";
	return os;
}

ostream& XM_CALLCONV operator << (ostream& os, FXMMATRIX m)
{
	for (int i = 0; i < 4; ++i)
	{
		os << XMVectorGetX(m.r[i]) << "\t";
		os << XMVectorGetY(m.r[i]) << "\t";
		os << XMVectorGetZ(m.r[i]) << "\t";
		os << XMVectorGetW(m.r[i]);
		os << endl;
	}
	return os;
}

int main()
{
	// Check support for SSE2 (Pentium4, AMD K8, and above).
	if (!XMVerifyCPUSupport())
	{
		cout << "directx math not supported" << endl;
		return 0;
	}

	// XMMATRIX 使用四个 XMVECTOR 实现，以此实现快速运算
	// 通过指定4个 XMVECTOR 进行 XMMATRIX 的初始化(看上去是自动分配)
	// 存储数据成员则使用 XMFLOAT4x4
	XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 4.0f, 0.0f,
		1.0f, 2.0f, 3.0f, 1.0f);

	// 返回单位矩阵
	XMMATRIX B = XMMatrixIdentity();

	XMMATRIX C = A * B;

	// 进行矩阵转置
	XMMATRIX D = XMMatrixTranspose(A);

	// 返回 ( det A, det A, det A, det A )，及4x1的行列式矩阵
	XMVECTOR det = XMMatrixDeterminant(A);
	// 返回 A 的逆矩阵
	XMMATRIX E = XMMatrixInverse(&det, A);

	XMMATRIX F = A * E;

	cout << "A = " << endl << A << endl;
	cout << "B = " << endl << B << endl;
	cout << "C = A*B = " << endl << C << endl;
	cout << "D = transpose(A) = " << endl << D << endl;
	cout << "det = determinant(A) = " << det << endl << endl;
	cout << "E = inverse(A) = " << endl << E << endl;
	cout << "F = A*E = " << endl << F << endl;

	/// 第三章的矩阵组合变换记录于此

	/// <summary>
	/// 提供各方向轴上的缩放系数
	/// </summary>
	/// <returns>构建一个缩放矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixScaling(
		float ScaleX, float ScaleY, float ScaleZ
	);*/

	/// <summary>
	/// 以 XMVECTOR 的方式提供各方向轴上的缩放系数
	/// </summary>
	/// <returns>以 XMVECTOR 的方式构建一个缩放矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixScalingFromVector(
		FXMVECTOR Scale
	);*/

	/// <summary>
	/// 以顺时针方向按弧度 Angle 绕 x 轴进行旋转
	/// </summary>
	/// <returns>构建一个绕 x 轴的旋转矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationX(
		float Angle
	);*/

	/// <summary>
	/// 以顺时针方向按弧度 Angle 绕 y 轴进行旋转
	/// </summary>
	/// <returns>构建一个绕 y 轴的旋转矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationY(
		float Angle
	);*/

	/// <summary>
	/// 以顺时针方向按弧度 Angle 绕 z 轴进行旋转
	/// </summary>
	/// <returns>构建一个绕 z 轴的旋转矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationZ(
		float Angle
	);*/

	/// <summary>
	/// 从 Axiz 轴的正方向看，以顺时针方向按弧度 Angle 进行旋转
	/// </summary>
	/// <returns>构建一个绕任意轴的旋转矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationAxiz(
		FXMVECTOR Axiz, float Angle
	);*/

	/// <summary>
	/// 提供各方向轴上的平移系数
	/// </summary>
	/// <returns>构建一个平移矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixTranslation(
		float OffsetX, float OffsetY, float OffsetZ
	);*/

	/// <summary>
	/// 以 XMVECTOR 的方式提供各方向轴上的平移系数
	/// </summary>
	/// <returns>以 XMVECTOR 的方式构建一个平移矩阵并返回</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixTranslationFromVector(
		FXMVECTOR Offset
	);*/

	/// <summary>
	/// 计算向量v和矩阵M的乘积，针对点变化故 v 的齐次坐标为1，无需显式传参
	/// </summary>
	/// <returns>针对点变换返回 v*M</returns>
	/*XMVECTOR XM_CALLCONV XMVector3TransformCoord(
		FXMVECTOR V, CXMMATRIX M
	);*/

	/// <summary>
	/// 计算向量v和矩阵M的乘积，针对向量变化故 v 的齐次坐标为0，无需显式传参
	/// </summary>
	/// <returns>针对向量变换返回 v*M</returns>
	/*XMVECTOR XM_CALLCONV XMVector3TransformNormal(
		FXMVECTOR V, CXMMATRIX M
	);*/

	return 0;
}