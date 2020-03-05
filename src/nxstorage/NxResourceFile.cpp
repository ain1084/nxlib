// NxResourceFile.cpp: CNxResourceFile �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxResourceFile.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpResType = RT_RCDATA)
// �T�v: CNxResourceFile �N���X�̃R���X�g���N�^
// ����: HINSTANCE hInstance ... �ǂݍ��ރ��\�[�X�̃��W���[��(instance)�n���h��
//       LPCTSTR lpResType   ... �ǂݍ��ރ��\�[�X�̎��
// �ߒl: �Ȃ�
// ���l: hInstance �� lpResType �̐ݒ�̂݁B���\�[�X�͓ǂݍ��܂�Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////

CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpResType)
{
	m_hInstance = hInstance;
	m_strType   = lpResType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpName, LPCTSTR lpResType = RT_RCDATA)
// �T�v: CNxResourceFile �N���X�̃R���X�g���N�^
// ����: HINSTANCE hInstance ... �ǂݍ��ރ��\�[�X�̃��W���[��(instance)�n���h��
//       LPCTSTR lpName      ... �ǂݍ��ރ��\�[�X�̖��O
//       LPCTSTR lpResType   ... �ǂݍ��ރ��\�[�X�̎��
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpName, LPCTSTR lpResType)
{
	m_hInstance = hInstance;
	m_strType   = lpResType;

	if (!Open(lpName))
	{
		_RPTF0(_CRT_ASSERT, _T("���\�[�X�̓ǂݍ��݂Ɏ��s���܂���.\n"));
	}
}

CNxResourceFile::~CNxResourceFile()
{
}

//////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxResourceFile::Open(LPCTSTR lpName)
// �T�v: ���\�[�X��ǂݍ��݁A�I�u�W�F�N�g���g�p�ł����Ԃɂ���
// ����: LPCTSTR lpName ... ���\�[�X�̖��O
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxResourceFile::Open(LPCTSTR lpName)
{
	HRSRC hRSRC;
	HGLOBAL hGlobal;
	LPVOID lpvData;
	DWORD dwSize;

	// ���\�[�X������
	hRSRC = ::FindResource(m_hInstance, lpName, m_strType);
	if (hRSRC == NULL)
		return FALSE;

	// ���\�[�X�̃T�C�Y�𓾂�
	dwSize = ::SizeofResource(m_hInstance, hRSRC);
	if (dwSize == NULL)
		return FALSE;
	
	// ���\�[�X�����[�h
	hGlobal = ::LoadResource(m_hInstance, hRSRC);
	if (hGlobal == NULL)
		return FALSE;

	// ���\�[�X�̃��b�N
	lpvData = ::LockResource(hGlobal);
	if (lpvData == NULL)
		return FALSE;

	MMIOINFO mi;
	memset(&mi, 0, sizeof(MMIOINFO));
	mi.fccIOProc = FOURCC_MEM;					// �������t�@�C��
	mi.pchBuffer = static_cast<LPSTR>(lpvData);	// �f�[�^�̐擪
	mi.cchBuffer = dwSize;						// �f�[�^�̃T�C�Y
	mi.adwInfo[0] = 0;							// �g���\�ȍŏ��o�C�g��(0 = �g���֎~)
	
	HMMIO hMMIO = ::mmioOpen(NULL, &mi, MMIO_READ);
	if (hMMIO == NULL)
		return NULL;

	std::basic_ostringstream<TCHAR> strTemp;
	strTemp.fill('0');
	strTemp.width(8);
	strTemp << _T("HINSTANCE = 0x") << std::hex << std::right << m_hInstance << std::endl;

	strTemp << _T("TYPE = ") << std::dec;
	if (m_strType.IsNumber())
		strTemp << LOWORD(static_cast<LPCTSTR>(m_strType)) << std::endl;
	else
		strTemp << '\'' << static_cast<LPCTSTR>(m_strType) << '\'' << std::endl;

	strTemp << _T("ID = ");
	if (HIWORD(lpName) == 0)
		strTemp << LOWORD(lpName);
	else
		strTemp << '\'' << lpName << '\'';

	return Attach(hMMIO, strTemp.str().c_str());
}

CNxResourceFile::CStringId::CStringId()
{
	m_lpString = NULL;
}

CNxResourceFile::CStringId::CStringId(LPCTSTR lpString)
{
	Set(lpString);
}

CNxResourceFile::CStringId::~CStringId()
{
	Remove();
}

CNxResourceFile::CStringId::CStringId(const CStringId& stringId)
{
	m_lpString = NULL;
	*this = stringId;
}

CNxResourceFile::CStringId& CNxResourceFile::CStringId::operator=(const CStringId& stringId)
{
	if (this != &stringId)
		Set(stringId.m_lpString);
	return *this;
}

void CNxResourceFile::CStringId::Set(LPCTSTR lpString)
{
	if (HIWORD(lpString) == 0)
		m_lpString = const_cast<LPTSTR>(lpString);
	else
		m_lpString = _tcsdup(lpString);
}

void CNxResourceFile::CStringId::Remove()
{
	if (HIWORD(m_lpString) == 0)
		return;

	free(m_lpString);
	m_lpString = NULL;
}
