#include "pch.h"
#include "..\Public\AnimCharacter_Tool.h"

#include "GameInstance.h"

CAnimCharacter_Tool::CAnimCharacter_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter_Tool(pDevice, pContext)
	, m_pImGui_Anim(CImGui_Animation_Tool::GetInstance())
{
	Safe_AddRef(m_pImGui_Anim);
}

CAnimCharacter_Tool::CAnimCharacter_Tool(const CAnimCharacter_Tool& rhs)
	: CCharacter_Tool(rhs)
	, m_pImGui_Anim(rhs.m_pImGui_Anim)
{
}

HRESULT CAnimCharacter_Tool::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;


	return S_OK;
}

HRESULT CAnimCharacter_Tool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(0);


	return S_OK;
}

void CAnimCharacter_Tool::Tick(_double dTimeDelta)
{
	if (true == m_isDead)
		return;

	ImGUI_Control(dTimeDelta);

	KeyInput(dTimeDelta);

	m_pModelCom->Play_Animation(dTimeDelta);


	__super::Tick(dTimeDelta);
}

void CAnimCharacter_Tool::LateTick(_double dTimeDelta)
{
	__super::LateTick(dTimeDelta);


	if (FAILED(m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this)))
		return;

#ifdef _DEBUG
	/*if (FAILED(m_pRendererCom->Add_DebugGroup(m_pNavigationCom)))
		return;*/
#endif 
}

HRESULT CAnimCharacter_Tool::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(i, m_pShaderCom, "g_DiffuseTexture", MESHMATERIALS::TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_ShaderBoneMatrices(i, m_pShaderCom, "g_BoneMatrices")))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CAnimCharacter_Tool::ImGUI_Control(_double dTimeDelta)
{
	//���۾��� ���� ù �ִϸ��̼� �ʱ�ȭ
	if (m_isFirst_Name)
	{
		m_isFirst_Name = false;

		vector<CAnimation*> vecAnim = m_pModelCom->Get_vecAnimation();
		for (auto& pAnim : vecAnim)
		{
			// �ִϸ��̼� ����Ʈ ��� index


			const char* szName = (pAnim->Get_AnimationDesc()).m_szName;
			size_t len = strlen(szName) + 1; // +1 for the null-terminator
			char* pNewName = new char[len];
			strcpy_s(pNewName, len, szName);

			m_vecName.emplace_back(pNewName);

			// isPlay �ʱ�ȭ false��
			CAnimation::CONTROLDESC control = pAnim->Get_ControlDesc();
			control.m_isPlay = false;
			pAnim->Set_ControlDesc(control);
		}
		m_pImGui_Anim->Set_vecName(m_vecName);
	}
	m_iNumAnim = m_pImGui_Anim->Get_AnimIndex();

	m_pModelCom->Set_Animation(m_iNumAnim);



	// ImGui�� ���� �ش� �ִϸ��̼� ������ �ֱ�. 
	m_pImGui_Anim->Set_Animation(m_pModelCom->Get_Animation());



	// ���� GUI ����
	m_pImGui_Anim->Animation_ImGui_Main();
}

void CAnimCharacter_Tool::KeyInput(_double dTimeDelta)
{
	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (pGameInstance->Get_DIKeyDown(DIK_UPARROW))
	{
		++m_iNumAnim;
		if (m_pModelCom->Get_NumAnims() <= m_iNumAnim)
			m_iNumAnim = m_pModelCom->Get_NumAnims() - 1;
		m_pModelCom->Set_Animation(m_iNumAnim);
	}
	else if (pGameInstance->Get_DIKeyDown(DIK_DOWNARROW))
	{
		if (0 < m_iNumAnim)
			--m_iNumAnim;
		if (0 > m_iNumAnim)
			m_iNumAnim = 0;
		m_pModelCom->Set_Animation(m_iNumAnim);
	}

	Safe_Release(pGameInstance);
}

HRESULT CAnimCharacter_Tool::Add_Components()
{
	// for.Com_Model 
	if (FAILED(__super::Add_Component(LEVEL_TOOL, TEXT("Prototype_Component_Model_Tanjiro"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
	{
		MSG_BOX("Failed to Add_Com_Model : CAnimCharacter_Tool");
		return E_FAIL;
	}

	// for.Com_Shader 
	if (FAILED(__super::Add_Component(LEVEL_TOOL, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
	{
		MSG_BOX("Failed to Add_Com_Shader : CAnimCharacter_Tool");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CAnimCharacter_Tool::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	_float4x4 ViewMatrix = pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW);
	if (FAILED(m_pShaderCom->SetUp_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;

	_float4x4 ProjMatrix = pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ);
	if (FAILED(m_pShaderCom->SetUp_Matrix("g_ProjMatrix", &ProjMatrix)))
		return E_FAIL;

	Safe_Release(pGameInstance);

	return S_OK;
}

CAnimCharacter_Tool* CAnimCharacter_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimCharacter_Tool* pInstance = new CAnimCharacter_Tool(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAnimCharacter_Tool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimCharacter_Tool::Clone(void* pArg)
{
	CAnimCharacter_Tool* pInstance = new CAnimCharacter_Tool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAnimCharacter_Tool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimCharacter_Tool::Free()
{
	__super::Free();

	Safe_Release(m_pImGui_Anim);

	for (auto& pName : m_vecName)
	{
		delete[] pName;
	}
	m_vecName.clear();
	
}
