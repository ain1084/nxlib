// NxResourceFile.h: CNxResourceFile クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxFile.h"

class CNxResourceFile : public CNxFile
{
public:
	CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpResType = RT_RCDATA);
	CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpName, LPCTSTR lpResType = RT_RCDATA);
	virtual ~CNxResourceFile();
	virtual BOOL Open(LPCTSTR lpName);

private:
	class CStringId
	{
	public:
		CStringId();
		CStringId(LPCTSTR lpString);
		~CStringId();
		operator LPCTSTR() const { return m_lpString; }
		CStringId(const CStringId& stringId);
		CStringId& operator=(const CStringId& stringId);
		BOOL IsNumber() const { return HIWORD(m_lpString) == 0; }
	private:
		void Set(LPCTSTR lpString);

		void Remove();
	private:
		LPTSTR m_lpString;
	};

	CStringId m_strType;
	HINSTANCE m_hInstance;

private:
	CNxResourceFile(const CNxResourceFile&);
	CNxResourceFile& operator=(const CNxResourceFile&);
};
