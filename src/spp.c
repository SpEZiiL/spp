/*
 * Script Preprocessor.
 * Copyright (C) 2019 Michael Federczuk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// src/spp.c
/*
 * spp.h implementation.
 * Source file for the core spp functions.
 *
 * Since: v0.1.0 2019-05-25
 * LastEdit: 2019-06-02
 */

#include <spp/spp.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <spp/utils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct spp_stat {
	bool ignore;
	cstr pwd;
};

int init_spp_stat(spp_stat* stat) {
	if(stat == NULL) return;

	stat->ignore = false;
	cstr pwd = getenv("PWD");

	errno = 0;
	stat->pwd = malloc(strlen(pwd) + 1);
	if(stat->pwd == NULL || errno == ENOMEM) return 1;

	strcpy(stat->pwd, pwd);
}

int spp_stat_set_pwd(spp_stat* stat, cstr pwd) {
	if(stat == NULL) return;

	if(strlen(stat->pwd) != strlen(pwd)) {
		errno = 0;
		cstr tmp = realloc(stat->pwd, CHAR_SIZE * (strlen(pwd) + 1));
		if(tmp == NULL || errno == ENOMEM) return 1;
		stat->pwd = pwd;
	}

	strcpy(stat->pwd, pwd);
}

void deinit_spp_stat(spp_stat* stat) {
	if(stat == NULL) return;

	stat->ignore = false;
	free(stat->pwd);
	stat->pwd = NULL;
}

#define STEP_PRE_DIR 0 // whitespace before directive
#define STEP_DIR_CMD 1 // directive command
#define STEP_DIR_PRE_ARG 2 // whitespace before directive command argument
#define STEP_DIR_ARG 3 // directive command argument

#define CMD_BUF_GROW 1.25
#define ARG_BUF_GROW 1.25

int checkln(cstr line, cstr* cmd, cstr* arg) {
	if(cmd == NULL || arg == NULL) return SPP_CHECKLN_ERR_INV_ARGS;
	if(*cmd != NULL || *arg != NULL) return SPP_CHECKLN_ERR_INV_ARGS;

	size_t lcmd_size = 16, lcmd_len = 0;
	errno = 0;
	cstr lcmd = malloc(CHAR_SIZE * lcmd_size);
	if(lcmd == NULL || errno == ENOMEM) return SPP_CHECKLN_ERR_NO_MEM;

	size_t larg_size = 16, larg_len = 0;
	errno = 0;
	cstr larg = malloc(CHAR_SIZE * larg_size);
	if(larg == NULL || errno == ENOMEM) {
		free(lcmd);
		return SPP_CHECKLN_ERR_NO_MEM;
	}

	unsigned char step = STEP_PRE_DIR;
	for(size_t i = 0, l = strlen(line); i < l; ++i) {
		switch(step) {
		case STEP_PRE_DIR: {
			if(isws(line[i])) { // pre directive whitespace
				continue;
			} else if(line[i] == '#') { // directive begins
				step = STEP_DIR_CMD;
			} else { // line is no directive
				free(lcmd);
				free(larg);
				return SPP_CHECKLN_NO_DIR;
			}
			break;
		}
		case STEP_DIR_CMD: {
			if(isws(line[i])) { // command name ends
				step = STEP_DIR_PRE_ARG;
			} else { // command name continues
				if(lcmd_len + (CHAR_SIZE * 2) > lcmd_size) { // grow buffer
					errno = 0;
					cstr tmp = realloc(lcmd,
					                   CHAR_SIZE * (lcmd_size *= CMD_BUF_GROW));
					if(tmp == NULL || errno == ENOMEM) {
						free(lcmd);
						free(larg);
						return SPP_CHECKLN_ERR_NO_MEM;
					}
					lcmd = tmp;
				}
				lcmd[lcmd_len] = line[i];
				++lcmd_len;
			}
			break;
		}
		case STEP_DIR_PRE_ARG: {
			if(isws(line[i])) { // pre argument whitespace
				continue;
			} else { // argument begins
				// no need to check if the buffer is too small, this is always
				// the first character i.e.: it will always have space
				larg[0] = line[i];
				larg_len = 1;
				step = STEP_DIR_ARG;
			}
			break;
		}
		case STEP_DIR_ARG: {
			if(i + 1 < l && line[i] != '\n') {
				// only add char if it isn't at the end of the string and it
				// isn't a newline character
				if(larg_len + (CHAR_SIZE * 2) > larg_size) { // grow buffer
					errno = 0;
					cstr tmp = realloc(larg,
					                   CHAR_SIZE * (larg_size *= ARG_BUF_GROW));
					if(tmp == NULL || errno == ENOMEM) {
						free(lcmd);
						free(larg);
						return SPP_CHECKLN_ERR_NO_MEM;
					}
					larg = tmp;
				}
				larg[larg_len] = line[i];
				++larg_len;
			}
			break;
		}
		}
	}

	if(step == STEP_PRE_DIR) {
		free(lcmd);
		free(larg);
		return SPP_CHECKLN_NO_DIR;
	}

	if(lcmd_len + CHAR_SIZE < lcmd_size) { // shorten buffer
		errno = 0;
		cstr tmp = realloc(lcmd, CHAR_SIZE * (lcmd_size = lcmd_len + CHAR_SIZE));
		if(tmp == NULL || errno == ENOMEM) {
			free(lcmd);
			free(larg);
			return SPP_CHECKLN_ERR_NO_MEM;
		}
		lcmd = tmp;
	}

	lcmd[lcmd_len] = '\0';

	if(larg_len + CHAR_SIZE < larg_size) { // shorten buffer
		errno = 0;
		cstr tmp = realloc(larg, CHAR_SIZE * (larg_size = larg_len + CHAR_SIZE));
		if(tmp == NULL || errno == ENOMEM) {
			free(lcmd);
			free(larg);
			return SPP_CHECKLN_ERR_NO_MEM;
		}
		larg = tmp;
	}

	larg[larg_len] = '\0';

	*cmd = lcmd;
	*arg = larg;
	return SPP_CHECKLN_DIR;
}

