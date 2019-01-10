#pragma once

#include "d3dUtil.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
};


struct PassConstans
{
	DirectX::XMFLOAT4X4	view	= MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj	= MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 viewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3	EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f,0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	//在GPU处理完当前帧内容前无法使用reset重置commandlist,否则allocator中的内存会被覆盖,因此每个FrameResource都需要维护一个Allocator
	ATL::CComPtr<ID3D12CommandAllocator>			CmdListAlloc = nullptr;


	std::unique_ptr<UploadBuffer<PassConstans>>		passCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>>	ObjectCB = nullptr;

	//设置一个Fence计数,标记当前GPU队列Command执行到计数器的位置,我们通过检查Fence的值来判断当前FrameResource是否
	//已经被GPU执行完毕
	UINT64 Fence = 0;
};