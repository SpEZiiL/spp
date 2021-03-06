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
// src/utils.c
/*
 * utils.h implementation.
 * Source file for utility functions.
 *
 * Since: v0.1.0 2019-05-24
 * LastEdit: 2019-05-31
 */

#include <spp/utils.h>

bool isws(char ch) {
	return ch == '\t' || ch == '\n'
	       || ch == '\v' || ch == '\f'
	       || ch == '\r'  || ch == ' ';
}
