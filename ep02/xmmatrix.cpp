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

	// XMMATRIX ʹ���ĸ� XMVECTOR ʵ�֣��Դ�ʵ�ֿ�������
	// ͨ��ָ��4�� XMVECTOR ���� XMMATRIX �ĳ�ʼ��(����ȥ���Զ�����)
	// �洢���ݳ�Ա��ʹ�� XMFLOAT4x4
	XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 4.0f, 0.0f,
		1.0f, 2.0f, 3.0f, 1.0f);

	// ���ص�λ����
	XMMATRIX B = XMMatrixIdentity();

	XMMATRIX C = A * B;

	// ���о���ת��
	XMMATRIX D = XMMatrixTranspose(A);

	// ���� ( det A, det A, det A, det A )����4x1������ʽ����
	XMVECTOR det = XMMatrixDeterminant(A);
	// ���� A �������
	XMMATRIX E = XMMatrixInverse(&det, A);

	XMMATRIX F = A * E;

	cout << "A = " << endl << A << endl;
	cout << "B = " << endl << B << endl;
	cout << "C = A*B = " << endl << C << endl;
	cout << "D = transpose(A) = " << endl << D << endl;
	cout << "det = determinant(A) = " << det << endl << endl;
	cout << "E = inverse(A) = " << endl << E << endl;
	cout << "F = A*E = " << endl << F << endl;

	/// �����µľ�����ϱ任��¼�ڴ�

	/// <summary>
	/// �ṩ���������ϵ�����ϵ��
	/// </summary>
	/// <returns>����һ�����ž��󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixScaling(
		float ScaleX, float ScaleY, float ScaleZ
	);*/

	/// <summary>
	/// �� XMVECTOR �ķ�ʽ�ṩ���������ϵ�����ϵ��
	/// </summary>
	/// <returns>�� XMVECTOR �ķ�ʽ����һ�����ž��󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixScalingFromVector(
		FXMVECTOR Scale
	);*/

	/// <summary>
	/// ��˳ʱ�뷽�򰴻��� Angle �� x �������ת
	/// </summary>
	/// <returns>����һ���� x �����ת���󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationX(
		float Angle
	);*/

	/// <summary>
	/// ��˳ʱ�뷽�򰴻��� Angle �� y �������ת
	/// </summary>
	/// <returns>����һ���� y �����ת���󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationY(
		float Angle
	);*/

	/// <summary>
	/// ��˳ʱ�뷽�򰴻��� Angle �� z �������ת
	/// </summary>
	/// <returns>����һ���� z �����ת���󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationZ(
		float Angle
	);*/

	/// <summary>
	/// �� Axiz ��������򿴣���˳ʱ�뷽�򰴻��� Angle ������ת
	/// </summary>
	/// <returns>����һ�������������ת���󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixRotationAxiz(
		FXMVECTOR Axiz, float Angle
	);*/

	/// <summary>
	/// �ṩ���������ϵ�ƽ��ϵ��
	/// </summary>
	/// <returns>����һ��ƽ�ƾ��󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixTranslation(
		float OffsetX, float OffsetY, float OffsetZ
	);*/

	/// <summary>
	/// �� XMVECTOR �ķ�ʽ�ṩ���������ϵ�ƽ��ϵ��
	/// </summary>
	/// <returns>�� XMVECTOR �ķ�ʽ����һ��ƽ�ƾ��󲢷���</returns>
	/*XMMATRIX XM_CALLCONV XMMatrixTranslationFromVector(
		FXMVECTOR Offset
	);*/

	/// <summary>
	/// ��������v�;���M�ĳ˻�����Ե�仯�� v ���������Ϊ1��������ʽ����
	/// </summary>
	/// <returns>��Ե�任���� v*M</returns>
	/*XMVECTOR XM_CALLCONV XMVector3TransformCoord(
		FXMVECTOR V, CXMMATRIX M
	);*/

	/// <summary>
	/// ��������v�;���M�ĳ˻�����������仯�� v ���������Ϊ0��������ʽ����
	/// </summary>
	/// <returns>��������任���� v*M</returns>
	/*XMVECTOR XM_CALLCONV XMVector3TransformNormal(
		FXMVECTOR V, CXMMATRIX M
	);*/

	return 0;
}