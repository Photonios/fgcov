/**
* The MIT License (MIT)
* Copyright (c) <2016> <Swen Kooij @ photonios@outlook.com>
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdio>
#include <dlfcn.h>

#include <string>
#include <type_traits>

#include "md5.h"

/*!
 * \brief Prototype for the \see fopen function.
 */
using fopen_func = std::add_pointer<FILE *(const char *, const char *)>::type;

/*!
 * \brief Calls the original \see fopen.
 *
 * \param _filename The file to open.
 * \param _mode		The mode with which to open the file.
 */
FILE * fopen_original(const char *_filename, const char *_mode)
{
	return reinterpret_cast<fopen_func>(dlsym(RTLD_NEXT, "fopen"))(_filename, _mode);
}

/*!
 * \brief Checks whether the specified string ends with the
 *		  specified suffix.
 *
 * \param str		The string to check for the suffix.
 * \param suffix	The suffix to check for in \paramref str.
 *
 * \returns True when the specified string ends with the specified
 *			suffix and false when it does not.
 */
bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/*!
 * \brief Overriden \see fopen.
 *
 * Checks whether a *.gcov file is being written
 * and turns the specified filename into a md5 hash
 * and writes that instead to avoid crossing the
 * system's filename length limit.
 *
 * If the filename does _not_ end in '.gcov', then
 * this functions does nothing but forward the
 * call to the original \see fopen.
 *
 * If the filename _does_ end in '.gcov' then the
 * entire filename, including the '.gcov' extension
 * is hashed using the MD5 algorithm and the new
 * filename will become: '[md5 hash].gcov'.
 *
 * Not only will this pass on the new hashed file
 * name to the original \see fopen function, it
 * will also overwrite the specified string so
 * that the gcov program takes notice of the
 * modified filename. This involves casting the
 * const-ness away since the filename is passed
 * as 'const char *'. It involves taking care of
 * potentially resizing the allocated memory to
 * make the new filename fit. In case the specified
 * string buffer is larger then needed, then
 * \see realloc() will take care of freeing the
 * remaining memory (as guarenteed by the standard).
 *
 * \param _filename The file that is being written.
 * \param _mode     The mode to open the file with.
 *
 * \returns A file descriptor or nullptr if opening
 *			the file failed.
 */
FILE * fopen(const char *_filename, const char *_mode)
{
	std::string filename(_filename);
	std::string mode(_mode);

	/* make sure we don't do anything with normal
	 * calls to fopen, forward to the actual fopen */
	if(!has_suffix(filename, ".gcov") || mode != "w") {
		return fopen_original(_filename, _mode);
	}

	/* hash the filename with md5 and add the .gcov extension */
	std::string filename_hash = md5(filename) + ".gcov";

	/* remove const-ness and assure the specified buffer
	 * is large enough to hold the new filename */
	char *unsafe_filename = const_cast<char *>(_filename);
	unsafe_filename = (char *) realloc(unsafe_filename, filename_hash.size() + 1);

	/* copy in the new filename and make sure the end is denoted
	 * by a null terminator */
	memcpy(unsafe_filename, filename_hash.c_str(), filename_hash.size());
	unsafe_filename[filename_hash.size()] = 0;

	/* call the original fopen */
	return fopen_original(filename_hash.c_str(), _mode);
}
