#include "ShapeWithLightApp.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")


const int gNumFrameResources = 3;


using namespace ATL;
using namespace DirectX;

struct RenderItem
{
	RenderItem() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources;

	UINT ObjCBIndex = -1;
	Material *Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT	IndexCount = 0;
	UINT	StartIndexLocation = 0;
	int		BaseVerTexLocation = 0;
};


ShapeWithLightApp::ShapeWithLightApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

ShapeWithLightApp::~ShapeWithLightApp()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool ShapeWithLightApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc, nullptr));
	
	

	BuildRootSignature();
	BuildShadersAndInputLayout();
	//BuildConstantBufferViews();
	BuildSkullGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOT();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
	return true;
}

void ShapeWithLightApp::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void ShapeWithLightApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void ShapeWithLightApp::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());
	//ThrowIfFailed(mCommandList->Reset(cmdListAlloc, mPSOs["opaque"]));
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc, mOpaquePSO.Get()));


	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	
	/*ID3D12DescriptorHeap *descriptorHeaps[] = { mCbvHeap };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);*/
	
	//mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->SetGraphicsRootSignature(pTest);


	/*int passCbvIndex = mPassCbvOffset + mCurrFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);
	*/
	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList, mOpaqueRitems);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	mCommandList->Close();
	ID3D12CommandList* cmdLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	mCurrFrameResource->Fence = ++mCurrentFence;
	mCommandQueue->Signal(mFence, mCurrentFence);
}

void ShapeWithLightApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void ShapeWithLightApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapeWithLightApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void ShapeWithLightApp::OnKeyboardInput(const GameTimer& gt)
{
	if (GetAsyncKeyState('1') & 0x8000)
		mIsWireframe = true;
	else
		mIsWireframe = false;
}

void ShapeWithLightApp::UpdateCamera(const GameTimer& gt)
{
	mEyePos.x = mRadius * sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadius * sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void ShapeWithLightApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto &e : mAllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			ObjectConstants objConstans;
			XMStoreFloat4x4(&objConstans.World, XMMatrixTranspose(world));

			currObjectCB->CopyData(e->ObjCBIndex, objConstans);
			e->NumFramesDirty--;
		}
	}
}

void ShapeWithLightApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void ShapeWithLightApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto matBuffer = mCurrFrameResource->MaterialCB.get();
	for (auto &e : mMaterials)
	{
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matCacheTransform = XMLoadFloat4x4(&mat->MatTransform);
			MaterialConstants matCache; 
			matCache.DiffuseAlbedo = mat->DiffuseAlbedo;
			matCache.FresnelR0 = mat->FresnelR0;
			matCache.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matCache.MatTransform, XMMatrixTranspose(matCacheTransform));

			matBuffer->CopyData(mat->MatCBIndex, matCache);

			mat->NumFramesDirty--;
		}
	}
}

void ShapeWithLightApp::BuildDescriptorHeaps()
{

	UINT objCount = mAllRitems.size();

	//渲染物体位置矩阵的ConstantBuffer+每个FrameResource的PrePass内容
	UINT numDescCount = (objCount + 1) * gNumFrameResources;
	
	//Pass的CBV在所有每个物体的矩阵CBV后面 
	mPassCbvOffset = objCount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.NumDescriptors = numDescCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	md3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mCbvHeap));
}

void ShapeWithLightApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice, 1, static_cast<UINT>(mAllRitems.size()),(UINT)mMaterials.size()));
	}
}

void ShapeWithLightApp::BuildConstantBufferViews()
{
	UINT objByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT objCount = mOpaqueRitems.size();

	for (int i = 0; i < gNumFrameResources; ++i)
	{
		auto objGPUResource = mFrameResources[i]->ObjectCB->Resource();
		for (UINT j = 0; j < objCount; ++j)
		{
			auto objGPUVAddr = objGPUResource->GetGPUVirtualAddress();
			objGPUVAddr += objByteSize * j;

			UINT heapIndex = i * objCount + j;
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = objGPUVAddr;
			//这里不能直接用buffer的size,因为有最小对齐要求
			//desc.SizeInBytes = sizeof(ObjectConstants);
			desc.SizeInBytes = objByteSize;
			md3dDevice->CreateConstantBufferView(&desc, handle);
		}
	}

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstans));
	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto passCBGPUResource = mFrameResources[frameIndex]->PassCB->Resource();
		auto passCBGPUAddr = passCBGPUResource->GetGPUVirtualAddress();
		UINT passHeapIndex = mPassCbvOffset + frameIndex;
		CD3DX12_CPU_DESCRIPTOR_HANDLE passCBHandle(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		passCBHandle.Offset(passHeapIndex, mCbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = passCBGPUAddr;
		desc.SizeInBytes = passCBByteSize;
		md3dDevice->CreateConstantBufferView(&desc, passCBHandle);
	}

}

void ShapeWithLightApp::BuildRootSignature()
{
	/*CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1,0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable2;
	cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsDescriptorTable(1, &cbvTable);
	rootParams[1].InitAsDescriptorTable(1, &cbvTable2);

	CComPtr<ID3DBlob> rootSigBlob;

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
*/
	

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);
	
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	CComPtr<ID3DBlob> serializedRootSig = nullptr;
	CComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSig, &errorBlob);


	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	/*ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf()))
	);*/
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(pTest.GetAddressOf()))
	);
	
}

void ShapeWithLightApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"APLHA_TEST", "1",
		NULL, NULL
	};

	mShaders["LightVS"] = d3dUtil::CompileShader(L"Apps\\ShapeWithLightApp\\MainLightPass.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["LightPS"] = d3dUtil::CompileShader(L"Apps\\ShapeWithLightApp\\MainLightPass.hlsl", nullptr, "PS", "ps_5_1");
	
	mInputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};
}	

void ShapeWithLightApp::BuildShapeGeometry()
{

}

void ShapeWithLightApp::BuildSkullGeometry()
{
	std::ifstream fin("Apps/ShapeWithLightApp/Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Skull.txt file not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;
	std::vector<Vertex> vetices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vetices[i].Pos.x >> vetices[i].Pos.y >> vetices[i].Pos.z;
		fin >> vetices[i].Normal.x >> vetices[i].Normal.y >> vetices[i].Normal.z;
	}
	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	const UINT vbByteSize = (UINT)vetices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto skullGeo = std::make_unique<MeshGeometry>();
	skullGeo->Name = "SkullGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &skullGeo->VertexBufferCPU));
	CopyMemory(skullGeo->VertexBufferCPU->GetBufferPointer(), vetices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &skullGeo->IndexBufferCPU));
	CopyMemory(skullGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	skullGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice, mCommandList, vetices.data(), vbByteSize,
		skullGeo->VertexBufferUploader);

	skullGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice, mCommandList, indices.data(), ibByteSize,
		skullGeo->IndexBufferUploader);

	skullGeo->VertexByteStride = sizeof(Vertex);
	skullGeo->VertexBufferByteSize = vbByteSize;
	skullGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
	skullGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	skullGeo->DrawArgs["skull"] = submesh;
	mGeometries[skullGeo->Name] = std::move(skullGeo);
}

void ShapeWithLightApp::BuildMaterials()
{
	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(Colors::ForestGreen);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;

	auto stone0 = std::make_unique<Material>();
	stone0->Name = "stone0";
	stone0->MatCBIndex = 1;
	stone0->DiffuseSrvHeapIndex = 1;
	stone0->DiffuseAlbedo = XMFLOAT4(Colors::LightSteelBlue);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 2;
	tile0->DiffuseSrvHeapIndex = 2;
	tile0->DiffuseAlbedo = XMFLOAT4(Colors::LightGray);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.2f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 3;
	skullMat->DiffuseSrvHeapIndex = 3;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05);
	skullMat->Roughness = 0.1f;

	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["stone0"] = std::move(stone0);
	mMaterials["tile0"] = std::move(tile0);
	mMaterials["skullMat"] = std::move(skullMat);
}

void ShapeWithLightApp::BuildPSOT()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	//opaquePsoDesc.pRootSignature = mRootSignature.Get();

	opaquePsoDesc.pRootSignature = pTest;

	BYTE *t = reinterpret_cast<BYTE*>(mShaders["LightVS"]->GetBufferPointer());

	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["LightVS"]->GetBufferPointer()),
		mShaders["LightVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["LightPS"]->GetBufferPointer()),
		mShaders["LightPS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));
}

void ShapeWithLightApp::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	//opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.pRootSignature = pTest;


	opaquePsoDesc.VS = 
	{
		reinterpret_cast<BYTE*>(mShaders["LightVS"]->GetBufferPointer()),
		mShaders["LightVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["LightPS"]->GetBufferPointer()),
		mShaders["LightPS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));
	//ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));

	//ATL::CComPtr<ID3D12PipelineState> xx = mPSOs["opaque"];
	//md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"]));
}

void ShapeWithLightApp::BuildRenderItems()
{
	auto skullRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skullRitem->World, XMMatrixScaling(0.5f, 0.5f, 0.5f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	
	skullRitem->TexTransform		= MathHelper::Identity4x4();
	skullRitem->ObjCBIndex			= 0;
	skullRitem->Mat					= mMaterials["skullMat"].get();
	skullRitem->Geo					= mGeometries["SkullGeo"].get();
	skullRitem->PrimitiveType		= D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullRitem->IndexCount			= skullRitem->Geo->DrawArgs["skull"].IndexCount;
	skullRitem->StartIndexLocation  = skullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
	skullRitem->BaseVerTexLocation = skullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;
	mAllRitems.push_back(std::move(skullRitem));

	for (auto &e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}

void ShapeWithLightApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize  = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT objMatByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	
	auto objectCB	= mCurrFrameResource->ObjectCB->Resource();
	auto matCB		= mCurrFrameResource->MaterialCB->Resource();


	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];
		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*objMatByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);
		/*UINT cbvIndex = mCurrFrameResourceIndex * (UINT)mOpaqueRitems.size() + ri->ObjCBIndex;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, mCbvSrvUavDescriptorSize);
		cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		*/cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVerTexLocation, 0);
	}
}


