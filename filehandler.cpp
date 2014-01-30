#include "./filehandler.h"


FileHandler::FileHandler(const std::string &_filename): filename(_filename)
{
	while(filename[filename.size()-1] == stringToChar(SLASH)){
		filename.erase((filename.rbegin() + 1).base(), filename.end());
	}
}

void FileHandler::recurseAll(){
	do
	{
		std::string newpath { filename };
		if(!directories.empty()){
			newpath = directories.top();
			directories.pop();
		}
		open(newpath, allFiles, directories);
	} while(!directories.empty());
}

void FileHandler::getSingle(){
	open(filename, allFiles, directories);
}

void FileHandler::open(std::string source, std::vector<std::string> &vec, std::stack<std::string> &stack)
{
	tinydir_dir dir;
	std::string c(source), stack_string;
	unsigned int i = 0;
	tinydir_open_sorted(&dir, c.c_str());
	for (; i < dir.n_files; i++)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);

		if (file.is_dir)
		{
			stack_string = source + std::string(SLASH) + file.name;
			if(!(stack_string[stack_string.size() - 1] == '.')){
				stack.push(stack_string);
			}
		} else {
			auto found = stack_string.find("..");
			if(found != std::string::npos){
					stack_string.erase(found, 2);
			}
			vec.push_back(stack_string + file.name);
		}
	}
	tinydir_close(&dir);
}

std::vector<std::string> FileHandler::getFiles(){
	return allFiles;
}

FileHandler::size_type FileHandler::getFileNumbers(){
	return allFiles.size();
}

char FileHandler::stringToChar(std::string s){
	char c = s[0];
	return c;
}

std::vector<std::string> FileHandler::getExtension(const std::string &ext)
{
	std::vector<std::string> temp(allFiles.cbegin(), allFiles.cend());
	auto partition = std::stable_partition(temp.begin(), temp.end(),
			[=](std::string s)->bool{ return s.find(ext) != std::string::npos; } );
	temp.erase(partition, temp.end());
	return temp;
}
