#include "pch.h"
#include "MFCStrUtil.h"
// эти функции только для проектов, написанных под MFC

/*
CString &NormalizePath(CString &strPath)
{
	if (!strPath.IsEmpty())
	{
		strPath.TrimRight(_T('\\'));
		strPath += _T('\\');
	}

	return strPath;
}

CString GetCurrentPath()
{
	CString strDrive;
	CString strPath;
	CString pModuleName;
	BOOL res = ::GetModuleFileName(AfxGetInstanceHandle(), pModuleName.GetBufferSetLength(_MAX_PATH), _MAX_PATH);
	pModuleName.ReleaseBuffer();

	if (res)
	{
		return GetFilePath(pModuleName);
	}

	return _T("");
}

CString GetFilePath(const CString &strFile)
{
	CString strDrive;
	CString strPath;
	_tsplitpath_s(strFile.GetString(), strDrive.GetBufferSetLength(_MAX_DIR), _MAX_DIR, strPath.GetBufferSetLength(_MAX_PATH), _MAX_PATH, nullptr, 0, nullptr, 0);
	strDrive.ReleaseBuffer();
	strPath.ReleaseBuffer();
	return NormalizePath(strDrive.Trim() + strPath.Trim());
}

CString GetFileTitle(const CString &strFile)
{
	CString strName;
	_tsplitpath_s(strFile.GetString(), nullptr, 0, nullptr, 0, strName.GetBufferSetLength(_MAX_FNAME), _MAX_FNAME, nullptr, 0);
	strName.ReleaseBuffer();
	return strName.Trim();
}

CString GetFileName(const CString &strFile)
{
	CString strName;
	CString strExt;
	_tsplitpath_s(strFile.GetString(), nullptr, 0, nullptr, 0, strName.GetBufferSetLength(_MAX_FNAME), _MAX_FNAME, strExt.GetBufferSetLength(_MAX_EXT), _MAX_EXT);
	strName.ReleaseBuffer();
	strExt.ReleaseBuffer();
	return strName.Trim() + strExt.Trim();
}

CString GetFileExt(const CString &strFile)
{
	CString strExt;
	_tsplitpath_s(strFile.GetString(), nullptr, 0, nullptr, 0, nullptr, 0, strExt.GetBufferSetLength(_MAX_EXT), _MAX_EXT);
	strExt.ReleaseBuffer();
	return strExt.Trim();
}
*/
