// NxTrackingSprite.cpp: CNxTrackingSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxSprite �h���N���X�B�����ǐՋ@�\�t���ŏ�ʃX�v���C�g�N���X
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxUpdateRegion.h"
#include "NxTrackingSprite.h"

class CNxTrackingSprite::CNxTrackingUpdateRegion : public NxDrawLocal::CNxUpdateRegion
{
public:
	enum
	{	// �f�t�H���g�̒P��
		DefaultXUnit = 32,
		DefaultYUnit =  8
	};


	CNxTrackingUpdateRegion(CNxSprite* pSprite, int nWidth, int nHeight, int nXUnit = DefaultXUnit, int nYUnit = DefaultYUnit);
	virtual ~CNxTrackingUpdateRegion();

	// CNxUpdateRegion override
	virtual void AddRect(const RECT* lpRect);

	void EnumRect(int nMaxWidth, int nMaxHeight, LPVOID lpContext);
	int GetWidth() const {
		return m_nRealWidth; }
	int GetHeight() const {
		return m_nRealHeight; }
	void Invalidate() {
		memset(m_pbFlag, 1, m_nLength); }
	static void GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound);

private:

	inline int GetLineLength(const BYTE* p, int nX, int nMaxWidth) const;

	int m_nWidth;					// �t���O�̕����I�ȕ��ƍ���
	int m_nHeight;
	int m_nRealWidth;				// �R���X�g���N�^�֓n���ꂽ�^�̕��ƍ���
	int m_nRealHeight;
	int m_nLength;
	UINT m_uXShift;
	UINT m_uXRound;
	UINT m_uYShift;
	UINT m_uYRound;
	LPBYTE m_pbFlag;
	CNxSprite* m_pSpriteOwner;		// ���̃I�u�W�F�N�g�����L���� CNxSprite �N���X

	CNxTrackingUpdateRegion& operator=(const CNxTrackingUpdateRegion&);
	CNxTrackingUpdateRegion(const CNxTrackingUpdateRegion&);
};

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxTrackingSprite()
 : CNxSprite(NULL /*��ɍŏ��*/)
 , m_bTracking(FALSE)
{
	m_sizeTrackingUnit.cx = CNxTrackingUpdateRegion::DefaultXUnit;
	m_sizeTrackingUnit.cy = CNxTrackingUpdateRegion::DefaultYUnit;
}

