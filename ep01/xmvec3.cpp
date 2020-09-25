
#include <windows.h> // for XMVerifyCPUSupport
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// Overload the  "<<" operators so that we can use cout to 
// output XMVECTOR objects.
ostream& XM_CALLCONV operator << (ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int main()
{
	XMVECTOR test = XMVectorZero();

	cout.setf(ios_base::boolalpha);

	// Check support for SSE2 (Pentium4, AMD K8, and above).
	// 可进行 XMVECTOR 的四位数并行运算
	if (!XMVerifyCPUSupport())
	{
		cout << "directx math not supported" << endl;
		return 0;
	}

	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	// Vector addition: XMVECTOR operator + 
	XMVECTOR a = u + v;

	// Vector subtraction: XMVECTOR operator - 
	XMVECTOR b = u - v;

	// Scalar multiplication: XMVECTOR operator * 
	XMVECTOR c = 10.0f * u;

	// ||u||
	XMVECTOR L = XMVector3Length(u);

	// d = u / ||u||
	XMVECTOR d = XMVector3Normalize(u);

	// s = u dot v
	XMVECTOR s = XMVector3Dot(u, v);

	// e = u x v
	XMVECTOR e = XMVector3Cross(u, v);

	// Find proj_n(w) and perp_n(w)
	XMVECTOR projW;     // 表示该向量在某个方向上的投影
	XMVECTOR perpW;     // 标识基于某方向投影的正交
	// 计算并传回 w 向量的正交化结果
	// projW 为 w 在 n 方向上的投影
	// perpW 为 w - n 且与 projW 正交的向量
	// 没有对计算结果进行归一化(标准化)
	XMVector3ComponentsFromNormal(&projW, &perpW, w, n);

	// Does projW + perpW == w?
	// DirectXMath 库提供判断浮点数是否相等的方法 XMVector3NearEqual()
	// 传入参数 FXMVECTOR FXMVECTOR Epsilon ， Epsilon 为可允许误差范围
	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;

	// The angle between projW and perpW should be 90 degrees.
	// 使用 XMVECTOR 可以并行计算的优越性进行高效率的计算，而非使用 XMFLOAT3 等形式
	// 计算完成后再将结果取出，能提升的效果未知
	XMVECTOR angleVec = XMVector3AngleBetweenVectors(projW, perpW);
	float angleRadians = XMVectorGetX(angleVec);
	float angleDegrees = XMConvertToDegrees(angleRadians);

	cout << "u                   = " << u << endl;
	cout << "v                   = " << v << endl;
	cout << "w                   = " << w << endl;
	cout << "n                   = " << n << endl;
	cout << "a = u + v           = " << a << endl;
	cout << "b = u - v           = " << b << endl;
	cout << "c = 10 * u          = " << c << endl;
	cout << "d = u / ||u||       = " << d << endl;
	cout << "e = u x v           = " << e << endl;
	cout << "L  = ||u||          = " << L << endl;
	cout << "s = u.v             = " << s << endl;
	cout << "projW               = " << projW << endl;
	cout << "perpW               = " << perpW << endl;
	cout << "projW + perpW == w  = " << equal << endl;
	cout << "projW + perpW != w  = " << notEqual << endl;
	cout << "angle               = " << angleDegrees << endl;

	/// <summary>
	/// 计算向量的分量式乘法
	/// 多用于计算光照
	/// (r,g,b) 分量* (0.5,0.75,0.25) = (0.5r,0.75g,0.25b)
	/// </summary>
	/// <returns>返回光照的分量式乘法计算结果</returns>
	/*XMVECTOR XM_CALLCONV XMColorModulate(
		FXMVECTOR C1, FXMVECTOR C2
	);*/

	return 0;
}
