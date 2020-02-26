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
#include "PartitionState.h"

VmPartitionState::VmPartitionState() {
	m_prstatus = NULL;

	m_dump_handle = NULL;
}

VmPartitionState::~VmPartitionState() {
	if (m_prstatus)
		delete[] m_prstatus;

	if (m_dump_handle)
		ReleaseSavedStateFiles(m_dump_handle);
}

bool VmPartitionState::ReadMemoryBlocks(VM_SAVED_STATE_DUMP_HANDLE dump_handle) {
	GetGuestRawSavedMemorySize(dump_handle, &m_totalramsize);
	wprintf(L"Memory Dump Size: %llu MB\n", m_totalramsize >> 20);

	MemoryBlock mem_block = { 0 };
	m_memory_blocks.clear();

	std::vector<GPA_MEMORY_CHUNK> memoryChunks;
	UINT64 pageSize = 0;
	UINT64 memoryChunkCount = ~0;
	if(GetGuestPhysicalMemoryChunks(m_dump_handle, &pageSize, memoryChunks.data(), &memoryChunkCount) != S_OK)
		memoryChunks.resize(memoryChunkCount);
        if(GetGuestPhysicalMemoryChunks(m_dump_handle, &pageSize, memoryChunks.data(), &memoryChunkCount) != S_OK) {
                wprintf(L"GetGuestPhysicalMemoryChunks error.\n");
		return false;
	}
	UINT64 i = 0;
	for (const auto& memoryChunk : memoryChunks) {
		mem_block.gpa_start_index = memoryChunk.GuestPhysicalStartPageIndex;
		mem_block.page_start_index = memoryChunk.GuestPhysicalStartPageIndex;
		mem_block.page_count = memoryChunk.PageCount;
		m_memory_blocks.push_back(mem_block);
		++i;
		//wprintf(L"Memory Chunk %I64u:\nGuestPhysicalStartPageIndex: %I64u\nPageCount: %I64u\n", i, memoryChunk.GuestPhysicalStartPageIndex, memoryChunk.PageCount);
        }
        return true;
}

// TODO:PUBLIC Entire function Hard coded to 64.
UINT64 GetRegisterValue64(VM_SAVED_STATE_DUMP_HANDLE handle, UINT32 vcpuid, REGISTER_ID_X64 id)
{
	VIRTUAL_PROCESSOR_REGISTER *vpr = new VIRTUAL_PROCESSOR_REGISTER;
	vpr->Architecture = Arch_x64;
	vpr->RegisterId_x64 = id;
	GetRegisterValue(handle, vcpuid, vpr);
	return vpr->RegisterValue;
}