#undef STEP_PRE_DIR
#undef STEP_DIR_CMD
#undef STEP_DIR_PRE_ARG
#undef STEP_DIR_ARG

#undef CMD_BUF_GROW
#undef ARG_BUF_GROW

int processln(cstr line, FILE* out, spp_stat* spp_statbuf) {
	if(out == NULL || stat == NULL) return SPP_PROCESSLN_ERR_INV_ARGS;

	cstr cmd = NULL, arg = NULL;
	switch(checkln(line, &cmd, &arg)) {
	case SPP_CHECKLN_DIR: {
		bool valid_cmd = true;

		if(strcmp(cmd, "include") == 0) {
			if(strncmp(arg, "/", 1) != 0) { // if arg does not begin with /
				size_t pwdlen = strlen(spp_statbuf->pwd),
				       arglen = strlen(arg);

				errno = 0;
				cstr tmp = malloc(CHAR_SIZE * (pwdlen + 1 + arglen) + CHAR_SIZE);
				if(tmp == NULL || errno == ENOMEM) {
					free(cmd);
					free(arg);
					return SPP_PROCESSLN_ERR_NO_MEM;
				}

				size_t i = 0;
				for(; i < pwdlen; ++i) {
					tmp[i] = spp_statbuf->pwd[i];
				}
				tmp[i] = '/';
				++i;
				for(size_t j = 0; j < arglen; ++j, ++i) {
					tmp[i] = arg[j];
				}
				tmp[i] = '\0';

				free(arg);
				arg = tmp;
			}

			struct stat statbuf;
			errno = 0;
			if(stat(arg, &statbuf) != 0) {
				switch(errno) {
				case ENAMETOOLONG:
				case ENOENT:
				case ENOTDIR: {
					// when the path name is too long or the path doesn't exist
					// we ignore the directive
					valid_cmd = false;
					break;
				}
				default: {
					int tmp = errno;
					free(cmd);
					free(arg);
					errno = tmp;
					return SPP_PROCESSLN_ERR_STAT;
				}
				}
			} else {
				errno = 0;
				FILE* file = fopen(arg, "r");
				if(file == NULL) {
					int tmp = errno;
					free(cmd);
					free(arg);
					errno = tmp;
					return SPP_PROCESSLN_ERR_FOPEN;
				}

				for(int ch = fgetc(file);
				        ch != EOF; ch = fgetc(file)) {

					fputc(ch, out);
				}

				fclose(file);
			}
		} else if(strcmp(cmd, "import") == 0) {
			// TODO directive import
		} else if(strcmp(cmd, "ignore") == 0) {
			spp_statbuf->ignore = true;
		} else if(strcmp(cmd, "end-ignore") == 0) {
			spp_statbuf->ignore = false;
		} else { // no such command; see as non-directive line
			valid_cmd = false;
		}

		free(cmd);
		free(arg);

		// fall through if the command does not exist; output the line normally
		if(valid_cmd) break;
	}
	case SPP_CHECKLN_NO_DIR: {
		if(!spp_statbuf->ignore) {
			errno = 0;
			int exc = fputs(line, out);
			if(exc < 0 || exc == EOF) {
				return SPP_PROCESSLN_ERR_FPUTS;
			}
		}
		break;
	}
	case SPP_CHECKLN_ERR_NO_MEM:
		return SPP_PROCESSLN_ERR_NO_MEM;
	}

	return SPP_PROCESSLN_SUCCESS;
}

