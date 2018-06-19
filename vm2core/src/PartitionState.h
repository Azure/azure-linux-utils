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
#ifndef PARTITION_STATE_H
#define PARTITION_STATE_H

#include <sdkddkver.h>
#include <windows.h>
#include <vector>
#include <tchar.h>
#include "elf.h"
#include <VmSavedStateDump.h>

#define PAGE_OFFSET 0xffff880000000000
#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_PER_BLOCK 256

#define HV_START_GPA_GAP	(0x00000000000F8000)
#define HV_END_GPA_GAP		(0x00000000000FFFFF)
#define GPA_GAP_SIZE		(0x0000000000008000)

#define UNCOMPRESSED_BLOCK_SIZE 10 * (PAGE_SIZE * PAGE_PER_BLOCK)

#if __linux__
#define roundup(x, y) (				\
{						\
	const typeof(y) __y = y;		\
	(((x) + (__y - 1)) / __y) * __y;	\
}						\
)
#elif _WIN32
static inline uint32_t roundup(uint32_t x, uint32_t y) {
	return ((x + y - 1) / y) * y;
}
#endif

#define NOTE_CORE_NAME "CORE"
#define NOTE_CORE_NAME_LEN 4
#define NOTE_CORE_NAME_PAD_LEN (roundup(NOTE_CORE_NAME_LEN + 1, 4))
#define NOTE_CORE_DATA_PAD_LEN (roundup(sizeof(struct elf_prstatus), 4))

#define NOTE_VMCOREINFO_NAME "VMCOREINFO"
#define NOTE_VMCOREINFO_NAME_LEN 10
#define NOTE_VMCOREINFO_NAME_PAD_LEN (roundup(NOTE_VMCOREINFO_NAME_LEN + 1, 4))
#define NOTE_VMCOREINFO_DATA_MAX_LEN 4096

using namespace std;

class VmPartitionState {

public:
	VmPartitionState();
	~VmPartitionState();

	HRESULT Load(wchar_t *binFile, wchar_t *vsvFile);
	HRESULT Load(wchar_t *vmrsFile);
    bool ReadPartitionBlob(VM_SAVED_STATE_DUMP_HANDLE dump_handle);
    bool ReadMemoryBlocks(VM_SAVED_STATE_DUMP_HANDLE dump_handle);
    HRESULT WriteDump(wchar_t *out_file);
    void FillElfNote(BYTE *buf, const char *name, void *data,
        uint32_t data_len, uint32_t type);
    bool WriteAllRam(FILE *filep);
    
private:
	VM_SAVED_STATE_DUMP_HANDLE m_dump_handle;

	char m_vmcoreinfo[NOTE_VMCOREINFO_DATA_MAX_LEN];
	uint32_t m_vmcoreinfo_len;
	struct elf_prstatus *m_prstatus;
	uint32_t m_num_vps;
	uint32_t m_vm_version;
	uint64_t m_totalramsize;
    BOOL m_Is64Bit;

    typedef struct MemoryBlock {
        uint32_t version;
        bool is_hot_add;
        uint64_t page_count;
        uint64_t page_start_index;
        uint64_t gpa_start_index;
        char virtual_node;
        uint64_t block_id;
    } MemoryBlock;

    vector<MemoryBlock> m_memory_blocks;
};

#endif
