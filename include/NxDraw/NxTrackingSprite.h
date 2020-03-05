// NxTrackingSprite.h: CNxTrackingSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxSprite �h���N���X�B�����ǐՋ@�\�t���ŏ�ʃX�v���C�g�N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include "NxSprite.h"

class __declspec(novtable) CNxTrackingSprite : public CNxSprite  
{
public:
	CNxTrackingSprite();
	virtual ~CNxTrackingSprite();

	BOOL Refresh(int nWidthOfEnumt, int nHeightOfEnum, LPVOID lpContext, BOOL bForce);
	void EnableTracking(BOOL bEnable);
	BOOL IsTrackingEnabled() const;
	void GetTrackingUnit(LPSIZE lpSize) const;
	int GetFPS();
	void ResetFPS();

	// CNxTrackingSprite virtual function
	virtual BOOL SetTrackingUnit(int nXUnit, int nYUnit);

	// CNxSprite override
	virtual BOOL SetRect(const RECT* lpRect);

protected:
	virtual void RefreshRect(const RECT* lpRect, LPVOID lpContext) const = 0;
	virtual BOOL RefreshBegin(LPVOID lpContext) const;
	virtual void RefreshEnd(LPVOID lpContext) const;

private:
	void CreateDrawRect(BOOL bForce);

	BOOL		 m_bTracking;				// �����ǐՂ��s���Ȃ�� TRUE
	SIZE		 m_sizeTrackingUnit;		// �����ǐՒP��

	class CNxTrackingUpdateRegion;
	std::auto_ptr<CNxTrackingUpdateRegion> m_pUpdateRegion;

	// FPS (frames per second) �v���p
	class CNxFPS
	{
	public:
		CNxFPS();
		void Increment();
		int Get() const { return m_nFPS; }
		void Clear() { m_nFPS = -1; }
		void Reset();

	protected:
		int m_nRefreshCount;	// �`���
		DWORD m_dwPrevTime;		// �v���J�n TickCount
		int m_nFPS;				// fps * 1000
	private:
		CNxFPS(const CNxFPS&);
		CNxFPS& operator=(const CNxFPS&);
	} m_fps;
private:
	CNxTrackingSprite(const CNxTrackingSprite&);
	CNxTrackingSprite& operator=(const CNxTrackingSprite&);
};

inline BOOL CNxTrackingSprite::IsTrackingEnabled() const {
	return m_bTracking; }

inline void CNxTrackingSprite::GetTrackingUnit(LPSIZE lpSize) const {
	*lpSize = m_sizeTrackingUnit; }

inline void CNxTrackingSprite::ResetFPS() {
	m_fps.Reset(); }
