#pragma once
#include "../../Common/d3dApp.h"
#include "../../Common/FrameResource.h"

extern const int gNumFrameResources;

struct RenderItem;

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	ShapesApp(const ShapesApp& rhs) = delete;
	ShapesApp& operator=(const ShapesApp& rhs) = delete;
	~ShapesApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	//只构建PerPass的ConstantView
	void BuildPerPassConstViews();
	void BuildRootSignature();
	void BuildRootSignature1();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

private:
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;

	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		mRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	mCbvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>				mGeometries;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>			mShaders;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	std::vector<RenderItem*> mOpaqueRitems;

	PassConstans mMainPassCB;

	UINT mPassCbvOffset = 0;

	bool mIsWireframe = false;

	DirectX::XMFLOAT3		mEyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4		mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4		mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * DirectX::XM_PI;
	float mPhi = 0.2f * DirectX::XM_PI;
	float mRadius = 15.0f;

	POINT	mLastMousePos;

};
