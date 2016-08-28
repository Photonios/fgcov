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
#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <string>

#include <unistd.h>

/*!
 * \brief Standard location of the GCOV executable.
 */
constexpr auto GCOV_LOCATION = "/usr/bin/gcov";

/*!
 * \brief Location that the shared libary that is embedded
 *		  is extracted to.
 */
constexpr auto LIBFGCOV_LOCATION = "/tmp/libfgcov.so";

/*!
 * \brief Embedded contents of the 'libfgcov.so' file.
 */
extern uint8_t LIBFGCOV_DATA[] asm("_binary_libfgcov_so_start");

/*!
 * \brief Size of the embedded contents of the 'libfgcov.so' file.
 */
extern uint8_t LIBFGCOV_DATA_SIZE[] asm("_binary_libfgcov_so_size");

int main(int argc, char **argv)
{
	/* assure gcov is installed on this machine */
	if(access(GCOV_LOCATION, F_OK) == -1) {
		fprintf(stderr, "error: gcov not installed in /usr/bin/gcov\n");
		return 1;
	}

	/* remove the existing libfgcov.so file from /tmp if it exists */
	if(access(LIBFGCOV_LOCATION, F_OK) != -1 && unlink(LIBFGCOV_LOCATION) == -1) {
		fprintf(stderr, "error: failed to remove %s\n", LIBFGCOV_LOCATION);
		return 1;
	}

	/* attempt to write the contents of the embedded libfgcov.so
	 * file to /tmp */
	FILE *libfgcov_fp = fopen(LIBFGCOV_LOCATION, "w");
	if(!libfgcov_fp) {
		fprintf(stderr, "error: failed to open %s, error %u\n", LIBFGCOV_LOCATION, errno);
		return 1;
	}

	/* get the total size of the embdded file */
	size_t libfgcov_size = (size_t)((void *)LIBFGCOV_DATA_SIZE);

	/* dump the contents of the embedded file to disk */
	if(fwrite(LIBFGCOV_DATA, 1, libfgcov_size, libfgcov_fp) != libfgcov_size) {
		fprintf(stderr, "error: failed to write %s, error %u\n", LIBFGCOV_LOCATION, errno);
		fclose(libfgcov_fp);
		return 1;
	}

	fclose(libfgcov_fp);

	/* bundle arguments into a single string to pass onto
	 * the real gcov */
	std::string args;
	for(int i = 1; i < argc; ++i) {
		args += "\"";
		args += argv[i];
		args += "\"";
		args += " ";
	}

	/* build up the gcov command */
	std::string command = "LD_PRELOAD=/tmp/libfgcov.so /usr/bin/gcov";
	if (argc > 1) {
		command += " ";
		command += args;
	}

	/* run gcov with the injected library */
	system(command.c_str());
	return 0;
}
