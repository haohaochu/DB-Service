
#pragma once

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();

private:
	CMapStringToString m_param;
	CMapStringToString m_count;

public:
	bool Parse(char*, int);
	bool Parse(char*, int, char*, int ary = -1);

	CString operator[](CString& strToken)
	{
		return m_param[strToken];
	}

	CString GetValue(CString& strToken)
	{
		return m_param[strToken];
	}

	int GetCount(CString& strToken)
	{
		return atoi((char*)m_count[strToken].GetBuffer(0));
	}
};