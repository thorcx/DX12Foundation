#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
	//�ڳ�ʼ��FrameResource��ʱ��Ҫ��PassCB,ObjectCB��GPU�ڴ�ӳ�䶼׼����
	//������Update��ʱ��ֻ��Ҫ��ϵBuffer�ڲ������ݿ�������
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