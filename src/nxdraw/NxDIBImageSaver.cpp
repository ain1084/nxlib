// NxDIBImageSaver.cpp: CNxDIBImageSaver �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �̕ۑ����s���ׂ̃v���g�R���N���X
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImageSaver.h"
#include "NxDIBImage.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImageSaver::CNxDIBImageSaver()
// �T�v: CNxDIBImageSaver �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxDIBImageSaver::CNxDIBImageSaver()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImageSaver::~CNxDIBImageSaver()
// �T�v: CNxDIBImageSaver �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxDIBImageSaver::~CNxDIBImageSaver()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxImageHandler::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//											   const RECT* lpRect = NULL) const = 0
// �T�v: CNxDIBImage �I�u�W�F�N�g�̓��e���t�@�C���֕ۑ�
// ����: CNxFile& nxfile                   ... �ۑ��� CNxFile �I�u�W�F�N�g�ւ̎Q��
//       const CNxDIBImage& srcDIBImage    ... �ۑ������ CNxDIBImage �I�u�W�F�N�g�ւ̎Q��
//       const RECT* lpRect                ... �ۑ���`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& /*srcDIBImage*/, const RECT* /*lpRect*/) const
{
	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : �t�@�C���͊J����Ă��܂���.\n");
		return FALSE;
	}
	return TRUE;
}
