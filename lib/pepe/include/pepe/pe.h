// SPDX-License-Identifier: MIT

/**
 * @file pe.h
 * @brief Dos & PE Image parser for calling constructor & destructor
 *
 * @author Henry Shin <henry.shin@thundersoft.com>
 * @date 2026-08-06
 *
 * @copyright Copyright (c) 2026 Henry Shin
 */

#ifndef __PEPE_PE_H
#define __PEPE_PE_H

#include <simplefi/types.h>
#include <third_party/edk2/MiniBase.h>
#include <third_party/edk2/IndustryStandard/PeImage.h>

#include <ltl/span.h>
#include <ltl/memory.h>
#include <ltl/expected.h>
#include <ltl/optional.h>

namespace pepe
{

struct PEImageInfo 
{
    UINT32 Signature;
    EFI_IMAGE_FILE_HEADER Header;
    
    // Optional Header Section
    UINT8 Data[1];
} __attribute__((packed));

class DosImage
{
    EFI_IMAGE_DOS_HEADER* base;

public:
    explicit DosImage(VOID* base) : base(reinterpret_cast<decltype(this->base)>(base)) {}
    ~DosImage() = default;

    EFI_IMAGE_DOS_HEADER* Get() { return base; }
    bool IsDos() { return (base->e_magic == EFI_IMAGE_DOS_SIGNATURE); }
    const UINT32& GetPeImageOffset() { return base->e_lfanew; }
};

class PeImage
{
    PEImageInfo* info = nullptr;

    EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION optionalHeader;

    // PE Image's section table
    ltl::span<EFI_IMAGE_SECTION_HEADER> sectionTable;

public:
    explicit PeImage(VOID* base) : info(reinterpret_cast<decltype(info)>(base)) {}
    ~PeImage() = default;

    bool IsPE() { return (info->Signature == EFI_IMAGE_NT_SIGNATURE); }
    EFI_STATUS Parse();
    ltl::span<EFI_IMAGE_SECTION_HEADER>& GetSectionTable() { return sectionTable; }
    ltl::expected<EFI_IMAGE_SECTION_HEADER, EFI_STATUS> GetSection(const CHAR8* sectionName);
};

class PECoff
{
    ltl::unique_ptr<DosImage> dosImage;
    ltl::unique_ptr<PeImage> peImage = nullptr;

public:
    explicit PECoff(VOID* base) : dosImage(new DosImage(base)) {}
    ~PECoff() = default;

    DosImage& GetDos() { return *dosImage; }
    PeImage& GetPE();

    EFI_STATUS CallConstructors();
};

} // namespace pepe

#endif