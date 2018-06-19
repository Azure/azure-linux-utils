/*
 * Copyright (c) 2018, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Alex Ng 		<alexng@microsoft.com>
 *   Sean Spratt	<seansp@microsoft.com>
 */
#include "stdafx.h"
#include <windows.h>
#include <winerror.h>
#include <comdef.h>

#include "PartitionState.h"
#include "elf.h"


void PrintHelp() {
	wprintf(L"Usage:\n");
	wprintf(L"On WS2012R2 or earlier: vm2core (BIN file) (VSV file) (Output file)\n");
	wprintf(L"On WS2016 or later: vm2core (VMRS file) (Output file)\n");
}

int wmain(int argc, wchar_t *argv[]) {
    wchar_t *output_file_name;
	VmPartitionState partition_state;
	HRESULT action;
	
	if (argc < 3 || argc > 4) {
		PrintHelp();
		return ERROR_INVALID_PARAMETER;
	}
	if (argc == 4) {
		action = partition_state.Load(argv[1], argv[2]);
		output_file_name = argv[3];
	}
	if (argc == 3) {
		action = partition_state.Load(argv[1]);
		output_file_name = argv[2];
	}
	if (action == S_OK)
	{
		action = partition_state.WriteDump(output_file_name);
	}
	if( action != S_OK )
	{
		_com_error err(action);
		printf(err.ErrorMessage());
	}
	return HRESULT_CODE(action);
}
