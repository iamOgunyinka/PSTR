#ifndef TINYDIRCPP_H
#define TINYDIRCPP_H

#include <string>
#include <memory>
#include <algorithm>
#include <errno.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

constexpr unsigned int TINYDIR_PATH_MAX = 4096;
#ifdef WIN32
constexpr unsigned int TINYDIR_PATH_EXTRA = 2;
#else
constexpr unsigned int TINYDIR_PATH_EXTRA = 0;
#endif
constexpr unsigned int TINYDIR_FILENAME_MAX = 256;

struct tinydir_file
{
	friend bool operator<(const tinydir_file &, const tinydir_file &);
	friend bool operator==(const tinydir_file &, const tinydir_file&);

	std::string path { };
	std::string name { };
	bool is_dir;
	bool is_reg;
	tinydir_file& operator=(const tinydir_file &rhs){
		this->path = rhs.path;
		this->name = rhs.name;
		this->is_dir = rhs.is_dir;
		this->is_reg = rhs.is_reg;
#ifndef WIN32
        this->s = rhs.s;
#endif
		return *this;
	}
#ifndef WIN32
    struct stat s;
#endif
};

bool operator<(const tinydir_file &a, const tinydir_file &b){
	return a.name < b.name;
}
bool operator==(const tinydir_file &a, const tinydir_file &b){
	return a.name == b.name;
}

struct tinydir_dir
{
	std::string path;
	bool has_next;
	size_t n_files;
	std::unique_ptr<tinydir_file[]> files;
	
#ifdef WIN32
    HANDLE h;
    WIN32_FIND_DATA f;
#else
    DIR *d;
    struct dirent *e;
#endif
};

static inline void bail(tinydir_dir *);
static inline int tinydir_open(tinydir_dir *, const std::string &);
static inline int tinydir_open_sorted(tinydir_dir *, const std::string &);
static inline void tinydir_close(tinydir_dir *);

static inline int tinydir_next(tinydir_dir *);
static inline int tinydir_readfile(const tinydir_dir *, tinydir_file *file);
static inline int tinydir_readfile_n(const tinydir_dir *, tinydir_file *file, size_t i);
static inline int tinydir_open_subdir_n(tinydir_dir *, size_t i);

