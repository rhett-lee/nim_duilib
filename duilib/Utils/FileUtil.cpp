#include "FileUtil.h"
#include <stdio.h>

namespace ui
{

bool FileUtil::ReadFileData(const std::wstring& filePath, std::vector<uint8_t>& fileData)
{
	bool isReadOk = false;
	FILE* f = nullptr;
	errno_t ret = ::_wfopen_s(&f, filePath.c_str(), L"rb");
	if ((ret == 0) && (f != nullptr)) {
		isReadOk = true;
		::fseek(f, 0, SEEK_END);
		int fileSize = ::ftell(f);
		::fseek(f, 0, SEEK_SET);
		if (fileSize > 0) {
			fileData.resize((size_t)fileSize);
			size_t readLen = ::fread(fileData.data(), 1, fileData.size(), f);
			ASSERT_UNUSED_VARIABLE(readLen == fileData.size());
			if (readLen != fileData.size()) {
				fileData.clear();
				isReadOk = false;
			}
		}
		::fclose(f);
	}
	return isReadOk;
}

}//namespace ui
