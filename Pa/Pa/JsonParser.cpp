
#include "stdafx.h"
#include "JsonParser.h"

CJsonParser::CJsonParser()
{
	m_param.RemoveAll();
}

CJsonParser::~CJsonParser()
{
}

bool CJsonParser::Parse(char* src, int len)
{
	return Parse(src, len, "root");
}

bool CJsonParser::Parse(char* src, int len, char* tag, int ary)
{
	char node[100]={0};
	sprintf_s(node, "%s", tag);

	int jsonarray = ary;

	char* jsonstart = NULL;
	char* jsonend = NULL;
	int jsonform = 0;

	bool param = false;
	bool value = false;
	char* paramstart = NULL;
	char* paramend = NULL;
	char* valuestart = NULL;
	char* valueend = NULL;

	bool split = false;

	if (len == 0)
		return FALSE;

	for (int i=0; i<len; i++)
	{
		switch (src[i])
		{
		case '{':
			{
				// Json Start
				if (jsonstart == NULL)
				{	
					jsonstart = &src[i]+1;
					jsonform ++;
				}
				else
					return false;

				for (int j=i+1; j<len; j++)
				{
					if (src[j] == '{')
						jsonform ++;

					if (src[j] == '}')
						jsonform --;
					
					if (jsonform == 0)
					{
						jsonend = &src[j]-1;
						break;
					}
				}
				if (jsonend == NULL)
					return false;
				else
				{
					i = jsonend-src+1;
					if (jsonarray >= 0)
					{
						char tmpnode[100] = {0};
						CString str("");
						str.AppendFormat(_T("%d"), jsonarray+1);
						m_count[CString(node)] = str;
						
						sprintf_s(tmpnode, "%s.%d", node, jsonarray);
						jsonarray++;

						Parse(jsonstart, jsonend-jsonstart+1, tmpnode, jsonarray);
					}
					else
						Parse(jsonstart, jsonend-jsonstart+1, node, jsonarray);

					param = false;
					value = false;
					split = false;
					jsonstart = NULL;
					jsonend = NULL;
					paramstart = NULL;
					paramend = NULL;
					valuestart = NULL;
					valueend = NULL;
				}
			}
			break;
		case '[':
			{
				// Json Array Start
				if (jsonarray < 0)
					jsonarray = 0;
				else
					return false;
			}
			break;
		case '\"':
			{
				// Json String Start/End
				if (!param && !value)
				{
					param = true;
					paramstart = &src[i]+1;

					for (int j=i+1; j<len; j++)
					{
						if (src[j] == '\"')
						{
							paramend = &src[j]-1;
							i=j;
							break;
						}
					}

					if (paramstart!=NULL && paramend!=NULL)
					{	
						char p[100]={0};
						memcpy(p, paramstart, paramend-paramstart+1);
						sprintf_s(node, "%s.%s", tag, p);
					}
					else
						return false;
				}
				else if (param && !value)
				{
					value = true;
					valuestart = &src[i]+1;

					for (int j=i+1; j<len; j++)
					{
						if (src[j] == '\"')
						{
							valueend = &src[j]-1;
							i=j;
							break;
						}
					}

					if (param && value && valuestart!=NULL && valueend!=NULL && split)
					{	
						char v[100]={0};
						memcpy(v, valuestart, valueend-valuestart+1);
						m_param[CString(node)] = CString(v);
						
						param = false;
						value = false;
						paramstart = NULL;
						paramend = NULL;
						valuestart = NULL;
						valueend = NULL;
						split = false;
					}
					else
						return false;
				}
				else
					return false;
			}
			break;
		case ']':
			{
				// Json Array End
				if (jsonarray >= 0)
					jsonarray = -1;
				else
					return false;
			}
			break;
		case '}':
			{
				// Json End
				if (jsonstart != NULL)
					continue;
				else
					return false;
			}
			break;
		case ':':
			{
				// Json String Split
				if (paramstart!=NULL && paramend!=NULL && split==false)
					split = true;
				else
					continue;
			}
			break;
		case '\\':
			{
				if (param || value)
					continue;

				i=i+1;
				continue;
			}
			break;
		default:
			{
				// Json Character
				continue;
			}
			break;
		}
	}
	
	return true;
}
