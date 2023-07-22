#include "..\Public\Channel.h"

#include "Model.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, _uint iBoneIndex)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.data);

	m_iBoneIndex = iBoneIndex;

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	_float3		vScale = { 0.f, 0.f, 0.f };
	_float4		vRotation = { 0.f, 0.f, 0.f, 0.f };
	_float3		vPosition = { 0.f, 0.f, 0.f };

	for (_uint i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME KeyFrame;
		ZeroMemory(&KeyFrame, sizeof KeyFrame);
	
		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.dTime = pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;
			KeyFrame.dTime = pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vPosition, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.dTime = pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vPosition = vPosition;

		m_KeyFrames.emplace_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Invalidate(CModel* pModel, _uint& pCurrentKeyFrame, _double TrackPosition)
{
	if (0.0 == TrackPosition)
		pCurrentKeyFrame = 0;

	//Ư�� �ִϸ��̼��� �ð��� ���� ���� ���¸� �����Ѵ�.

	_float3		vScale;
	_float4		vRotation;
	_float3		vPosition;

	KEYFRAME	LastKeyFrame = m_KeyFrames.back();
	//�� �ִϸ��̼��� ������ Ű ������

	if (TrackPosition >= LastKeyFrame.dTime)
	{
		/*��Ȥ ��ü ����ð� ������ Ű �������� ������ ��� ���� ����ð� ���� 
		* ������ ���¸� ���������ֱ� ���� ����ó��
		*/
		vScale = LastKeyFrame.vScale;
		vRotation = LastKeyFrame.vRotation;
		vPosition = LastKeyFrame.vPosition;
	}
	else
	{
		while (TrackPosition >= m_KeyFrames[pCurrentKeyFrame + 1].dTime)
			++pCurrentKeyFrame;
		//��� �ð��� ���� Ű���������� �Ѿ�� ���� Ű �������� ������Ų��

		_double dRatio = (TrackPosition - m_KeyFrames[pCurrentKeyFrame].dTime)
			/ (m_KeyFrames[pCurrentKeyFrame + 1].dTime - m_KeyFrames[pCurrentKeyFrame].dTime);
		/*�� Ű ������ �������� ���� �󸶳� ����ƴ��� 0 ~ 1
		* 1���� Ŀ���� ���� Ű���������� �Ѿ�� - ���� if���� ����
		*/

		_float3		vSourScale, vDestScale;
		_float4		vSourRotation, vDestRotation;
		_float3		vSourPosition, vDestPosition;
		
		vSourScale = m_KeyFrames[pCurrentKeyFrame].vScale;
		vDestScale = m_KeyFrames[pCurrentKeyFrame + 1].vScale;

		vSourRotation = m_KeyFrames[pCurrentKeyFrame].vRotation;
		vDestRotation = m_KeyFrames[pCurrentKeyFrame + 1].vRotation;

		vSourPosition = m_KeyFrames[pCurrentKeyFrame].vPosition;
		vDestPosition = m_KeyFrames[pCurrentKeyFrame + 1].vPosition;

		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vSourScale), XMLoadFloat3(&vDestScale), (_float)dRatio));
		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vSourRotation), XMLoadFloat4(&vDestRotation), (_float)dRatio));
		XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vSourPosition), XMLoadFloat3(&vDestPosition), (_float)dRatio));
		
		/*�� ������ ������ ���¸� ���������ϴ� �۾� - Lerp, Slerp
		* Ű������ �������� - Sour
		* Ű������ ������ - Dest
		*/
	}

	_float4 vTranslation = _float4(vPosition.x, vPosition.y, vPosition.z, 1.f);

	_matrix TransformationMatrix = XMMatrixAffineTransformation(
		XMLoadFloat3(&vScale),
		XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation),
		XMLoadFloat4(&vTranslation));

	pModel->Get_Bone(m_iBoneIndex)->Set_TransformationMatrix(TransformationMatrix);
	//������ ������ ���·� ����� �����, �ش� ��ķ� ���� �����Ѵ�
}

CChannel* CChannel::Create(const aiNodeAnim* pAIChannel, _uint iBoneIndex)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, iBoneIndex)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChannel::Free()
{
}
