#include "..\Public\Animation.h"

#include "Channel.h"
#include "Model.h"
#include "Bone.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& rhs)
	: m_dDuration(rhs.m_dDuration)
	, m_dTickPerSecond(rhs.m_dTickPerSecond)
	, m_dTimeAcc(rhs.m_dTimeAcc)
	, m_isFinish(rhs.m_isFinish)
	, m_iNumChannels(rhs.m_iNumChannels)
	, m_Channels(rhs.m_Channels)
	, m_iCurrentKeyFrames(rhs.m_iCurrentKeyFrames)
{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	strcpy_s(m_szName, rhs.m_szName);
}

HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, CModel* pModel)
{
	strcpy_s(m_szName, pAIAnimation->mName.data);
	m_dDuration = pAIAnimation->mDuration;
	m_dTickPerSecond = pAIAnimation->mTicksPerSecond;
	
	m_iNumChannels = pAIAnimation->mNumChannels;
	m_iCurrentKeyFrames.resize(m_iNumChannels);
	
	for (_uint i = 0; i < m_iNumChannels; i++)
	{
		//�� �ִϸ��̼ǿ��� �����̴� ���� �̸��� ���� ���� ���� ã�� pBone�� ����
		CBone * pBone = pModel->Get_Bone(pAIAnimation->mChannels[i]->mNodeName.data);

		CChannel* pChannel = CChannel::Create(pAIAnimation->mChannels[i], pModel->Get_BoneIndex(pBone->Get_Name()));
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.emplace_back(pChannel);
	}

	return S_OK;
}

void CAnimation::Invalidate_TransformationMatrices(CModel* pModel, _double dTimeDelta)
{
	m_isFinish = false;

	/* ���� ����Ǵ� �ִϸ��̼� */
	m_dTimeAcc += m_dTickPerSecond * dTimeDelta;

	if (m_dDuration <= m_dTimeAcc)
	{
		// ��ü ����ð����� �����ð��� Ŀ���� == �ִϸ��̼��� ������
		m_isFinish = true;
		m_dTimeAcc = 0.0;
	}

	_uint	iIndex = { 0 };
	for (auto& pChannel : m_Channels)
	{
		//�� �ִϸ��̼ǿ��� �����̴� ������ ���¸� �ð��� �°� �����Ѵ�.
		pChannel->Invalidate(pModel, m_iCurrentKeyFrames[iIndex++], m_dTimeAcc);
	}
}

CAnimation* CAnimation::Create(const aiAnimation* pAIAnimation, class CModel* pModel)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(pAIAnimation, pModel)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}
