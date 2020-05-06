
#include "Index.h"
#include "stdlib.h"
#include "stdio.h"
#include "io.h"
#include <stack>


//This file adds and removes files in the database by using an indexing algorithm 
Indexing::Indexing() 
{
	m_maxIndices = 1024;
	m_fileIndices.reserve(m_maxIndices); 
	FileTypeInfo tmp;
	tmp.type = 1;
	tmp.actionRequired = FILE_NEED_NOACTION;
	m_supportedFileTypes[std::wstring(L".txt")] = tmp;
	tmp.type = 2;
	tmp.actionRequired = FILE_NEED_NOACTION;
	m_supportedFileTypes[std::wstring(L".xml")] = tmp;
	tmp.type = 3;
	tmp.actionRequired = FILE_NEED_SCAN;
	m_supportedFileTypes[std::wstring(L".exe")] = tmp;
	tmp.type = 4;
	tmp.actionRequired = FILE_NEED_BACKUP;
	m_supportedFileTypes[std::wstring(L".doc")] = tmp;
	tmp.type = 5;
	tmp.actionRequired = FILE_NEED_NOACTION;
	m_supportedFileTypes[std::wstring(L".xls")] = tmp;
	tmp.type = 6;
	tmp.actionRequired = FILE_NEED_NOACTION;
	m_supportedFileTypes[std::wstring(L".ppt")] = tmp;
}


bool Indexing::maximumindex(const unsigned int& maxIndices)
{
	if (maxIndices < m_fileIndices.max_size())  
	{
		m_maxIndices = maxIndices;
		return true;
	}

	else
		return false;
}

bool Indexing::addfile(const wchar_t* ext, const File_Type_Action& _actionRequired)
{
	if (!ext) return false;

	std::wstring s(ext);
	DynamicFileType::iterator it = m_supportedFileTypes.find(s);

	if (it == m_supportedFileTypes.end())
	{
		FileTypeInfo tmp;
		tmp.type = m_supportedFileTypes.size() + 1;
		tmp.actionRequired = FILE_NEED_NOACTION;
		m_supportedFileTypes[std::wstring(L".txt")] = FileTypeInfo(1, FILE_NEED_NOACTION);
	}

	else
	{
		it->second.actionRequired = _actionRequired;
	}

	return true;
}



bool Indexing::directory(const wchar_t* name)
{
	if (!name) return false;

	if (m_fileIndices.size() >= m_maxIndices) return false;

	std::stack<std::wstring> pathNameST;                                  
	std::wstring initialPath = name;
	pathNameST.push(initialPath);
	std::wstring curS, search, fname;
	FileDef f;
	DynamicFileType::const_iterator it;

	do
	{
		curS = pathNameST.top();
		pathNameST.pop();
		search = curS + L"\\*";
		_wfinddata_t fd;
		intptr_t handle = _wfindfirst(search.c_str(), &fd);

		if (handle != -1)
		{
			do
			{
				// if it's a sub-dir and not ".." and "." string, then push to stack
				if (fd.attrib & _A_SUBDIR && wcscmp(fd.name, L"..") && wcscmp(fd.name, L"."))
				{
					fname = curS + L"\\" + fd.name;
					pathNameST.push(fname);
				} /* else if it's a file (except system files) then */

				else if (!(fd.attrib & _A_SYSTEM) && !(fd.attrib & _A_SUBDIR))
				{
					f.size = fd.size;
					fname = curS + L"\\" + fd.name;
					_wsplitpath(fname.c_str(), f.drive, f.path, f.name, f.ext);
					it = m_supportedFileTypes.find(std::wstring(f.ext));

					if (it != m_supportedFileTypes.end())
					{
						f.type = it->second.type;
						m_fileIndices.push_back(f);

						// if fileIndex.size is equal to the maximum number of indices, the process should be terminated.
						if (m_fileIndices.size() == m_maxIndices)
						{
							_findclose(handle);
							return true;
						}
					}
				}
			} while (_wfindnext(handle, &fd) == 0);
		}

		else
		{
			int err;
			_get_errno(&err);

			switch (err)
			{
			case EINVAL:
			case ENOMEM:
				_findclose(handle);
				return false;

			case ENOENT:
				break;

			default:
				_findclose(handle);
				return false;
			}
		}

		_findclose(handle);
	} while (!pathNameST.empty());

	return true;
}

bool Indexing::ff(const wchar_t ext, unsigned int& fileNo, FileDef fd)
{
	if (!ext || !fd) return false;

	DynamicFileType::const_iterator it;
	bool ret = false;
	it = m_supportedFileTypes.find(std::wstring(ext));

	if (it == m_supportedFileTypes.end()) return false;

	FileType type = it->second.type;

	for (unsigned int i = 0; i < (unsigned int)m_fileIndices.size(); i++)
		if (m_fileIndices[i].type == type)
		{
			*fd = m_fileIndices[i];
			fileNo = i;
			ret = true;
			break;
		}

	return ret;
}

bool Indexing::nf(unsigned int& fileNo, FileDef* fd)
{
	if (!fd) return false;

	bool ret = false;

	for (unsigned int i = fileNo + 1; i < (unsigned int)m_fileIndices.size(); i++)
		if (m_fileIndices[i].type == m_fileIndices[fileNo].type)
		{
			*fd = m_fileIndices[i];
			fileNo = i;
			ret = true;
			break;
		}

	return ret;
}

void Indexing::listsupportedfiles()
{
	DynamicFileType::const_iterator it;
	wprintf(L"%10s%15s\r\n", L"File Type", L"Extension");

	for (it = m_supportedFileTypes.begin(); it != m_supportedFileTypes.end(); it++)
	{
		wprintf(L"%5u%15s\r\n", it->second.type, it->first.c_str());
	}
}

void Indexing::listFiles(const wchar_t* ext)
{
	FileDef fd;
	unsigned int fileNo;

	if (ff(ext, fileNo, &fd))
	{
		wprintf(L"%s files:\n", ext);
		wprintf(L"%90s%15s\r\n", L"NAME", L"SIZE");

		do
		{
			wprintf(L"%90s%15i\r\n", fd.name, fd.size);
		} while (nf(fileNo, &fd));
	}
}

bool Indexing::needsAction(const File_Type_Action& action, const FileDef* fd)
{
	if (!fd) return false;

	DynamicFileType::const_iterator it = m_supportedFileTypes.find(std::wstring(fd->ext));

	if (it != m_supportedFileTypes.end())
	{
		return static_cast<bool>(it->second.actionRequired & action);
	}

	else
		return false;
}


void Indexing::_testIndexer()
{
	bool ret1 = maximumindex(3024);
	bool ret2 = addfile(L".pdf");
	supportfile();
	bool ret3 = directory(L"D:\\");
	bool ret4 = directory(L"C:\\");
	//directory("C:\\test directory1" );
	//directory("C:\\test directory2" );
	listFiles(L".exe");
	listFiles(L".txt");
	listFiles(L".xml");
	listFiles(L".bat");
}