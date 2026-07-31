#include <simplefi/efi.h>
#include <simplefi/defs.h>
#include <simplefi/types.h>

#include <simplefi/utility/file.h>
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
    SimpleFile root;
    SimpleFile file;
    ltl::vector<ltl::vector<CHAR16>> param;

    Runtime::InitializeRuntime(ImageHandle, SystemTable);

    if(auto ret = GetShellParameters(); ret.has_value())
    {
        if (ret->size() < 2)
        {
            printf("You forget to give file to command\r\n");
            return EFI_SUCCESS;
        }

        param = ltl::move(ret).value();
    }
    else 
    {
        status = ret.error();
        printf("Failed to Get Shell Parameters... : 0x%llx\r\n", status);
        return status;
    }


    if (auto ret = OpenProtocol<EfiLoadedImageProtocol>(ImageHandle); ret.has_value()) 
    {
        loadedImage = ltl::move(ret).value();
    }
    else 
    {
        status = ret.error();
        printf("Failed to Open Protocol for Loaded Image: 0x%llx\r\n", status);
        return status;
    }

    if (auto ret = OpenProtocol<EfiSimpleFileSystemProtocol>(loadedImage->DeviceHandle); ret.has_value())
    {
        simpleFs = ltl::move(ret).value();
    }
    else
    {
        status = ret.error();
        printf("Failed to Open Protocol for Simple Filesystem: 0x%llx\r\n", status);
        return status;
    }

    // @TODO : Working on OpenVolume
    auto rootVolume = SimpleFS(ltl::move(simpleFs));

    if (auto ret = rootVolume.OpenVolume(); ret.has_value())
    {
        root = ltl::move(ret).value();
    }
    else
    {
        status = ret.error();
        printf("Failed to Open Root Volume: 0x%llx\r\n", status);
        return status;
    }

    if (auto ret = root.Open(
        param[1], 
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 
        0
    ); ret.has_value())
    {
        printf("Success to Open File\r\n");
        file = ltl::move(ret).value();
    }
    else 
    {
        status = ret.error();
        printf("Failed to Open File: 0x%llx\r\n", status);
        return status;
    }

    ltl::vector<CHAR8> testStr { 'A', 'B', 'C', 'D', 'E', '\0' };
    if (auto ret = file.Write(testStr); ret.has_value())
    {
        printf("Success to Write Data\r\n");
    }
    else 
    {
        status = ret.error();
        printf("Failed to Write File: 0x%llx\r\n", status);
        return status;
    }

    return status;
}