bool VmPartitionState::ReadPartitionBlob(VM_SAVED_STATE_DUMP_HANDLE dump_handle) {

	m_Is64Bit = true;
	wprintf(L"64-Bit: %s\n", L"Fixed to TRUE");
    // API provides seperate architectures for seperate processors.
    // TODO: Unset this fixed attribute if we support things beyond Arch_x64
    // VIRTUAL_PROCESSOR_ARCH vpa;
    // action = GetArchitecture(dump_handle, vpx, &vpa);
    // if (vpa == Arch_x64)
    GetVpCount(dump_handle, &m_num_vps);
	wprintf(L"vm2core: %d processor%s detected.\n", m_num_vps, m_num_vps == 1 ? L"" : L"s");

	m_prstatus = new struct elf_prstatus[m_num_vps];

	for (uint32_t i = 0; i < m_num_vps; i++) {
		//wprintf(L"CPU ID: %u", i );
		m_prstatus[i].pr_reg[0] = GetRegisterValue64(dump_handle, i, X64_RegisterR15);
		m_prstatus[i].pr_reg[1] = GetRegisterValue64(dump_handle, i, X64_RegisterR14);
		m_prstatus[i].pr_reg[2] = GetRegisterValue64(dump_handle, i, X64_RegisterR13);
		m_prstatus[i].pr_reg[3] = GetRegisterValue64(dump_handle, i, X64_RegisterR12);
		m_prstatus[i].pr_reg[4] = GetRegisterValue64(dump_handle, i, X64_RegisterRbp);
		m_prstatus[i].pr_reg[5] = GetRegisterValue64(dump_handle, i, X64_RegisterRbx);
		m_prstatus[i].pr_reg[6] = GetRegisterValue64(dump_handle, i, X64_RegisterR11);
		m_prstatus[i].pr_reg[7] = GetRegisterValue64(dump_handle, i, X64_RegisterR10);
		m_prstatus[i].pr_reg[8] = GetRegisterValue64(dump_handle, i, X64_RegisterR9);
		m_prstatus[i].pr_reg[9] = GetRegisterValue64(dump_handle, i, X64_RegisterR8);
		m_prstatus[i].pr_reg[10] = GetRegisterValue64(dump_handle, i, X64_RegisterRax);
		m_prstatus[i].pr_reg[11] = GetRegisterValue64(dump_handle, i, X64_RegisterRcx);
		m_prstatus[i].pr_reg[12] = GetRegisterValue64(dump_handle, i, X64_RegisterRdx);
		m_prstatus[i].pr_reg[13] = GetRegisterValue64(dump_handle, i, X64_RegisterRsi);
		m_prstatus[i].pr_reg[14] = GetRegisterValue64(dump_handle, i, X64_RegisterRdi);
		// orig_ax not stored in state files.
		//m_prstatus[i].pr_reg[15] = GetRegisterValue64(dump_handle, i, X64_RegisterOrigAx);
		m_prstatus[i].pr_reg[15] = 0;
		m_prstatus[i].pr_reg[16] = GetRegisterValue64(dump_handle, i, X64_RegisterRip);
		m_prstatus[i].pr_reg[17] = GetRegisterValue64(dump_handle, i, X64_RegisterSegCs);
		m_prstatus[i].pr_reg[18] = GetRegisterValue64(dump_handle, i, X64_RegisterRFlags);
		m_prstatus[i].pr_reg[19] = GetRegisterValue64(dump_handle, i, X64_RegisterRsp);
		m_prstatus[i].pr_reg[20] = GetRegisterValue64(dump_handle, i, X64_RegisterSegSs);
		m_prstatus[i].pr_reg[21] = GetRegisterValue64(dump_handle, i, X64_RegisterBaseFs);
		m_prstatus[i].pr_reg[22] = GetRegisterValue64(dump_handle, i, X64_RegisterBaseGs);
		m_prstatus[i].pr_reg[23] = GetRegisterValue64(dump_handle, i, X64_RegisterSegDs);
		m_prstatus[i].pr_reg[24] = GetRegisterValue64(dump_handle, i, X64_RegisterSegEs);
		m_prstatus[i].pr_reg[25] = GetRegisterValue64(dump_handle, i, X64_RegisterSegFs);
		m_prstatus[i].pr_reg[26] = GetRegisterValue64(dump_handle, i, X64_RegisterSegGs);
    }

	return true;
}

HRESULT VmPartitionState::Load(wchar_t *vmrsFile) {
	HRESULT hr = LoadSavedStateFile(vmrsFile, &m_dump_handle);
	if (hr == S_OK)
	{
		wprintf(L"Loaded file: '%s'\n", vmrsFile);
		ReadPartitionBlob(m_dump_handle);
		ReadMemoryBlocks(m_dump_handle);
	}
	return hr;
}

HRESULT VmPartitionState::Load(wchar_t *binFile, wchar_t *vsvFile) {
	HRESULT hr;
	hr = LoadSavedStateFiles(binFile, vsvFile, &m_dump_handle);
	if( hr == S_OK) {
		wprintf(L"Loaded files: '%s' and '%s'\n", binFile, vsvFile);
		ReadPartitionBlob(m_dump_handle);
		ReadMemoryBlocks(m_dump_handle);
	}
	return hr;
}

