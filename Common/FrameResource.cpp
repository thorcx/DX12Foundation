#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
	//在初始化FrameResource的时候，要把PassCB,ObjectCB的GPU内存映射都准备好
	//这样在Update的时候只需要关系Buffer内部的数据拷贝问题
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CmdListAlloc)));

	PassCB	= std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	if(materialCount != 0)
		MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
	
}

FrameResource::~FrameResource()
{

}