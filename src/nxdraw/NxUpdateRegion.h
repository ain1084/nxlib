// NxUpdateRegion.h: CNxUpdateRegion �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#pragma once

namespace NxDrawLocal
{

class _declspec(novtable) CNxUpdateRegion
{
public:
	virtual ~CNxUpdateRegion() { }
	virtual void AddRect(const RECT* lpRect) = 0;
};

}
