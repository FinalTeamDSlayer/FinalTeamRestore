
#include "Shader_Defines.hpp"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_Texture;

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};

VS_OUT VS_Main(VS_IN _In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix matWV	= mul(g_WorldMatrix, g_ViewMatrix);
	matrix matWVP	= mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(_In.vPosition, 1.f), matWVP);
	Out.vTexUV = _In.vTexUV;

	return Out;
};

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	vector		vColor : SV_TARGET0;
};

PS_OUT  PS_Main(PS_IN _In)
{
	PS_OUT	Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, _In.vTexUV);

	return Out;
};

technique11 DefaultTechnique
{
	pass General
	{
		VertexShader = compile vs_5_0 VS_Main();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_Main();
	}

	pass Cloaking
	{
		VertexShader = compile vs_5_0 VS_Main();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_Main();
	}

};



////======================================���̴� �����=============================================
//// ���̴� = ����� ���� ������ ������ ����
//// DX9 = ������� ������ ������ ����
//
//// ��ġ�� �˾Ƽ� ���ִ� ������ ������������ ������� �ʰ�
//// ���� ������ ������������ ����� �����ϸ鼭
//// ��� �߰� �� ����ȭ�� �ϴ°� = ���̴��� ����Ѵ�
//
//// ���̴��� �����Ǵ� ����
//// ���̴� �ڵ带 ��üȭ���Ѿ� �Ѵ� - ���̴� Ŭ���� ������Ʈ�� ���� ó���� ��
//// ID3DX11Effect* pEffect ��ü - ���̴� Ŭ������ ����������
//// pEffect���� Apply() - ���̴��� �׸����̴�.
//
//// m_pContext->DrawIndexed();
//// �ε��� ������ ù��° �ε����� ������ ��������
//// �� ������ ���ڷ� ���ؽ� ���̴� �Լ��� ȣ���Ѵ� (VS_Main())
//// �Լ� ȣ���� ������, ����Ʈ ��ȯ�� ����, ����Ʈ ��ȯ���� ������
//// ���� �ε����� ������ �����ͼ� �ٽ� ������ �����Ѵ�.
//// 3��° �������� ������ ������ (�ﰢ���� �׸� �� �ְ� �Ǹ�)
//// �����Ͷ���� �����Ѵ�. - ������ ������ ������� �ȼ��� �����س����� ����
//
////���̴� ������ /fx (ȿ��) ���̴��̱� ������ technique11�� �����Ѵ�
////� �Լ��� ȣ��� ������ ���̴� ���� �ȿ��� �˷��ִ°�
////�ٸ� ������ ��쿡�� ���̴� ���Ͽ��� �Լ��� �����, Ŭ���̾�Ʈ���� � �Լ��� ȣ������ �������ش�.
////������ �ϵ��� �´� ���̴� �������� ���̴��� ������ �� �� �ټ��� technique11�� �����Ѵ�
//
//struct VS_IN
//{
//	// Ŭ�󿡼� �����ִ� ������ ���� ����ü - ������ ���� ���� �ٸ� ���̴� ������ ���� ��
//	// ex)VTXTEX
//	// : POSITION - �ø�ƽ - D3DDECLUSAGE ����ü�� _POSITION, _NORMAL ������
//	// ��������� �ø�ƽ�� �������� ���� ����Ұ�
//	float3	vPosition : POSITION;
//	float2	vTexCoord : TEXCOORD0;
//};
//
//struct VS_OUT
//{
//	// ���� �ڵ忡���� VS_Main()���� ����, ��, ���� ��ȯ�� ��� �����Ѵ�
//	// ���� ����� ���� �ڿ� w��������� �ؾ� ������ȯ�� �Ϸ�ȴ�.
//	// VS_IN ������ Position�� float3�̶� w�� ����
//	// ������ȯ�� �Ϸ��� w���� �־�� w�����⸦ �� �� �ֱ⶧���� float4�� �������� �Ѵ�
//	// IN�� OUT�� �ڷ����� �޶����� ������ IN, OUT�� ���� �����ߴ�.
//
//
//	float4 vPosition : SV_POSITION;
//	float2 vTexCoord : TEXCOORD0;
//};
//
//VS_OUT VS_Main(VS_IN _In)
//{
//	//���� �ϳ��� �޾Ƽ� ������ ����� ������ ���� ���̴�
//
//	//_In.vPosition * ���� * �� * ����
//	// mul() - ����� ���� �� ���
//	// DX9���� ����ߴ� TransformCoord�� �ٸ��� �ڵ����� z�����⸦ �������� �ʴ´�.
//	// �ܼ��� ���ϱ��� ���
//}
//
////������ ����, ��, ���� ����� ��� ���ϰ� �Լ��� ����Ǹ�
//// �ڵ����� z �����⸦ �����ؼ� ���� ��ȯ�� �Ϸ��Ѵ�.
//// ���� ����Ʈ ��ȯ�� �Ѵ�.
//
////�����Ͷ�����
//struct PS_IN
//{
//	// �ȼ� ���̴����� ����ϴ� ����ü
//	// �ȼ��� ������ ������� ��������� ������ VS_OUT���� ����ص� ����������
//	// �������� ���� PS_IN���� �����
//
//	//�ȼ� ���̵� �ܰ迡���� �̹� �ȼ��� ��ġ�� �������ֱ� ������ Position�� ��� �ʿ����� �ʴ� - �����ص� ����
//	float4 vPosition : SV_POSITION;
//	float2 vTexCoord : TEXCOORD0;
//};
//
//struct PS_OUT
//{
//	//�ȼ� ���̴��� OUT����ü
//	//�ȼ� ���̴��� �ȼ��� ���� �����ϱ� ������ ���� ���� float4(vector)�� ������ �ȴ�.
//	//TARGET�� 0~7���� ����, �� ��ȣ�� ����Ÿ�ٿ� ��������� ������ �׸���
//	vector vColor : SV_TARGET0;
//	vector vShade : SV_TARGET1;
//};
//
//
//// vector PS_Main(PS_IN _In) : SV_TARGET0 ���� ����ص� ������ OUT ����ü�� ���� ����� ����Ұ�(������ ���)
//PS_OUT PS_Main(PS_IN _In)
//{
//	// �ȼ��� �������� ���� �����Ѵ�.
//	// �ȼ��� �� ��ŭ ����Ǳ� ������ �ִ��� ������ ����°� ����
//
//	PS_OUT vOut = (PS_OUT)0;	//vOut�� 0���� �ʱ�ȭ
//
//
//	return vOut;
//}
//
//technique11 DefaultTechnique
//{
//	//������� ���� ���̴��� ������� ������ �ٸ� ���̴� ���� ��
//	//������� ���� ���̴��� �����ȴ�.
//	//���� ������� �ʵ��� ����� �� ���̴�. (������ ���)
//
//	//VertexShader = compile vs_5_0 VS_Main();
//	// ���ؽ� ���̴� = vs_5_0�������� �����ϰ�, VS_Main() �Լ��� ȣ���Ѵ�
//	// VS_Main()�Լ��� �������� ������ ������ �߻��Ѵ�
//	//GeometryShader = NULL;
//	//HullShader = NULL;
//	//DomainShader = NULL;
//	//PixelShader = compile vs_5_0 PS_Main();
//	// �ȼ� ���̴� = vs_5_0�������� �����ϰ�, PS_Main() �Լ��� ȣ���Ѵ�
//
//
//
//	//pass - ����� ĸ��ȭ
//
//	//0�� ���� - �Ϲ� - �׸���, �븻��, �𵨸� ���� ����
//	//1�� ���� - ���� - ����������� ��ȿ������ ����
//	//���� ���·� ���� ������ ���� ���¸� �ٸ��� �� ��쿡 ����ϱ� ����
//	//[���� ����] -- �߿�
//	pass General
//	{
//		VertexShader = compile vs_5_0 VS_Main();
//		GeometryShader = NULL;
//		HullShader = NULL;
//		DomainShader = NULL;
//		PixelShader = compile vs_5_0 PS_Main();
//	};
//
//	pass Cloaking
//	{
//		VertexShader = compile vs_5_0 VS_Main();
//		GeometryShader = NULL;
//		HullShader = NULL;
//		DomainShader = NULL;
//		PixelShader = compile vs_5_0 PS_Main();
//	};
//};
