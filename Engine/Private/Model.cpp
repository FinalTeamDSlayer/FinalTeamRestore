#include "..\Public\Model.h"

#include "Animation.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Bone.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice, pContext)
{
}

CModel::CModel(const CModel& rhs)
	: CComponent(rhs)
	, m_pAIScene(rhs.m_pAIScene)
	, m_iNumMeshes(rhs.m_iNumMeshes)
	, m_Meshes(rhs.m_Meshes)
	, m_iNumMaterials(rhs.m_iNumMaterials)
	, m_Materials(rhs.m_Materials)
	, m_PivotMatrix(rhs.m_PivotMatrix)
	, m_iCurrentAnimIndex(rhs.m_iCurrentAnimIndex)
	, m_iNumAnimations(rhs.m_iNumAnimations)
{
	for (auto& pPrototypeBone : rhs.m_Bones)
		m_Bones.emplace_back(pPrototypeBone->Clone());

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_AddRef(Material.pTexture[i]);
	}

	for (auto& pPrototypeAnim : rhs.m_Animations)
		m_Animations.emplace_back(pPrototypeAnim->Clone());
}

CBone* CModel::Get_Bone(const char* pBoneName)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
		{
			if (false == strcmp(pBone->Get_Name(), pBoneName))
				return true;

			return false;
		}
	);

	if (iter == m_Bones.end())
		return nullptr;

	return *iter;
}

_int CModel::Get_BoneIndex(const char* pBoneName)
{
	_int iIndex = { -1 };

	_uint iNumBones = (_uint)m_Bones.size();

	for (_uint i = 0; i < iNumBones; i++)
	{
		if (false == strcmp(m_Bones[i]->Get_Name(), pBoneName))
		{
			iIndex = i;
			break;
		}
	}

	return iIndex;
}

