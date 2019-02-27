#pragma once

#include "../../Common/d3dApp.h"
#include "../../Common/FrameResource.h"
#include "../../ThorLib/Pointer/TxRefPtr.h"
#include "Waves.h"

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	Count
};

class CrateWithTexApp :public D3DApp
{

	struct RenderItem
	{
		RenderItem() = default;

		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

		DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

		int NumFramesDirty = gNumFrameResources;

		UINT ObjCBIndex = -1;

		Material *Mat = nullptr;
		MeshGeometry* Geo = nullptr;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		int BaseVertexLocation = 0;
	};

public:
	CrateWithTexApp(HINSTANCE hInstance);
	CrateWithTexApp(const CrateWithTexApp &rhs) = delete;
	CrateWithTexApp& operator=(const CrateWithTexApp& rhs) = delete;
	~CrateWithTexApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer &gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer &gt);


	void LoadTextures();
	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildShaderAndInputLayout();
	void BuildShapeGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();
	void BuildLandGeometry();
	void BuildWavesGeometry();

	float GetHillsHeight(float x, float z) const;
	DirectX::XMFLOAT3 GetHillsNormal(float x, float z) const;

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
private:

	

	ThorcxLib::TRefCountPtr<ID3D12RootSignature>	mRootSignature = nullptr;
	ThorcxLib::TRefCountPtr<ID3D12DescriptorHeap>	mSrvDescriptorHeap = nullptr;
	ThorcxLib::TRefCountPtr<ID3D12PipelineState>	mOpaquePSO = nullptr;

	std::vector<RenderItem*> mOpaqueRitems;
	std::vector<std::unique_ptr<RenderItem>>	mAllRitems;
	std::vector<std::unique_ptr<FrameResource>>	mFrameResources;


	RenderItem* mWavesRitem = nullptr;


	//Render items divided by PSO
	std::vector<RenderItem*>	mRitemLayer[(int)RenderLayer::Count];
	std::unique_ptr<Waves>		mWaves;

	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	std::vector<D3D12_INPUT_ELEMENT_DESC>	mInputLayout;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>				mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>>					mMaterials;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>			mShaders;

	std::unordered_map<std::string, std::unique_ptr<Texture>>					mTextures;

	std::unordered_map<std::string, ThorcxLib::TRefCountPtr<ID3D12PipelineState>> mPSOs;



	PassConstants mMainPassCB;

	DirectX::XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.3f * DirectX::XM_PI;
	float mPhi = 0.4f * DirectX::XM_PI;
	float mRadius = 2.5f;

	POINT mLastMousePos;
};