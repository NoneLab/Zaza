#include <pepe/pe.h>

#include <stdio.h>
#include <string.h>

namespace
{
    
using ConstructorFn = VOID (*)(VOID);

void RunConstructors(
    VOID* ctorAddr,
    UINT32 size
)
{
    if ((size % sizeof(ConstructorFn)))
    {
        printf("You Get Wrong Address... %p %u\r\n", ctorAddr, size);
        return;
    }

    ltl::span<ConstructorFn> ctorView { reinterpret_cast<ConstructorFn*>(ctorAddr), (size / sizeof(ConstructorFn)) };

    for (auto ctor : ctorView)
    {
        if (!ctor)
            continue;

        ctor();
    }
}

}  // namespace

namespace pepe
{

EFI_STATUS PeImage::Parse()
{
    EFI_STATUS status = EFI_SUCCESS;

    if (!info)
        return EFI_NOT_READY;

    optionalHeader.Pe32 = reinterpret_cast<EFI_IMAGE_NT_HEADERS32*>(info->Data);

    auto sectionTableAddr = reinterpret_cast<EFI_IMAGE_SECTION_HEADER*>(info->Data + info->Header.SizeOfOptionalHeader);
    sectionTable = decltype(sectionTable) { sectionTableAddr, info->Header.NumberOfSections };

    return status;
}

ltl::expected<EFI_IMAGE_SECTION_HEADER, EFI_STATUS> PeImage::GetSection(const CHAR8* sectionName)
{
    for (auto& section : GetSectionTable())
    {
        auto ret = memcmp(section.Name, sectionName, strlen(sectionName));
        if (!ret)
            return section;
    }

    return ltl::unexpected(EFI_UNSUPPORTED);
}

PeImage& PECoff::GetPE()
{
    if (!peImage)
    {
        peImage = ltl::make_unique<PeImage>(reinterpret_cast<unsigned char*>(dosImage->Get()) + dosImage->GetPeImageOffset());
    }

    return *peImage;
}

EFI_STATUS PECoff::CallConstructors()
{
    EFI_STATUS status = EFI_SUCCESS;

    if (!GetDos().IsDos() || !GetPE().IsPE())
    {
        return EFI_INVALID_PARAMETER;
    }

    status = GetPE().Parse();
    if (status != EFI_SUCCESS)
        return status;

    if (auto ret = GetPE().GetSection(".ctors"); ret.has_value())
    {
        auto sectionOffset = ret->VirtualAddress;
        auto sectionSize = ret->Misc.VirtualSize;
        if (sectionSize == 0)
            sectionSize = ret->SizeOfRawData;

        RunConstructors(reinterpret_cast<UINT8*>(GetDos().Get()) + sectionOffset, sectionSize);
    }
    else 
    {
        printf("Failed to find .ctors\r\n");
    }
    
    return status;
}

}  // namespace pepe