void VmPartitionState::FillElfNote(BYTE *buf, const char *name, void *data,
				uint32_t data_len, uint32_t type) {

	struct elf64_note *note = (struct elf64_note *)buf;
	
	note->n_namesz = (uint32_t)(strlen(name) + 1);
	note->n_descsz = data_len;
	note->n_type = type;
	buf += sizeof(struct elf64_note);
	memcpy(buf, name, note->n_namesz - 1);
	buf += roundup(note->n_namesz, 4);
	memcpy(buf, data, note->n_descsz);
}

void VmPartitionState::FwriteErrCheck(const void *buffer, size_t size, size_t count, FILE *stream) {
    int ret = fwrite(buffer, size, count, stream);
    if(ret < count) {
        wprintf(L"Failed to write to file. Likely not enough disk space.\n");
        exit(EXIT_FAILURE);
    }
}

bool VmPartitionState::WriteAllRam(FILE *filep) {
    BYTE *ram_block;
    uint32_t ram_block_size = UNCOMPRESSED_BLOCK_SIZE;
    bool res = true;

    ram_block = (BYTE *)malloc(ram_block_size);
    if (!ram_block) {
        wprintf(L"Unable to alloc memory for ram block\n");
        return false;
    }

    if (m_dump_handle) {
        uint32_t bytesRead = 0;
        for (uint64_t i = 0; i < m_totalramsize; i += bytesRead) {
            ReadGuestRawSavedMemory(m_dump_handle, i, ram_block, ram_block_size, &bytesRead);
            if (bytesRead > 0) {
                wprintf(L"Writing Memory: %I64u/%I64u MB written\r", i >> 20, m_totalramsize >> 20);
                FwriteErrCheck(ram_block, sizeof(BYTE), bytesRead, filep);
            } else {
                wprintf(L"No bytes could be read from dump handle.\n");
                res = false;
                break;
            }
        }
    }

    free(ram_block);
    return res;
}