CNxTrackingSprite::~CNxTrackingSprite()
{

}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxTrackingSprite::CreateDrawRect(BOOL bForce)
// �T�v: CNxDrawRect �I�u�W�F�N�g���쐬�B���ɍ쐬�ς݂ł���A�T�C�Y�������Ȃ�Ώ��������Ȃ�
// ����: BOOL bForce  .... �����I�ɍĐ�������Ȃ�� TRUE
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CreateDrawRect(BOOL bForce)
{
	if (!bForce)
	{
		if (m_pUpdateRegion.get() != 0)
		{	// �쐬�ς݂̏ꍇ...
			if ((m_pUpdateRegion->GetWidth() - GetWidth() | m_pUpdateRegion->GetHeight() - GetHeight()) == 0)
			{	// �T�C�Y�������Ȃ�΍Đ��������ɓ��e�̏������̂�
				m_pUpdateRegion->Invalidate();
				return;
			}
		}
	}
	// �Đ���
	if (GetWidth() == 0 || GetHeight() == 0)
	{
		m_pUpdateRegion.reset();
	}
	else
	{
		m_pUpdateRegion.reset(new CNxTrackingUpdateRegion(this, GetWidth(), GetHeight(), m_sizeTrackingUnit.cx, m_sizeTrackingUnit.cy));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxTrackingSprite::Refresh(int nMaxWidthOfEnum, int nMaxHeightOfEnum, LPVOID lpContext, BOOL bForce = FALSE)
// �T�v: �ǐՂɂ���Đ����������̕`��
//		 int nMaxWidthOfEnum  ... �񋓂�����`�̍ő�̕�
//       int nMaxHeightOfEnum ... �񋓂�����`�̍ő�̍���
//		 LPVOID lpContext     ... �R���e�L�X�g(�C�ӂ̃|�C���^��)�ւ̃|�C���^
//       BOOL bForce          ... TRUE �ɂ���Ƌ����I�ɑS����ĕ`��(�S�̈悪�ĕ`��Ώ�)
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::Refresh(int nMaxWidthOfEnum, int nMaxHeightOfEnum, LPVOID lpContext, BOOL bForce) 
{
	_ASSERT(nMaxWidthOfEnum > 0 && nMaxHeightOfEnum > 0);
	
	BOOL bUpdate = FALSE;
	BOOL bDoPreUpdate = TRUE;

	if (bForce)
	{
		// �X�V��`��S�̂Ƃ��Đݒ�
		if (m_pUpdateRegion.get() != 0)
		{
			m_pUpdateRegion->Invalidate();
		}

		// bFore = true �Ȃ�� CNxSprite::PreUpdate() �͌Ăяo���Ȃ�
		bDoPreUpdate = FALSE;

		// �����X�V
		bUpdate = TRUE;
	}

	if (GetUpdateRegion(m_pUpdateRegion.get(), bDoPreUpdate) | bUpdate)
	{
		// �����J�n
		if (!RefreshBegin(lpContext))
		{
			return FALSE;
		}
		if (m_pUpdateRegion.get() == 0)
		{	// �����ǐՂ��s��Ȃ�
			nMaxWidthOfEnum = static_cast<UINT>(max(nMaxWidthOfEnum + m_sizeTrackingUnit.cx - 1, m_sizeTrackingUnit.cx));
			nMaxWidthOfEnum = static_cast<UINT>(nMaxWidthOfEnum) / m_sizeTrackingUnit.cx * m_sizeTrackingUnit.cx;
			nMaxHeightOfEnum = static_cast<UINT>(max(nMaxHeightOfEnum + m_sizeTrackingUnit.cy - 1, m_sizeTrackingUnit.cy));
			nMaxHeightOfEnum = static_cast<UINT>(nMaxHeightOfEnum) / m_sizeTrackingUnit.cy * m_sizeTrackingUnit.cy;

			RECT rect;
			for (rect.top = 0; rect.top < GetHeight(); rect.top += nMaxHeightOfEnum)
			{
				rect.bottom = min(GetHeight(), rect.top + nMaxHeightOfEnum);
				for (rect.left = 0; rect.left < GetWidth(); rect.left += nMaxWidthOfEnum)
				{
					rect.right = min(GetWidth(), rect.left + nMaxWidthOfEnum);
					RefreshRect(&rect, lpContext);
				}
			}
		}
		else
		{	// �����ǐՂ��s��
			m_pUpdateRegion->EnumRect(nMaxWidthOfEnum, nMaxHeightOfEnum, lpContext);
		}
		// �����I��
		RefreshEnd(lpContext);
	}
	m_fps.Increment();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual BOOL CNxTrackingSprite::RefreshBegin(LPVOID lpContext) const
// �T�v: �X�V��`������ꍇ�A��`�̗񋓂���钼�O�ɌĂяo�����֐�
// ����: Refresh() �֐��֓n���ꂽ lpContext ����
// �ߒl: TRUE �Ȃ�΋�`�̗񋓂��J�n�AFALSE �Ȃ�Β��~
//////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::RefreshBegin(LPVOID /*lpContext*/) const
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxTrackingSprite::RefreshEnd(LPVOID lpContext) const
// �T�v: �X�V��`������ꍇ�A�S�Ă̋�`�̗񋓂��I��������ɌĂяo�����֐�
// ����: Refresh() �֐��֓n���ꂽ lpContext ����
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::RefreshEnd(LPVOID /*lpContext*/) const
{

}


//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxTrackingSprite::SetRect(const RECT* lpRect)
// �T�v: �X�v���C�g�̋�`��ݒ�
// ����: const RECT* lpRect ... �ݒ肷���`(NULL �Ȃ�΋�ɂȂ�)
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::SetRect(const RECT* lpRect)
{
	if (!CNxSprite::SetRect(lpRect))
		return FALSE;

	// �ǐՂ��L���Ȃ�΁ACNxDrawRect �I�u�W�F�N�g�����ւ���
	if (m_bTracking)
		CreateDrawRect(FALSE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxTrackingSprite::SetTrackingUnit(int nXUnit, int nYUnit)
// �T�v: �����ǐՂ̒P�ʂ�ݒ�
// ����: int nXUnit ... �������̒P��
//       int nYUnit ... �c�����̒P��
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::SetTrackingUnit(int nXUnit, int nYUnit)
{
	// ���ꂼ�� 2^n �T�C�Y�֐؂�グ��
	UINT uXRound;
	UINT uYRound;
	CNxTrackingUpdateRegion::GetUnitShiftRound(nXUnit, NULL, &uXRound);		// ���Z�l�݂̂��擾
	CNxTrackingUpdateRegion::GetUnitShiftRound(nYUnit, NULL, &uYRound);
	m_sizeTrackingUnit.cx = uXRound + 1;
	m_sizeTrackingUnit.cy = uYRound + 1;

	// CNxDrawRect �������I�ɍĐ���
	if (m_bTracking)
		CreateDrawRect(TRUE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::EnableTracking(BOOL bEnable)
// �T�v: �����ǐՂ�L��/�����ɂ���
// ����: BOOL bEnable ... TRUE �Ȃ�΍����ǐՂ�L���ɂ���
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::EnableTracking(BOOL bEnable) 
{
	if (bEnable == m_bTracking)
		return;	// ���݂Ɠ���

	m_bTracking = bEnable;

	if (!m_bTracking)
	{	// ������
		m_pUpdateRegion.reset();
	}
	else
	{	// �L����
		CreateDrawRect(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxTrackingSprite::GetFPS()
// �T�v: FPS (Frame per second) �l��Ԃ�
// ����: �Ȃ�
// �ߒl: ���O�� FPS �� 1000�{�����l�܂��� -1
//       �A�����ČĂ΂ꂽ�ꍇ�A��1�b�Ԋu�� FPS ��Ԃ�
//       ��x�擾����Ǝ���X�V���܂ł� -1 ��Ԃ�
////////////////////////////////////////////////////////////////////////////////////

int CNxTrackingSprite::GetFPS()
{
	int result = m_fps.Get();
	m_fps.Clear();
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// 
// CNxTrackingSprite::CNxFPS
//
// FPS �v���p���[�J���N���X
//
////////////////////////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxFPS::CNxFPS()
{
	Reset();
}


////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::CNxFPS::Increment()
// �T�v: FPS �����J�E���^�𑝉�����B��ʍX�V���ɌĂ�
// ����: �Ȃ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxFPS::Increment()
{
	m_nRefreshCount++;
	DWORD dwCurrentTime = ::GetTickCount();
	if (m_dwPrevTime + 1000 <= dwCurrentTime)
	{
		m_nFPS = static_cast<int>(static_cast<DWORDLONG>(m_nRefreshCount) * 1000000i64
								  / static_cast<DWORDLONG>(dwCurrentTime - m_dwPrevTime));
		m_dwPrevTime = dwCurrentTime - (dwCurrentTime - m_dwPrevTime - 1000);
		m_nRefreshCount = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::CNxFPS::Reset()
// �T�v: �����J�E���^������������BFPS �͖��v����ԂɂȂ�
// ����: �Ȃ�
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxFPS::Reset()
{
	m_nRefreshCount = 0;				// �X�V�J�E���g��
	m_dwPrevTime = ::GetTickCount();	// �ȑO�̌v���J�n����
	m_nFPS = -1;						// FPS ����(-1 = ���v��)
}


//////////////////////////////////////////////////////////////////////////////
// public:
//	static void CNxDrawRect::GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound)
// �T�v: �����P�ʂ��V�t�g�񐔂Ɖ��Z�l�֕ϊ�
// ����: int  nUnit     ... �����P��(2^n �ł��鎖)
//       UINT* lpuShift ... �V�t�g�񐔂��󂯎�� UNIT �^�ϐ��ւ̃|�C���^(NULL��)
//       UINT* lpuRound ... ���Z�l���󂯎�� UNIT �^�ϐ��ւ̃|�C���^(NULL��)
// �ߒl: �Ȃ�
// ���l: nUnit  shift  round
//           1      0      0
//           2      1      1
//           4      2      3
//           8      3      7
//          16      4     15
//                  :
//                  :
//       nUnit �� 2^n �łȂ��ꍇ�A�K���Ɋۂ߂�
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound)
{
	UINT uShift = 0;
	UINT uRound;
	if (--nUnit != 0)
	{	// uUnit != 1
		while (!(nUnit & 0x80000000))
		{
			nUnit <<= 1;
			uShift++;
		}
		uRound = 0xffffffff >> uShift;
	}
	else
	{	// nUnit == 1
		uShift = 32;
		uRound = 0x0;
	}
	
	
	if (lpuRound != NULL)
		*lpuRound = uRound;

	if (lpuShift != NULL)
		*lpuShift = 32 - uShift;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDrawRect::CNxDrawRect(UINT uWidth, UINT uHeight)
// �T�v: CNxDrawRect �N���X�̍\�z
// ����: UINT uWidth  ... ��
//       UINT uHeight ... ����
// �ߒl: ---
///////////////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxTrackingUpdateRegion::CNxTrackingUpdateRegion(CNxSprite* pSpriteOwner, int nWidth, int nHeight, int nXUnit, int nYUnit)
{
	_ASSERTE(nWidth > 0 && nHeight > 0);
	
	// �^�̕��ƍ�����ۑ�
	m_nRealWidth = nWidth;
	m_nRealHeight = nHeight;

	m_pSpriteOwner = pSpriteOwner;

	GetUnitShiftRound(nXUnit, &m_uXShift, &m_uXRound);
	GetUnitShiftRound(nYUnit, &m_uYShift, &m_uYRound);
	
	// �u���b�N�P�ʂ̕��ƍ���
	m_nWidth = (nWidth + m_uXRound) >> m_uXShift;
	m_nHeight = (nHeight + m_uYRound) >> m_uYShift;

	// ���������m�ۂ���
	m_nLength = m_nWidth * m_nHeight;
	m_pbFlag = new BYTE[m_nLength];

	// �S�ăt���O�𗧂Ă���Ԃɂ���
	Invalidate();
}


CNxTrackingSprite::CNxTrackingUpdateRegion::~CNxTrackingUpdateRegion()
{
	delete []m_pbFlag;
}

///////////////////////////////////////////////////////////////////////////////////////
// private:
//	inline UINT CNxDrawRect::GetLineLength(const BYTE *p, int nX) const
// �T�v: �w�肳�ꂽ�s�̍X�V�t���O�̒�����Ԃ�(�s�̒[�Œ�~)
// ����: const BYTE *p  ... �����𒲂ׂ�X�V�t���O�̍s�̍��[�|�C���^
//       int nX         ... �������J�n���� x ���W
//       int nMaxWidth  ... �ő啝
// �ߒl: �t���O�̒���
// ���l: �����p�C�����C���֐�
///////////////////////////////////////////////////////////////////////////////////////

int CNxTrackingSprite::CNxTrackingUpdateRegion::GetLineLength(const BYTE* p, int nX, int nMaxWidth) const
{
	p += nX;
	const BYTE *ps = p;
	int nMax = m_nWidth - nX;
	nMax = min(nMax, nMaxWidth);

	while (--nMax >= 0 && *p != 0)
		p++;
	return p - ps;
}


///////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDrawRect::SetRect(const RECT* lpRect)
// �T�v: �w�肳�ꂽ��`�̃t���O�𗧂Ă�
// ����: const RECT* lpRect ... �t���O�𗧂Ă��`
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::AddRect(const RECT* lpRect)
{
	RECT rect;
	// �t���O���W�n�֕ϊ�
	rect.left = lpRect->left >> m_uXShift;
	rect.top = lpRect->top >> m_uYShift;
	rect.right = (lpRect->right + m_uXRound) >> m_uXShift;
	rect.bottom = (lpRect->bottom + m_uYRound) >> m_uYShift;

	// for bug
	_ASSERTE(rect.left >= 0 && rect.right <= m_nWidth);
	_ASSERTE(rect.top >= 0 && rect.bottom <= m_nHeight);

	LPBYTE lpFlag = m_pbFlag + rect.left + (rect.top * m_nWidth);
	for (int n = 0; n < rect.bottom - rect.top; n++)
	{
		memset(lpFlag, 1, rect.right - rect.left);
		lpFlag += m_nWidth;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDrawRect::EnumRect(BOOL bScanY, EnumRectProc pfnEnumRectProc, LPVOID lpContext)
// �T�v: �t���O�𒲍����ċ�`��������x�� pfnEnumRectProc �֐����Ăяo��
// ����: UINT uMaxWidth           ... �񋓂�����`�𐧌�����ő�̕�
//       EnumRectProc pfnEnumRect ... �Ăяo�����֐�
//       LPVOID lpContext         ... �R���e�L�X�g
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::EnumRect(int nMaxWidth, int nMaxHeight, LPVOID lpContext)
{
	_ASSERTE(nMaxWidth > 0 && nMaxHeight > 0);
	
	// �ŏ����ƍ��������킹��
	nMaxWidth = (nMaxWidth + m_uXRound) >> m_uXShift;
	nMaxHeight = (nMaxHeight + m_uYRound) >> m_uYShift;
	nMaxWidth = (nMaxWidth == 0) ? 1 : nMaxWidth;
	nMaxHeight = (nMaxHeight == 0) ? 1 : nMaxHeight;

	LPBYTE pbScanFlag;
	int sx, sy, nx, ny;

	pbScanFlag = m_pbFlag;
	for (sy = 0; sy < m_nHeight; pbScanFlag += m_nWidth, sy++)
	{
		// �������ɕ��f����Ă����`���E�ׂƌ�������(�C����)����
		for (sx = 0; sx < m_nWidth - 2; sx += 2)
		{
			*(pbScanFlag + sx + 1) |= *(pbScanFlag + sx + 0) & *(pbScanFlag + sx + 2);
		}

		for (sx = 0; sx < m_nWidth; sx++)
		{
			if (*(pbScanFlag + sx) != 0)
			{
				// �t���O�𔭌�����
				nx = GetLineLength(pbScanFlag, sx, nMaxWidth);					// �J�n���C���̃t���O���𐔂���
				memset(pbScanFlag + sx, 0, nx);									// �t���O�~�낷
				int nScanLimit = min(m_nHeight - sy, nMaxHeight);				// �c�����ւ̒T�� Limit

				LPBYTE pbStartLine = pbScanFlag + m_nWidth;
				for (ny = 1; ny < nScanLimit; ny++, pbStartLine += m_nWidth)
				{
					// ���[�łȂ���� ���� 0(���E)�ł��鎖���m�F����
					if (sx != 0 && *(pbStartLine + sx - 1) != 0)
						break;

					// �s�̃t���O���𐔂��ĈقȂ�Β��~
					if (GetLineLength(pbStartLine, sx, nMaxWidth) != nx)
						break;

					// �t���O���~�낷
					memset(pbStartLine + sx, 0, nx);
				}

				// (Pixel�P�ʂ�)�X�V���W�֕ϊ�
				RECT rcConv;
				rcConv.top  = sy << m_uYShift;
				rcConv.bottom  = min((sy + ny) << m_uYShift, m_nRealHeight);
				rcConv.left    = sx << m_uXShift;
				rcConv.right   = min((sx + nx) << m_uXShift, m_nRealWidth);
				sx += nx - 1;

				// �񋓊֐��Ăяo��
				static_cast<CNxTrackingSprite*>(m_pSpriteOwner)->RefreshRect(&rcConv, lpContext);
			}
		}
	}
}
