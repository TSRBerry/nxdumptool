/*
 * nso.h
 *
 * Copyright (c) 2020, DarkMatterCore <pabloacurielz@gmail.com>.
 *
 * This file is part of nxdumptool (https://github.com/DarkMatterCore/nxdumptool).
 *
 * nxdumptool is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * nxdumptool is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef __NSO_H__
#define __NSO_H__

#define NSO_HEADER_MAGIC    0x4E534F30  /* "NSO0". */
#define NSO_MOD_MAGIC       0x4D4F4430  /* "MOD0". */

typedef enum {
    NsoFlags_TextCompress = BIT(0), ///< Determines if .text segment is LZ4-compressed.
    NsoFlags_RoCompress   = BIT(1), ///< Determines if .rodata segment is LZ4-compressed.
    NsoFlags_DataCompress = BIT(2), ///< Determines if .data segment is LZ4-compressed.
    NsoFlags_TextHash     = BIT(3), ///< Determines if .text segment hash must be checked during load.
    NsoFlags_RoHash       = BIT(4), ///< Determines if .rodata segment hash must be checked during load.
    NsoFlags_DataHash     = BIT(5)  ///< Determines if .data segment hash must be checked during load.
} NsoFlags;

typedef struct {
    u32 file_offset;    ///< NSO segment offset.
    u32 memory_offset;  ///< Memory segment offset.
    u32 size;           ///< Decompressed segment size.
} NsoSegmentHeader;

typedef struct {
    u32 offset; ///< Relative to the .rodata segment start.
    u32 size;
} NsoSectionHeader;

/// This is the start of every NSO.
/// This is always followed by a NsoModuleName block.
typedef struct {
    u32 magic;                                  ///< "NSO0".
    u32 version;                                ///< Always set to 0.
    u8 reserved_1[0x4];
    u32 flags;                                  ///< NsoFlags.
    NsoSegmentHeader text_segment_header;
    u32 module_name_offset;                     ///< NsoModuleName block offset.
    NsoSegmentHeader rodata_segment_header;
    u32 module_name_size;                       ///< NsoModuleName block size.
    NsoSegmentHeader data_segment_header;
    u32 bss_size;
    u8 module_id[0x20];                         ///< Also known as build ID.
    u32 text_file_size;                         ///< .text segment compressed size (if NsoFlags_TextCompress is enabled).
    u32 rodata_file_size;                       ///< .rodata segment compressed size (if NsoFlags_RoCompress is enabled).
    u32 data_file_size;                         ///< .data segment compressed size (if NsoFlags_DataCompress is enabled).
    u8 reserved_2[0x1C];
    NsoSectionHeader api_info_section_header;
    NsoSectionHeader dynstr_section_header;
    NsoSectionHeader dynsym_section_header;
    u8 text_segment_hash[0x20];                 ///< Decompressed .text segment SHA-256 checksum.
    u8 rodata_segment_hash[0x20];               ///< Decompressed .rodata segment SHA-256 checksum.
    u8 data_segment_hash[0x20];                 ///< Decompressed .data segment SHA-256 checksum.
} NsoHeader;

/// Usually placed right after NsoHeader, but it's actual offset may vary.
/// If the 'module_name_size' member from NsoHeader is greater than 1 and the 'name_length' element from NsoModuleName is greater than 0, 'name' will hold the module name.
typedef struct {
    u8 name_length;
    char name[];
} NsoModuleName;

/// Placed at the very start of the decompressed .text segment.
typedef struct {
    u32 entry_point;
    u32 mod_offset;     ///< NsoModHeader block offset (relative to the start of this header). Almost always set to 0x8 (the size of this struct).
} NsoModStart;

/// This is essentially a replacement for the PT_DYNAMIC program header available in ELF binaries.
/// All offsets are signed 32-bit values relative to the start of this header.
/// This is usually placed at the start of the decompressed .text segment, right after a NsoModStart block.
/// However, in some NSOs, it can instead be placed at the start of the uncompressed .rodata segment, right after its NsoModuleInfo block.
/// In these cases, the 'mod_offset' value from the NsoModStart block will point to an offset within the .rodata segment.
typedef struct  {
    u32 magic;                      ///< "MOD0".
    s32 dynamic_offset;
    s32 bss_start_offset;
    s32 bss_end_offset;
    s32 eh_frame_hdr_start_offset;
    s32 eh_frame_hdr_end_offset;
    s32 module_object_offset;       ///< Typically equal to .bss base.
} NsoModHeader;

/// Placed at the start of the decompressed .rodata segment + 0x4.
/// If the 'name_length' element is greater than 0, 'name' will hold the module name.
typedef struct {
    u32 name_length;
    char name[];
} NsoModuleInfo;

#endif /* __NSO_H__ */