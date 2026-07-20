#include "simplefi/protocols/loaded_image.h"
#include <simplefi/efi.h>
#include <simplefi/defs.h>
#include <simplefi/types.h>
#include <simplefi/utility/guid.h>

#include <stdio.h>

EFI_STATUS EFIAPI
EfiMain(EFI_HANDLE ImageHandle, EfiSystemTable* SystemTable)
{
    Runtime::InitializeRuntime(ImageHandle, SystemTable);
    printf("Hello, newlib on UEFI\r\n");

    EfiLoadedImageProtocol* loadedImage = nullptr; 
    auto status = OpenProtocol<EfiLoadedImageProtocol>(ImageHandle, loadedImage);
    printf("Check Open Status: %llu\r\n", status);

    return EFI_SUCCESS;
}