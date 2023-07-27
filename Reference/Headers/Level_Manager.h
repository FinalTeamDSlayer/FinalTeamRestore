#pragma once
#include "Base.h"
#include "Level.h"

BEGIN(Engine)

class CLevel_Manager final : public CBase
{
    DECLARE_SINGLETON(CLevel_Manager)
private:
    CLevel_Manager();
    virtual ~CLevel_Manager() = default;

public:
    _bool   Get_IsStage() const {
        return m_pCurrentLevel->Get_IsStage();
    }

public:
    HRESULT Open_Level(_uint iLevelIndex, class CLevel* pNextLevel, _bool isStage);   //���� �̵� �� ����� ��
    void    Tick_Level(_double dTimeDelta);

private:
    CLevel*     m_pCurrentLevel = { nullptr };  //���� ����
    _uint       m_iLevelIndex = { 0 };

public:
    virtual void Free() override;
};

END