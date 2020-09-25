//***************************************************************************************
// BoxApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//
// Shows how to draw a box in Direct3D 12.
//
// Controls:
//   Hold the left mouse button down and move the mouse to rotate.
//   Hold the right mouse button down and move the mouse to zoom in and out.
//***************************************************************************************

#include "../Common/d3dApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

// 单个顶点结构体
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

// 着色器所需的常量
struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	BoxApp(const BoxApp& rhs) = delete;
	BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();	// 创建几何体
	void BuildPSO();

private:

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	/*ComPtr<ID3D12PipelineState> mPSO1 = nullptr;
	int mPSOTest = 0;*/

	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		BoxApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps();			// 创建描述符堆
	BuildConstantBuffers();			// 创建常量缓冲区并储存于描述符堆中
	BuildRootSignature();			// 使用描述符堆中的描述符表创建根签名(此处混乱，可能需要参考 ep7)
	BuildShadersAndInputLayout();	// 编译着色器并创建输入布局
	BuildBoxGeometry();				// 创建正方体几何体
	BuildPSO();						// 将上述操作集成到流水线

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	// 计算更新后的变换矩阵并拷贝至上传堆
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	mObjectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	// 重置命令列表并使用初始 mPSO 作为流水线进行绘制
	// 如有需要可以使用 mCmdList->SetPipelineState( mPSO2.Get()); 转变使用的流水线
	// 如下方被注释代码所示，性能开销较大
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));
	/*if (mPSOTest % 20 != 0)
	{
		mCommandList->SetPipelineState(mPSO1.Get());
	}
	mPSOTest++;*/

	// 描述要输出的视口与裁剪矩形
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 设置后续引用的描述符堆
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 将当前根签名布局设置为所需根签名的样式
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// 将顶点缓冲区绑定至输入槽
	mCommandList->IASetVertexBuffers(
		0,								// 起始输入槽(共有16个)
		1,								// 要绑定缓冲数量(若有 n 个缓冲区则使用 n 个输入槽)
		&mBoxGeo->VertexBufferView());	// 指向缓冲区第一个元素的指针
	// 将索引缓冲区绑定至输入装配器阶段
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 本例中只有一个 CBV 故直接使用 GetGPUDescriptorHandleForHeapStart()
	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,	// 每个实例要绘制的索引数量
		1,										// 实例个数
		0,										// 希望读取的起始索引
		0,										// 索引偏移量(另外一个物体相对于之前的物体)
		0
	);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// 使其对着色器可见
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	// 偏移到想绘制的第若干个物体
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	// 创建只有一个 CBV 的描述符表( slotRootParameter 长度为 1)
	CD3DX12_DESCRIPTOR_RANGE cbvTable;		// 描述符区域(描述符表) (这里区域和表的概念比较混乱)
	cbvTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	// 描述符表中存放的描述符类型
		1,									// 描述符表中的描述符数量
		0									// 要绑定的基准着色器寄存器(如 b0)
	);
	slotRootParameter[0].InitAsDescriptorTable(
		1,									// 描述符表中描述符的数量(应该可以作为数组传入)
		&cbvTable							// 指向描述符表的指针
	);

	// A root signature is an array of root parameters.
	// 描述根签名
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	// 创建只含一个寄存器槽位的根签名
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	// DX12 需要对描述布局进行序列化以匹配 ID3DBlob (即对 CD3DX12_ROOT_SIGNATURE_DESC 进行序列化)
	// 指定根签名版本为1.0，较新系统会默认1.1进行编译，但会不兼容较老版本系统
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void BoxApp::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	// 要与 shader 的输入匹配
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void BoxApp::BuildBoxGeometry()
{
	// 顶点列表
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	// 索引列表
	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	// 计算缓冲区所需大小
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	// 储存 CPU 副本并准备上传到 GPU 中的默认堆
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	// 记录单个几何体的顶点数量以及相关偏移量
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

void BoxApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	// 指定输入布局
	psoDesc.InputLayout =
	{ mInputLayout.data(), (UINT)mInputLayout.size() };			// 数组与其中元素的数量
// 指定根签名，一定要和以下着色器相兼容
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),	// 字节码指针
		mvsByteCode->GetBufferSize()								// 所占字节大小
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),	// 同上
		mpsByteCode->GetBufferSize()
	};

	psoDesc.RasterizerState =
		CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);						// 配置光栅器
	// 注释掉的两行将开启线框渲染模式并禁用背面剔除
	/*psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;*/

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);			// 用于实现混合
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;									// 无关数量对所有采样点采样
	psoDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;						// 指定图元拓扑类型
	psoDesc.NumRenderTargets = 1;									// 要渲染的目标数量
																	// 同 RTVFormats 数组的 size
	psoDesc.RTVFormats[0] = mBackBufferFormat;						// 指定渲染目标格式的数组
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	// 将所有的资源绑定至流水线，可以在初始化阶段生成驱动流水线的全部代码
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

	//ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	//// 指定输入布局
	//psoDesc.InputLayout =
	//{ mInputLayout.data(), (UINT)mInputLayout.size() };			// 数组与其中元素的数量
	//// 指定根签名，一定要和以下着色器相兼容
	//psoDesc.pRootSignature = mRootSignature.Get();
	//psoDesc.VS =
	//{
	//	reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),	// 字节码指针
	//	mvsByteCode->GetBufferSize()								// 所占字节大小
	//};
	//psoDesc.PS =
	//{
	//	reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),	// 同上
	//	mpsByteCode->GetBufferSize()
	//};

	//psoDesc.RasterizerState =
	//	CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);						// 配置光栅器
	//// 注释掉的两行将开启线框渲染模式并禁用背面剔除
	//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);			// 用于实现混合
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.SampleMask = UINT_MAX;									// 无关数量对所有采样点采样
	//psoDesc.PrimitiveTopologyType =
	//	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;						// 指定图元拓扑类型
	//psoDesc.NumRenderTargets = 1;									// 要渲染的目标数量
	//																// 同 RTVFormats 数组的 size
	//psoDesc.RTVFormats[0] = mBackBufferFormat;						// 指定渲染目标格式的数组
	//psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	//psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	//psoDesc.DSVFormat = mDepthStencilFormat;

	//// 将所有的资源绑定至流水线，可以在初始化阶段生成驱动流水线的全部代码
	//ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO1)));
}