#define LINE_BUF_GROW 1.25
#define LINE_BUF_INIT_SIZE 64

int process(FILE* in, FILE* out, cstr pwd) {
	if(in == NULL || out == NULL) return SPP_PROCESS_ERR_INV_ARGS;

	size_t size = LINE_BUF_INIT_SIZE, len = 0;
	errno = 0;
	cstr line = malloc(CHAR_SIZE * size);
	if(line == NULL || errno == ENOMEM) return SPP_PROCESS_ERR_NO_MEM;

	spp_stat stat;
	if(init_spp_stat(&stat) == 1) {
		free(line);
		return SPP_PROCESS_ERR_NO_MEM;
	}

	if(pwd != NULL) spp_stat_set_pwd(&stat, pwd);

	bool read = true;
	for(int ch = fgetc(in);
	        read; ch = fgetc(in)) {

		if(ch != EOF) {
			// build line
			if(len + (CHAR_SIZE * 2) > size) { // grow buffer
				errno = 0;
				cstr tmp = realloc(line, CHAR_SIZE * (size *= LINE_BUF_GROW));
				if(tmp == NULL || errno == ENOMEM) {
					free(line);
					return SPP_PROCESS_ERR_NO_MEM;
				}
				line = tmp;
			}
			line[len] = ch;
			++len;
		} else {
			read = false;
		}

		if(ch == EOF || ch == '\n') {
			// finish up building line
			if(len + CHAR_SIZE < size) { // shorten buffer
				errno = 0;
				cstr tmp = realloc(line, CHAR_SIZE * (size = len + CHAR_SIZE));
				if(tmp == NULL || errno == ENOMEM) {
					free(line);
					return SPP_PROCESS_ERR_NO_MEM;
				}
				line = tmp;
			}
			line[len] = '\0';

			// work with line
			errno = 0;
			switch(processln(line, out, &stat)) {
			case SPP_PROCESSLN_ERR_NO_MEM: {
				free(line);
				return SPP_PROCESS_ERR_NO_MEM;
			}
			case SPP_PROCESSLN_ERR_STAT: {
				int tmp = errno;
				free(line);
				errno = tmp;
				return SPP_PROCESS_ERR_STAT;
			}
			case SPP_PROCESSLN_ERR_FPUTS: {
				free(line);
				return SPP_PROCESS_ERR_FPUTS;
			}
			case SPP_PROCESSLN_ERR_FOPEN: {
				int tmp = errno;
				free(line);
				errno = tmp;
				return SPP_PROCESS_ERR_FOPEN;
			}
			}

			// reset line
			if(size != LINE_BUF_INIT_SIZE) {
				// shrink buffer to init size if it was grown
				errno = 0;
				cstr tmp = realloc(line, CHAR_SIZE * (size = LINE_BUF_INIT_SIZE));
				if(tmp == NULL || errno == ENOMEM) {
					free(line);
					return SPP_PROCESS_ERR_NO_MEM;
				}
				line = tmp;
			}
			len = 0;
		}
	} // end for

	free(line);
	deinit_spp_stat(&stat);
	return SPP_PROCESS_SUCCESS;
}

#undef LINE_BUF_GROW
#undef LINE_BUF_INIT_SIZE
