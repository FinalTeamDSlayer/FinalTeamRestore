//��ɾ�(����/���͸� ����)	�ɼ�(����� ����� ����)		���� ������ �ִ� ��ġ						�纻 ������ ������ ��ġ
xcopy						/y								.\Engine\Bin\Engine.dll					.\Client\Bin\

xcopy						/y								.\Engine\Bin\Engine.lib					.\Reference\Librarys\
xcopy						/y								.\Engine\ThirdPartyLib\*.lib			.\Reference\Librarys\
xcopy						/y/s							.\Engine\Public\*.*						.\Reference\Headers\
xcopy						/y								.\Engine\Bin\ShaderFiles\*.hlsl			.\Reference\ShaderFiles\
xcopy						/y								.\Engine\Bin\ShaderFiles\*.hpp			.\Reference\ShaderFiles\

xcopy						/y								.\Engine\Bin\Engine.dll					.\Tool\Bin\

