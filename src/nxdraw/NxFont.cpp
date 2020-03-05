// NxFont.cpp: CNxFont �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#include <nxdraw.h>
#include "NxFont.h"
#include "NxSurface.h"


//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont()
// �T�v: CNxFont �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// ���l: �t�H���g�� "Arial", �T�C�Y�� 10 �ŏ�����
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont()
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
{
	memset(&m_lf, 0, sizeof(LOGFONT));
	m_lf.lfHeight = 10;
	m_lf.lfCharSet = DEFAULT_CHARSET;
	::lstrcpyn(m_lf.lfFaceName, _T("Arial"), LF_FACESIZE);
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(LPCTSTR lpszFaceName, LONG lSize)
// �T�v: CNxFont �N���X�̃R���X�g���N�^
//       �t�H���g���ƃT�C�Y���w�肵�ď�����
// ����: LPCTSTR lpszFaceName ... �t�H���g�̖��O
//       LONG lSize           ... �t�H���g�̃T�C�Y
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(LPCTSTR lpszFaceName, LONG lSize)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
{
	memset(&m_lf, 0, sizeof(LOGFONT));
	m_lf.lfHeight = lSize;
	m_lf.lfCharSet = DEFAULT_CHARSET;
	::lstrcpyn(m_lf.lfFaceName, lpszFaceName, LF_FACESIZE);
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(const LONGFONT* lpLogFont)
// �T�v: CNxFont �N���X�̃R���X�g���N�^
//       LOGFONT �\���̂��R�s�[���ď�����
// ����: const LOGFONT* lpLogFont ... �R�s�[�� LOGFONT �\����
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(const LOGFONT* lpLogFont)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
 , m_lf(*lpLogFont)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(const CNxFont& font)
// �T�v: CNxFont �N���X�̃R�s�[�R���X�g���N�^
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(const CNxFont& font)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
 , m_lf(font.m_lf)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont& CNxFont::operator=(const CNxFont& font)
// �T�v: CNxFont �N���X�̑�����Z�q
//////////////////////////////////////////////////////////////////////

CNxFont& CNxFont::operator=(const CNxFont& font)
{
	if (this != &font)
	{
		m_lf = font.m_lf;
		m_bDirty = TRUE;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::~CNxFont()
// �T�v: CNxFont �N���X�̃f�X�g���N�^
// ���l: �쐬�����t�H���g�͍폜�����
//////////////////////////////////////////////////////////////////////

CNxFont::~CNxFont()
{
	deleteFont();
}

//////////////////////////////////////////////////////////////////////
// private:
//	void CNxFont::deleteFont()
// �T�v: �쐬�����t�H���g���폜
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

void CNxFont::deleteFont()
{
#if _MSC_VER >= 1200
#pragma warning (disable : 4390)
#endif
	if (m_hFont != NULL)
	{
		if (!::DeleteObject(m_hFont))
		{
			_RPTF0(_CRT_WARN, "CNxFont : �t�H���g�̍폜�Ɏ��s���܂����B�t�H���g���f�o�C�X�R���e�L�X�g�֑I������Ă���\��������܂�.\n");
		}
		m_hFont = NULL;
	}
	if (m_hFontSmooth != NULL)
	{
		if (!::DeleteObject(m_hFontSmooth))
		{
			_RPTF0(_CRT_WARN, "CNxFont : �t�H���g�̍폜�Ɏ��s���܂����B�t�H���g���f�o�C�X�R���e�L�X�g�֑I������Ă���\��������܂�.\n");
		}
		m_hFontSmooth = NULL;
	}
#if _MSC_VER >= 1200
#pragma warning (default : 4390)
#endif
	m_bDirty = FALSE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFont::Attach(HFONT hFont)
// �T�v: HFONT �����ѕt����
// ����: HFONT hFont ... �t�H���g�ւ̃n���h��
// �ߒl: �����Ȃ� TRUE
// ���l: ���ѕt����ꂽ�t�H���g�̏��L���� CNxFont �ֈڂ�܂��B
//////////////////////////////////////////////////////////////////////

BOOL CNxFont::Attach(HFONT hFont)
{
	// ���݂̃t�H���g���폜
	deleteFont();

	// HFONT ���� LOGFONT ���擾
	if (::GetObject(hFont, sizeof(LOGFONT), &m_lf) == 0)
	{
		_RPTF0(_CRT_ASSERT, "CNxFont::Attach() : ::GetObject() �Ɏ��s���܂���.\n");
		return FALSE;
	}
	m_hFont = hFont;
	m_bDirty = FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	HFONT CNxFont::Detach()
// �T�v: CNxFont ���� HFONT ��؂藣���ĕԂ�
// ����: �Ȃ�
// �ߒl: �����Ȃ�t�H���g�̃n���h��
// ���l: �t�H���g���쐬����Ă��Ȃ���΁A�쐬���ĕԂ��B
//       �Ԃ��ꂽ�n���h�����s�v�ɂȂ����� ::DeleteObject() �ō폜���ĉ�����
//////////////////////////////////////////////////////////////////////

HFONT CNxFont::Detach()
{
	HFONT hFont = GetHandle();
	m_hFont = NULL;					// HFONT �؂藣��
	return hFont;
}

//////////////////////////////////////////////////////////////////////
// public:
//	HFONT CNxFont::GetHandleInternal(BOOL bForSmooth)
// �T�v: HFONT ���쐬���ĕԂ�
// ����: ---
// �ߒl: �����Ȃ�t�H���g�̃n���h��
// ���l: �����p�֐��ł��B���ڎg�p���Ȃ��ł�������
//////////////////////////////////////////////////////////////////////

HFONT CNxFont::GetHandleInternal(FontType fontType)
{
	if (m_bDirty)
	{	// LOGFONT ���ύX���ꂽ�Ȃ�΁A���݂̃t�H���g���폜
		deleteFont();
	}

	if (fontType == FontType_Smooth)
	{	// �X���[�W���O�p�t�H���g�̍쐬
		// height, width �� CNxSurface::SmoothFontRatio(4) �{�ɂ��č쐬
		if (m_hFontSmooth == NULL)
		{
			LOGFONT lf = m_lf;
			lf.lfHeight *= CNxSurface::SmoothFontRatio;
			lf.lfWidth *= CNxSurface::SmoothFontRatio;
			m_hFontSmooth = ::CreateFontIndirect(&lf);
			if (m_hFontSmooth == NULL)
			{
				_RPTF0(_CRT_ASSERT, "CNxFont::GetFont() �X���[�W���O�p�t�H���g�̍쐬�Ɏ��s���܂���.\n");
				return NULL;
			}
		}
		return m_hFontSmooth;
	}
	else
	{	// �ʏ�t�H���g�̍쐬
		if (m_hFont == NULL)
		{
			m_hFont = ::CreateFontIndirect(&m_lf);
			if (m_hFont == NULL)
			{
				_RPTF0(_CRT_ASSERT, "CNxFont::GetFont() �t�H���g�̍쐬�Ɏ��s���܂���.\n");
				return NULL;
			}
		}
		return m_hFont;
	}
}
