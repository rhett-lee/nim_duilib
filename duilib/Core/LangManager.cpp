#include "LangManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FileUtil.h"

namespace ui 
{
LangManager::LangManager()
{
};

LangManager::~LangManager()
{
	m_stringTable.clear();
};

bool LangManager::LoadStringTable(const std::wstring& strFilePath)
{
	m_stringTable.clear();
	std::vector<uint8_t> fileData;
	FileUtil::ReadFileData(strFilePath, fileData);
	ASSERT(!fileData.empty());
	if (fileData.empty()) {
		return false;
	}
	return LoadStringTable(fileData);
}

bool LangManager::LoadStringTable(const std::vector<uint8_t>& fileData)
{
	std::vector<std::wstring> string_list;
	if (fileData.empty()) {
		return false;
	}
	size_t bomSize = 0;
	if ((fileData.size() >= 3)   &&
		(fileData.at(0) == 0xEF) &&
		(fileData.at(1) == 0xBB) &&
		(fileData.at(2) == 0xBF) ) {
		//跳过UTF8的BOM头
		bomSize = 3;
	}
	std::string fragment((const char*)fileData.data() + bomSize, fileData.size() - bomSize);
	StringHelper::ReplaceAll("\r\n", "\n", fragment);
	StringHelper::ReplaceAll("\r", "\n", fragment);
	fragment.append("\n");
	std::string src;
	std::wstring string_resourse;
	for (const auto& it : fragment)	{
		if (it == '\0' || it == '\n') {
			if (!src.empty()) {
				string_resourse.clear();
				StringHelper::MBCSToUnicode(src.c_str(), string_resourse, CP_UTF8);
				string_resourse = StringHelper::Trim(string_resourse);
				if (!string_resourse.empty()) {
					string_list.push_back(string_resourse);
				}
				src.clear();
			}
			continue;
		}
		src.push_back(it);
	}
	AnalyzeStringTable(string_list);
	return true;
}

void LangManager::ClearStringTable()
{
	m_stringTable.clear();
}

bool LangManager::AnalyzeStringTable(const std::vector<std::wstring>& list)
{
	int	nCount = (int)list.size();
	if (nCount <= 0) {
		return false;
	}
	std::wstring id;
	std::wstring strResource;
	for (int i = 0; i < nCount; ++i) {
		const std::wstring& strSrc = list[i];
		if (strSrc.empty() || strSrc.at(0) == L';') {
			//注释以";"开头
			continue;
		}
		size_t pos = strSrc.find(L'=');
		if (pos == std::wstring::npos) {
			//无分隔符，忽略
			continue;
		}

		id = strSrc.substr(0, pos);
		if (!id.empty()) {
			id = StringHelper::Trim(id);
		}
		if ((pos + 1) < strSrc.size()) {
			strResource = strSrc.substr(pos + 1);
			strResource = StringHelper::Trim(strResource);
			//将\n和\r替换为真实的换行符、回车符
			StringHelper::ReplaceAll(L"\\r", L"\r", strResource);
			StringHelper::ReplaceAll(L"\\n", L"\n", strResource);
		}
		else {
			strResource.clear();
		}
		if (!id.empty()) {
			m_stringTable[id] = strResource;
		}
	}
	return true;
}

std::wstring LangManager::GetStringViaID(const std::wstring& id)
{
	std::wstring text;
	if (id.empty()) {
		return text;
	}
	auto it = m_stringTable.find(id);
	if (it == m_stringTable.end()) {
		ASSERT(!"MultiLang::GetStringViaID failed!");
		return text;
	}
	else {
		text = it->second;
	}
	return text;
}

}//namespace ui 
