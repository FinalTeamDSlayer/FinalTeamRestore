#include "pch.h"
#include "..\Public\ImGui_Animation_Tool.h"


IMPLEMENT_SINGLETON(CImGui_Animation_Tool)

CImGui_Animation_Tool::CImGui_Animation_Tool()
{

}


void CImGui_Animation_Tool::Animation_ImGui_Main()
{
    CGameInstance* pGameInstance = CGameInstance::GetInstance();
    Safe_AddRef(pGameInstance);

    ImGui::Begin("Animation_List");

    CAnimation::ANIMATIONDESC AnimationDesc = m_pAnimation->Get_AnimationDesc();
    CAnimation::CONTROLDESC ControlDesc = m_pAnimation->Get_ControlDesc();

#pragma region Play(SpaceBar) 
    // ������ spacebar
    if (pGameInstance->Get_DIKeyDown(DIK_SPACE))
    {
        if (m_isPlay)
            m_isPlay = false;
        else
            m_isPlay = true;
    }
   
#pragma endregion



#pragma region Index_List  


    m_iSave_AnimIndex = m_iAnimIndex;

    ImGui::ListBox("Animation_List", &m_iAnimIndex, m_vecName_ForListBox.data(), (_int)(m_vecName_ForListBox.size()), 6);

    ImGui::Text("Current_Index : %d", m_iAnimIndex);
    
    // �ִϸ��̼� ��� "Ŭ�� ����ÿ���" �����ġ �ʱ�ȭ
    if (m_iSave_AnimIndex != m_iAnimIndex)
    {
        //AnimationDesc.m_dTimeAcc = 0.0;
        m_isPlay = false;
        m_Signal_to_Change_Anim = true;

        m_iSave_AnimIndex = m_iAnimIndex;
    }
    
#pragma endregion


    /* ������ index �� �ִϸ��̼����� �����. */
#pragma region Connect_Animation
    // Connect
    m_iConnectIndex = ControlDesc.m_iConnect_Anim;

    ImGui::Text("Connect to Index : %d", m_iConnectIndex);
    ImGui::InputInt("Connect to Next Animation", &m_iConnectIndex);
    
    ControlDesc.m_iConnect_Anim = m_iConnectIndex;
    m_pAnimation->Set_ControlDesc(ControlDesc);

#pragma endregion

#pragma region Root
    // Root
    m_isRootAnimation = ControlDesc.m_isRootAnimation;
    ImGui::Checkbox("Root Anim", &m_isRootAnimation);
    ControlDesc.m_isRootAnimation = m_isRootAnimation;
    m_pAnimation->Set_ControlDesc(ControlDesc);

#pragma endregion

    ImGui::Text("");


    /* üũ��, �޺��� ����� ������
        �޺� ����� �ִϸ��̼��� ������ ��, Ű�� ������ ���� �ִϸ��̼����� �� �� ������
        ������ ���� Ű�� �ȴ����� �״�� �����.*/
#pragma region Combo_Animation
        // Combo
    m_isCombo = ControlDesc.m_isCombo;
    ImGui::Checkbox("Combo_Animation", &m_isCombo);
    ControlDesc.m_isCombo = m_isCombo;
    m_pAnimation->Set_ControlDesc(ControlDesc);


    /* �޺� �� �ִϸ��̼����� �����. */

    if (m_isCombo)
    {
        // Connect
        m_iConnect_Combo_Index = ControlDesc.m_iConnect_ComboAnim;
        ImGui::Text("Connect to Combo Index : %d", m_iConnect_Combo_Index);
        ImGui::InputInt("Connect to Next Combo Animation", &m_iConnect_Combo_Index);
        ControlDesc.m_iConnect_ComboAnim = m_iConnect_Combo_Index;
        m_pAnimation->Set_ControlDesc(ControlDesc);
    }
#pragma endregion

/*    ImGui::End();

    ImGui::Begin("Animation Control Console");*/
    ImGui::Text("");
   
#pragma region Play_Slider  
    // Sliders ui
    static ImGuiSliderFlags AnimSliderflags = ImGuiSliderFlags_None;
 
    _float fCur_Time = (_float)AnimationDesc.m_dTimeAcc;
    _float fEnd_Time = (_float)AnimationDesc.m_dDuration;

    ImGui::Text("Current Time : %f", fCur_Time);
    ImGui::Text("End Time : %f", fEnd_Time);
    ImGui::SliderFloat("Play_Slider", &fCur_Time, 0.0f, fEnd_Time, "%.2f", AnimSliderflags);

    //���� �����̴�  ��ġ ���� �ִϸ��̼�������Ʈ�� �����ֱ�
    AnimationDesc.m_dTimeAcc = fCur_Time;
    m_pAnimation->Set_AnimationDesc(AnimationDesc);

#pragma endregion

#pragma region Add_Event

    static int Button_Event_Add = 0;
    if (ImGui::Button("Event_Add"))
        Button_Event_Add++;
    if (Button_Event_Add & 1)
    {
        Button_Event_Add--;
       
        CAnimation::EVENTDESC EventDesc;
        EventDesc.m_dTime = AnimationDesc.m_dTimeAcc;
        ControlDesc.m_vecTime_Event.emplace_back(EventDesc);
        m_pAnimation->Set_ControlDesc(ControlDesc);
    }

    static int Button_Event_Delete = 0;
    if (ImGui::Button("Event_Delete"))
        Button_Event_Delete++;
    if (Button_Event_Delete & 1)
    {
        Button_Event_Delete--;

        if (ControlDesc.m_vecTime_Event.size() > 0)
        {
            ControlDesc.m_vecTime_Event.pop_back();
            m_pAnimation->Set_ControlDesc(ControlDesc);
        }
    }



    ImGui::Text("Event Test int : %d", ControlDesc.m_iTest);

#pragma endregion


    ImGui::Text("");



    /* ���ǵ� ���� 0.1�� ~ 2.0�� ���� */
#pragma region AnimaSpeed_Slider  

   _float fSpeed = ControlDesc.m_fAnimationSpeed;

    ImGui::Text("Speed : %f", fSpeed);
    ImGui::SliderFloat("FrameSpeed ", &fSpeed, 0.1f, 2.0f, "%.2f");

    ControlDesc.m_fAnimationSpeed = fSpeed;
    m_pAnimation->Set_ControlDesc(ControlDesc);
    
#pragma endregion

    ImGui::Text("");
    ImGui::Text("");

#pragma region Save_Load  

    static int Button_Save = 0;
    if (ImGui::Button("Save_Animation"))
        Button_Save++;
    if (Button_Save & 1)
    {
        Button_Save--;

        m_isSave = true;
       
        MSG_BOX("Save_Complete");
    }

#pragma endregion


    ImGui::End();


#pragma region Event_Time_List  
    ImGui::Begin("Event_Time_List");
    
    // EventCall �ߵ� ����
    for (auto& event : ControlDesc.m_vecTime_Event)
    {
        _float fEventTime = (_float)event.m_dTime;
        ImGui::SliderFloat("Event", &fEventTime, 0.0f, fEnd_Time, "%.3f", AnimSliderflags);
    }
    ImGui::End();
#pragma endregion

    Safe_Release(pGameInstance);
}


void CImGui_Animation_Tool::Set_vecName(vector<char*> vecName)
{
    size_t iNameSize = vecName.size();
    m_vecName_ForListBox.reserve(iNameSize); // Reserve memory to avoid reallocation
    for (size_t i = 0; i < iNameSize; i++)
        m_vecName_ForListBox.push_back(vecName[i]);
}

void CImGui_Animation_Tool::Free()
{
}