HRESULT VmPartitionState::WriteDump(wchar_t *out_file) {
    struct elf64_hdr hdr = { 0 };
    struct elf64_phdr *phdrs;
    BYTE *core_note_buf;
    BYTE *vmcoreinfo_buf;

    /*
    * Populate ELF header
    */
    memcpy(hdr.e_ident, ELFMAG, SELFMAG);
    hdr.e_ident[EI_CLASS] = ELFCLASS64;
    hdr.e_ident[EI_DATA] = ELFDATA2LSB;
    hdr.e_ident[EI_VERSION] = EV_CURRENT;
    hdr.e_ident[EI_OSABI] = ELFOSABI_NONE;
    memset(hdr.e_ident + EI_PAD, 0, EI_NIDENT - EI_PAD);

    hdr.e_type = ET_CORE;
    hdr.e_machine = (EM_X86_64);
    hdr.e_version = EV_CURRENT;
    hdr.e_phoff = sizeof(elf64_hdr);
    hdr.e_ehsize = sizeof(elf64_hdr);
    hdr.e_phentsize = sizeof(elf64_phdr);

    // TODO: Add support for 32-bit ELF hdrs and phdrs

    /*
    * Populate NOTE program header (proc info + vmcoreinfo)
    */
    uint32_t note_name_sz = NOTE_CORE_NAME_PAD_LEN;
    uint32_t note_data_sz = NOTE_CORE_DATA_PAD_LEN;
    uint32_t note_sz = sizeof(struct elf64_note) +
        note_name_sz + note_data_sz;
    uint32_t core_note_buf_sz = m_num_vps * note_sz;
    core_note_buf = (BYTE *)malloc(core_note_buf_sz);
    memset(core_note_buf, 0, core_note_buf_sz);

    for (uint32_t i = 0; i < m_num_vps; i++) {
        FillElfNote(core_note_buf + (i * note_sz),
            NOTE_CORE_NAME, &m_prstatus[i],
            sizeof(struct elf_prstatus), NT_PRSTATUS);
    }

    // Fill fake data in VMCOREINFO to keep crash tool happy
    // TODO: Is it possible to get real data from state file ?
    strcpy_s(m_vmcoreinfo, "FAKE1=IGNORE1\n");
    strcat_s(m_vmcoreinfo, "FAKE2=IGNORE2\n");
    strcat_s(m_vmcoreinfo, "FAKE3=IGNORE3\n");
    m_vmcoreinfo_len = (uint32_t)strlen(m_vmcoreinfo);

    note_name_sz = NOTE_VMCOREINFO_NAME_PAD_LEN;
    note_data_sz = roundup(m_vmcoreinfo_len, 4);
    uint32_t vmcoreinfo_note_sz = sizeof(struct elf64_note) +
        note_name_sz + note_data_sz;
    vmcoreinfo_buf = (BYTE *)malloc(vmcoreinfo_note_sz);
    memset(vmcoreinfo_buf, 0, vmcoreinfo_note_sz);

    FillElfNote(vmcoreinfo_buf, NOTE_VMCOREINFO_NAME, m_vmcoreinfo,
        m_vmcoreinfo_len, 0);

    hdr.e_phnum = (uint16_t)(m_memory_blocks.size() + 1);

    phdrs = (struct elf64_phdr *)malloc(hdr.e_phnum * sizeof(struct elf64_phdr));
    memset(phdrs, 0, hdr.e_phnum * sizeof(struct elf64_phdr));

    struct elf64_phdr *phdr = phdrs;
    uint64_t file_offset = sizeof(hdr) + hdr.e_phnum * sizeof(struct elf64_phdr);
    uint64_t total_note_sz = core_note_buf_sz + vmcoreinfo_note_sz;
    phdr->p_type = PT_NOTE;
    phdr->p_filesz = phdr->p_memsz = total_note_sz;
    phdr->p_offset = file_offset;
    file_offset += total_note_sz;
    phdr++;

    for (uint32_t i = 0; i < m_memory_blocks.size(); i++) {
        MemoryBlock *mem_block = &m_memory_blocks[i];

        phdr->p_type = PT_LOAD;
        phdr->p_filesz = phdr->p_memsz = mem_block->page_count << PAGE_SHIFT;
        phdr->p_offset = file_offset;
        phdr->p_paddr = mem_block->gpa_start_index << PAGE_SHIFT;
        phdr->p_vaddr = phdr->p_paddr + PAGE_OFFSET;
        file_offset += phdr->p_filesz;
        phdr++;
    }

    /*
    * Write everything out
    */
#if 0
    ofstream fs;
    fs.open(out_file, ios::binary | ios::out);
    fs.write((char*)&hdr, sizeof(hdr));
    fs.write((char*)phdrs, hdr.e_phnum * sizeof(struct elf64_phdr));
    fs.write((char*)core_note_buf, core_note_buf_sz);
    fs.write((char*)vmcoreinfo_buf, vmcoreinfo_note_sz);
    WriteAllRam(&fs);
    fs.close();
#else
    FILE* filep;
    errno_t err = _wfopen_s(&filep, out_file, L"wb");
    FwriteErrCheck(&hdr, sizeof(hdr), 1, filep);
    FwriteErrCheck(phdrs, sizeof(struct elf64_phdr), hdr.e_phnum, filep);
    FwriteErrCheck(core_note_buf, sizeof(BYTE), core_note_buf_sz, filep);
    FwriteErrCheck(vmcoreinfo_buf, sizeof(BYTE), vmcoreinfo_note_sz, filep);
    WriteAllRam(filep);
    fclose(filep);
#endif

    wprintf(L"Linux vmcore dump successfully written to: %s\n", out_file);

    free(vmcoreinfo_buf);
    free(core_note_buf);
    free(phdrs);
    return S_OK;
}