HRESULT CModel::Initialize_Prototype(TYPE eModelType, const char* pModelFilePath, _fmatrix PivotMatrix)
{
	m_eModelType = eModelType;

	_uint iFlag = { 0 };

	if (TYPE_NONANIM == eModelType)
		iFlag = aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;
	else
		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("Failed to ReadModelFile : CModel");
		return E_FAIL;
	}

	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	if (FAILED(Ready_HierarchyBones(m_pAIScene->mRootNode, -1)))
	{
		MSG_BOX("Failed to Ready_HierarchyBones : CModel");
		return E_FAIL;
	}

	if (FAILED(Ready_Meshes()))
	{
		MSG_BOX("Failed to Ready_Meshes : CModel");
		return E_FAIL;
	}

	if (FAILED(Ready_Materials(pModelFilePath)))
	{
		MSG_BOX("Failed to Ready_Materials : CModel");
		return E_FAIL;
	}

	if (FAILED(Ready_Animations()))
	{
		MSG_BOX("Failed to Ready_Animations : CModel");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Play_Animation(_double dTimeDelta)
{
	m_Animations[m_iCurrentAnimIndex]->Invalidate_TransformationMatrices(this, dTimeDelta);
	/* ���� ����ؾ��� �ִϸ��̼� ��ȣ n == m_iCurrentAnimIndex
	*  n���� �ִϸ��̼��� ��� ���·� Trans����� �����Ѵ�.
	*/

	//������ ������ Trans�� �̿��Ͽ� ��� ���� Combined�� �����Ѵ�
	for (auto& pBone : m_Bones)
		pBone->Invalidate_CombinedTransformationMatrix(this);

	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CModel::Bind_ShaderResource(_uint iMeshIndex, CShader* pShader, const char* pConstantName, aiTextureType eType)
{
	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()].pTexture[eType]->Bind_ShaderResourceView(pShader, pConstantName);
	/* A�޽��� �׷����� ���� �ʿ��� B ��Ƽ������ ���� �ؽ�ó�� �� C�� ���̴��� ���������� �����ش�
	* A�޽��� �׷����� ���� �ʿ��� B ��Ƽ������ �ε��� = m_Meshes[iMeshIndex]->Get_MaterialIndex()
	* B��Ƽ������ ���� �ؽ�ó�� �� C = m_Materials[].pTexture[eType]
	* C�� ���̴��� ���������� �����ش� = ->Bind_ShaderResourceView(pShader, pConstantName)
	*/
}

HRESULT CModel::Bind_ShaderBoneMatrices(_uint iMeshIndex, CShader* pShader, const char* pConstantName)
{
	_float4x4	BoneMatrices[256];

	m_Meshes[iMeshIndex]->Get_BoneMatrices(BoneMatrices, XMLoadFloat4x4(&m_PivotMatrix), this);

	return pShader->SetUp_Matrix_Array(pConstantName, BoneMatrices, 256);
}

HRESULT CModel::Ready_Meshes()
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	m_iNumMeshes = m_pAIScene->mNumMeshes;

	for (_uint i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eModelType, m_pAIScene->mMeshes[i], XMLoadFloat4x4(&m_PivotMatrix), this);
		if (nullptr == pMesh)
		{
			MSG_BOX("Failed to CreateMesh : CModel");
			return E_FAIL;
		}

		m_Meshes.emplace_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const char* pModelFilePath)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	m_iNumMaterials = m_pAIScene->mNumMaterials;

	//���� ����(fbx)�� ��Ƽ���� �� ��ŭ �ݺ�
	for (_uint i = 0; i < m_iNumMaterials; i++)
	{
		aiMaterial* pAIMaterial = m_pAIScene->mMaterials[i];

		MESHMATERIALS MeshMaterial;
		ZeroMemory(&MeshMaterial, sizeof MeshMaterial);

		//��Ƽ������ ���� �� �ִ� �ִ� �ؽ�ó�� ����(18)��ŭ �ݺ�
		for (_uint j = 0; j < AI_TEXTURE_TYPE_MAX; j++)
		{
			aiString StrPath;
			//���Ͽ� ����� �ؽ�ó�� �̸��� StrPath�� ����
			if (FAILED(pAIMaterial->GetTexture(aiTextureType(j), 0, &StrPath)))
			{
				/*if (12 == j)
				{
					_splitpath_s(pModelFilePath, nullptr, 0, nullptr, 0, StrPath.data, MAX_PATH, nullptr, 0);
					strcat_s(StrPath.data, "_bc.png");
				}


				else*/
					continue;
			}

			//������ �ؽ�ó�� ��ο� �߸��� ���� ���� �� �����Ƿ� ���� �̸��� ��θ� �ٽ� ����ִ� �۾�

			char szDrive[MAX_PATH] = { "" };
			char szDir[MAX_PATH] = { "" };
			//��δ� �� ������ �������� �����´� = �𵨰� �ؽ�ó�� ���� ��ο� �־�� �Ѵ�
			_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);

			char szFileName[MAX_PATH] = { "" };
			char szExt[MAX_PATH] = { "" };

			_splitpath_s(StrPath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

			char szFullPath[MAX_PATH] = { "" };

			strcpy_s(szFullPath, szDrive);
			strcat_s(szFullPath, szDir);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szExt);

			_tchar	szConvertFullPath[MAX_PATH] = TEXT("");

			MultiByteToWideChar(CP_ACP, 0, szFullPath, (int)strlen(szFullPath), szConvertFullPath, MAX_PATH);

			//���� ����� ������ ã�Ƽ� �ؽ�ó�� �����

			MeshMaterial.pTexture[j] = CTexture::Create(m_pDevice, m_pContext, szConvertFullPath);
			if (nullptr == MeshMaterial.pTexture[j])
				return E_FAIL;
		}

		m_Materials.emplace_back(MeshMaterial);
	}

	return S_OK;
}

HRESULT CModel::Ready_HierarchyBones(aiNode* pAINode, _int iIndex)
{
	CBone* pBone = CBone::Create(pAINode, iIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.emplace_back(pBone);

	_uint	iParentIndex = (_uint)m_Bones.size() - 1;

	for (_uint i = 0; i < pAINode->mNumChildren; i++)
		Ready_HierarchyBones(pAINode->mChildren[i], iParentIndex);

	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	//���� �ִϸ��̼� ������ŭ CAnimation�� ����� ���Ϳ� ����
	m_iNumAnimations = m_pAIScene->mNumAnimations;

	for (_uint i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], this);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.emplace_back(pAnimation);
	}

	return S_OK;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eModelType, const char* pModelFilePath, _matrix PivotMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, pModelFilePath, PivotMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);

	m_Animations.clear();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);

	m_Bones.clear();


	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(Material.pTexture[i]);
	}

	m_Materials.clear();
}