static inline int tinydir_open(tinydir_dir *dir, const std::string &fileName)
{
	if (dir == nullptr || fileName.size() == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if ((fileName.size() + TINYDIR_PATH_EXTRA) >= TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	dir->files = nullptr;

#ifdef WIN32
	dir->h = INVALID_HANDLE_VALUE;
#else
	dir->d = nullptr;
#endif
	tinydir_close(dir);

	dir->path = fileName;
#ifdef WIN32
	*(dir).path += std::string("\\*");
	dir->h = FindFirstFile((dir->path).c_str(), &dir->f);
	dir->path[(dir->path).size() - 2] = '\0';
	if (dir->h == INVALID_HANDLE_VALUE)
#else
	dir->d = opendir(fileName.c_str());
	if (dir->d == nullptr)
#endif
	{
		errno = ENOENT;
		bail(dir);
		return -1;
	}

	/* read first file */
	dir->has_next = true;
#ifndef WIN32
	dir->e = readdir(dir->d);
	if (dir->e == nullptr)
	{
		dir->has_next = false;
	}
#endif
	return 0;
}


static inline int tinydir_open_sorted(tinydir_dir *dir, const std::string &fileName)
{
	/* Count the number of files first, to pre-allocate the files array */
	size_t n_files = 0;
	if (tinydir_open(dir, fileName) == -1)
	{
		return -1;
	}
	while (dir->has_next)
	{
		n_files++;
		if (tinydir_next(dir) == -1)
		{
			bail(dir);
			return -1;
		}
	}
	tinydir_close(dir);

	if (tinydir_open(dir, fileName) == -1)
	{
		return -1;
	}

	dir->n_files = 0;
	std::unique_ptr<tinydir_file[]> temp(new tinydir_file [n_files]);
	(dir->files).reset(temp.release());
	temp = nullptr;
	if (dir->files == nullptr)
	{
		errno = ENOMEM;
		bail(dir);
		return -1;
	}
	while (dir->has_next)
	{
		tinydir_file *p_file = nullptr;
		dir->n_files++;

		p_file = &dir->files[dir->n_files - 1];
		if (tinydir_readfile(dir, p_file) == -1)
		{
			bail(dir);
			return -1;
		}

		if (tinydir_next(dir) == -1)
		{
			bail(dir);
			return -1;
		}

		/* Just in case the number of files has changed between the first and
		second reads, terminate without writing into unallocated memory */
		if (dir->n_files == n_files)
		{
			break;
		}
	}
	
	std::sort((dir->files).get(), (dir->files).get() + n_files);
	return 0;
}

static inline void tinydir_close(tinydir_dir *dir)
{
	if (dir == nullptr)
	{
		return;
	}
	dir->path = "";
	dir->has_next = false;
	dir->n_files = false;
	if (dir->files != nullptr)
	{
		dir->files.reset();
	}
	dir->files = nullptr;
#ifdef WIN32
	if (dir->h != INVALID_HANDLE_VALUE)
	{
		FindClose(dir->h);
	}
	dir->h = INVALID_HANDLE_VALUE;
#else
	if (dir->d)
	{
		closedir(dir->d);
	}
	dir->d = nullptr;
	dir->e = nullptr;
#endif
}

static inline int tinydir_next(tinydir_dir *dir)
{
	if (dir == nullptr)
	{
		errno = EINVAL;
		return -1;
	}
	if (!dir->has_next)
	{
		errno = ENOENT;
		return -1;
	}

#ifdef WIN32
	if (FindNextFile(dir->h, &dir->f) == 0)
#else
	dir->e = readdir(dir->d);
	if (dir->e == nullptr)
#endif
	{
		dir->has_next = false;
#ifdef WIN32
		if (GetLastError() != ERROR_SUCCESS &&
			GetLastError() != ERROR_NO_MORE_FILES)
		{
			tinydir_close(dir);
			errno = EIO;
			return -1;
		}
#endif
	}
	return 0;
}

static inline void bail(tinydir_dir *dir)
{
	tinydir_close(dir);
}

static inline int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file)
{
	if (dir == nullptr || file == nullptr)
	{
		errno = EINVAL;
		return -1;
	}
#ifdef WIN32
	if (dir->h == INVALID_HANDLE_VALUE)
#else
	if (dir->e == nullptr)
#endif
	{
		errno = ENOENT;
		return -1;
	}
	if ((dir->path).size() +
		std::string(
#ifdef WIN32
			dir->f.cFileName
#else
			dir->e->d_name
#endif
		).size() + 1 + TINYDIR_PATH_EXTRA >=
		TINYDIR_PATH_MAX)
	{
		/* the path for the file will be too long */
		errno = ENAMETOOLONG;
		return -1;
	}
	if (std::string(
#ifdef WIN32
			dir->f.cFileName
#else
			dir->e->d_name
#endif
		).size() >= TINYDIR_FILENAME_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}
	
	file->path = dir->path;
	file->path += std::string("/");

	file->name += std::string(
#ifdef WIN32
		dir->f.cFileName
#else
		dir->e->d_name
#endif
	);
	file->path += file->name;
#ifndef WIN32
	if (stat((file->path).c_str(), &file->s) == -1)
	{
		return -1;
	}
#endif
	file->is_dir =
#ifdef WIN32
		!!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
		S_ISDIR(file->s.st_mode);
#endif
	file->is_reg =
#ifdef WIN32
		!!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
		(
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) &&
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
#ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM) &&
#endif
#ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA) &&
#endif
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) &&
			!(dir->f.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY));
#else
		S_ISREG(file->s.st_mode);
#endif

	return 0;
}

static inline int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i)
{
	if (dir == nullptr || file == nullptr)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files)
	{
		errno = ENOENT;
		return -1;
	}
	
	*file = (dir->files)[i];
	
	return 0;
}

static inline int tinydir_open_subdir_n(tinydir_dir *dir, size_t i)
{
	std::string path {};
	if (dir == nullptr)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files || !dir->files[i].is_dir)
	{
		errno = ENOENT;
		return -1;
	}

	path = (dir->files)[i].path;
	tinydir_close(dir);
	if (tinydir_open_sorted(dir, path) == -1)
	{
		return -1;
	}
	
	return 0;
}

#endif
