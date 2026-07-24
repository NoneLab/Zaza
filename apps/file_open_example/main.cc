#include <simplefi/efi.h>
#include <simplefi/defs.h>
#include <simplefi/types.h>

#include <simplefi/utility/guid.h>
#include <simplefi/utility/shell.h>

#include <simplefi/protocols/file.h>
#include <simplefi/protocols/simple_filesystem.h>
#include <simplefi/protocols/loaded_image.h>

#include <stdio.h>

#include <ltl/vector.h>
#include <ltl/utility.h>
#include <ltl/expected.h>

EFI_STATUS EFIAPI
EfiMain(EFI_HANDLE ImageHandle, EfiSystemTable* SystemTable)
{
    EFI_STATUS status = EFI_SUCCESS; 
    ProtocolPtr<EfiLoadedImageProtocol> loadedImage = nullptr;
    ProtocolPtr<EfiSimpleFileSystemProtocol> simpleFs = nullptr;

    Runtime::InitializeRuntime(ImageHandle, SystemTable);

    if(auto ret = GetShellParameters(); ret.has_value())
    {
        printf("Check Parameter\r\n");
        auto& param = ret.value();
        printf("Parameter 0: %ls\r\n", param[0].data());
    }
    else 
    {
        status = ret.error();
        printf("Failed to Get Shell Parameters... : 0x%llx\r\n", status);
        goto done;
    }


    if (auto ret = OpenProtocol<EfiLoadedImageProtocol>(ImageHandle); ret.has_value()) 
    {
        loadedImage = ltl::move(ret).value();
    }
    else 
    {
        status = ret.error();
        printf("Failed to Open Protocol for Loaded Image: 0x%llx\r\n", status);
        goto done;
    }

    if (auto ret = OpenProtocol<EfiSimpleFileSystemProtocol>(loadedImage->DeviceHandle); ret.has_value())
    {
        simpleFs = ltl::move(ret).value();
    }
    else
    {
        status = ret.error();
        printf("Failed to Open Protocol for Simple Filesystem: 0x%llx\r\n", status);
        goto done;
    }

    // @TODO : Working on OpenVolume

done:
    return status;
}