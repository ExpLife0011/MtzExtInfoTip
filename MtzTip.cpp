// MtzTip.cpp : Implementation of CMtzTip
/*
The MIT License (MIT)

Copyright (c) 2017 Robert Oeffner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "stdafx.h"
#include "MtzExt.h"
#include "MtzTip.h"
#include <strsafe.h>
#include <math.h>
#include <string.h>
#include "ccp4\cmtzlib.h"

/////////////////////////////////////////////////////////////////////////////
// CMtzTip

HRESULT CMtzTip::GetInfoTip(DWORD dwFlags, LPWSTR* ppwszTip)
{
	USES_CONVERSION;

	// dwFlags is currently unused.

	// At this point the shell is requiring the tip
	// for a certain Mtz file whose name is now 
	// stored in the m_szFile member of the 
	// IPersistFileImpl class. 

	// Opens the Mtz file and returns size and colors
	CComBSTR bstrInfo;
	GetStructurefactorInfo((CComBSTR *)&bstrInfo);
    
	*ppwszTip = (WCHAR*) m_pAlloc->Alloc( 
		(bstrInfo.Length() +1) * sizeof(WCHAR));
	if (*ppwszTip)
		wcscpy(*ppwszTip, (WCHAR*)(BSTR)bstrInfo);
	
	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// Private Members
// Present MTZ header file information as an infotip when hovering the mouse over a file on Windows

HRESULT CMtzTip::GetStructurefactorInfo(CComBSTR *p)
{
	p->Empty();

	FILETIME ftCreate, ftAccess, ftWrite;
	const int nchr = 2000;
	SYSTEMTIME stUTC, stLocal;
	TCHAR sizestr[20], datestr[30], szTemp[nchr], colsstr[nchr];
	char clabs[50][31], ctyps[50][3], colstr[nchr];
	int csetid[50];

	HANDLE hFile = CreateFile(m_szFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, 0, NULL);
	if (hFile == NULL)
	{
		p->LoadString(IDS_TEXT_UNABLETOREAD);
		return S_OK;
	}	
	DWORD bsize = GetFileSize(hFile,  NULL);
	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return FALSE;

	BOOL bFlag = CloseHandle(hFile);

	// Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	// Build a string showing the date and time.
	StringCchPrintf(datestr, 30, "%02d/%02d/%d  %02d:%02d",
		stLocal.wMonth, stLocal.wDay, stLocal.wYear,
		stLocal.wHour, stLocal.wMinute);
	// format file size string
	TCHAR suffixes[7][4] = {{"B\0"}, {"KB\0"}, {"MB\0"}, {"GB\0"}, {"TB\0"}, {"PB\0"}};
	int s = 0;
	double size = (double)bsize;
	while (size >= 1000)
	{
		s++;
		size /= 1000;
	}
	StringCchPrintf(sizestr, 20, "%2.1f %s", size, suffixes[s]);

	CMtz::MTZ *mtzdata = CMtz::MtzGet(m_szFile, 0);
// get name of the columns and the letter indicating their type
	int ncol = CMtz::MtzListColumn(mtzdata, clabs, ctyps, csetid);
	int i = 0;
	colstr[0] = 0;
	while (i < ncol)
	{
		char tmpstr[34];
		_strlwr_s(ctyps[i],3); // lowercase column type to improve readability
		if (i < (ncol-1))
			sprintf(tmpstr, "%s(%s), ", clabs[i], ctyps[i]);
		else
			sprintf(tmpstr, "%s(%s)", clabs[i], ctyps[i]);

		strcat(colstr, tmpstr);
		i++;
	}
	wsprintf(colsstr, colstr);
	
	// Format the infotip string 
	HRESULT hr = StringCchPrintf(szTemp, nchr, 
		_T("Type: Structure Factors\nReflections: %d\nPoint group: %s\n"
		"Space group: %s\nResolution: %2.3f - %2.3f�\n"
		"Cell: %2.2f� %2.2f� %2.2f�  %2.2f� %2.2f� %2.2f�\n"
		"Columns: %s\n"
		"Size: %s\nDate modified: %s"), 
		mtzdata->nref, 
		mtzdata->mtzsymm.pgname, 
		mtzdata->mtzsymm.spcgrpname,
		1.0/sqrt(mtzdata->xtal[0]->resmax), 
		1.0/sqrt(mtzdata->xtal[0]->resmin),
		mtzdata->xtal[0]->cell[0],
		mtzdata->xtal[0]->cell[1],
		mtzdata->xtal[0]->cell[2],
		mtzdata->xtal[0]->cell[3],
		mtzdata->xtal[0]->cell[4],
		mtzdata->xtal[0]->cell[5],
		colsstr,
		sizestr,
		datestr);  

	if (FAILED(hr))
		return hr;

	p->Append(szTemp); 
	MtzFree(mtzdata);
	return S_OK;
}

