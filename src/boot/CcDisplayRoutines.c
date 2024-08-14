#include "include/FrameWork.h"
#include "include/MachineInfo.h"
#include "include/Wrapper.h"
#include "include/ReadFile.h"

extern EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *gSimpleFileSystem;
extern EFI_FILE_PROTOCOL *RootProtocol;
extern EFI_SYSTEM_TABLE *gSystemTable;

STATIC UINT32 Horizontal;
STATIC UINT32 Vertical;
STATIC UINT32 PixelsPerScanLine;

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;


STATIC UINT32 swap_endian(UINT32 value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}




VOID GetPNGSize(
	IN CHAR16* PngPath,
	IN OUT UINT32* Width,
	IN OUT UINT32* Height
)
{
	VOID* Buffer;
	char* FileBuffer;
	UINTN FileSize;

	UINT32 SizeOfIHDR;
	UINT32 PNGWidth;
	UINT32 PNGHeight;

	ReadFileToBuffer(PngPath, &Buffer, &FileSize);
	FileBuffer = (char*)Buffer;

	// is support 8 bit data on transmission system?
	if (FileBuffer[0] != ((char)0x89))
	{
		goto ParsingFailed;
	}
	// PNG signature
	if (FileBuffer[1] != 'P' || FileBuffer[2] != 'N' || FileBuffer[3] != 'G')
	{
		goto ParsingFailed;
	}
	// CRLF in dos environment
	if (FileBuffer[4] != ((char)0x0D) || FileBuffer[5] != ((char)0x0A))
	{
		goto ParsingFailed;
	}
	// prevent file to display EOF in dos environment
	if (FileBuffer[6] != ((char)0x1A))
	{
		goto ParsingFailed;
	}
	// LF in unix environment
	if (FileBuffer[7] != ((char)0x0A))
	{
		goto ParsingFailed;
	}
	FileBuffer += 8;
	SizeOfIHDR = *(UINT32*)(FileBuffer);
	SizeOfIHDR = swap_endian(SizeOfIHDR);

	FileBuffer += 4;

	// RDHI: 0x52444849
	if (*(UINT32*)(FileBuffer) != 0x52444849)
	{
		goto ParsingFailed;
	}
	FileBuffer += 4;
	PNGWidth = *(UINT32*)(FileBuffer);
	*Width = swap_endian(PNGWidth);

	FileBuffer += 4;
	PNGHeight = *(UINT32*)(FileBuffer);
	*Height = swap_endian(PNGHeight);

	FreePool(Buffer);
	goto ParsingSuccessful;

ParsingFailed:

	Print(L"Png Parsing Failed!\n\r");
	FreePool(Buffer);
	UEFI_PANIC;

ParsingSuccessful:

}

#ifndef _DEBUG
#define LOADING_ICO_PATH 											L"icon\\loading.ico"
#define LOADING_OFFSET												30
#define LOGO_ICO_PATH 												L"icon\\logo.ico"


EFI_STATUS DisplayImage(
		EFI_IMAGE_OUTPUT *ImageOutput,
		UINTN XOfUpperLeftHand,
		UINTN YOfUpperLeftHand)
{
	EFI_STATUS Status;
	UINTN Row, Column;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Pixel;

	for (Row = 0; Row < ImageOutput->Height; Row++)
	{
		for (Column = 0; Column < ImageOutput->Width; Column++)
		{
			Pixel = &ImageOutput->Image.Bitmap[Row * ImageOutput->Width + Column];
			if (Pixel->Reserved != 0)
			{
				Status = gGraphicsOutput->Blt(
						gGraphicsOutput,
						Pixel,
						EfiBltVideoFill,
						0,
						0,
						Column + XOfUpperLeftHand,
						Row + YOfUpperLeftHand,
						1,
						1,
						0);
				if (EFI_ERROR(Status))
				{
					return Status;
				}
			}

		}
	}

	return Status;
}

#define LOGO1_WIDTH							256
#define LOGO1_HEIGHT						256

#define LOGO2_WIDTH							300
#define LOGO2_HEIGHT						80

VOID DisplayLoadingLogo()
{
	EFI_STATUS Status;

	VOID *FileBuffer;
	UINTN FileSize;
	EFI_IMAGE_OUTPUT ImageOutput;

	UINTN XOfUpperLeftHand;
	UINTN YOfUpperLeftHand;


	ReadFileToBuffer(LOGO_ICO_PATH, &FileBuffer, &FileSize);
	ImageOutput.Width = LOGO1_WIDTH;
	ImageOutput.Height = LOGO1_HEIGHT;
	ImageOutput.Image.Bitmap = FileBuffer;

	XOfUpperLeftHand = (Horizontal - ImageOutput.Width) >> 1;
	YOfUpperLeftHand = (Vertical >> 3);

	Status = DisplayImage(&ImageOutput, XOfUpperLeftHand, YOfUpperLeftHand);
	if (EFI_ERROR(Status))
		Print(L"Unable to display logo.ico\n");

	FreePool(FileBuffer);

	ReadFileToBuffer(LOADING_ICO_PATH, &FileBuffer, &FileSize);
	ImageOutput.Width = LOGO2_WIDTH;
	ImageOutput.Height = LOGO2_HEIGHT;
	ImageOutput.Image.Bitmap = FileBuffer;


	XOfUpperLeftHand = ((Horizontal - ImageOutput.Width) >> 1) + LOADING_OFFSET;
	YOfUpperLeftHand = (Vertical >> 2) + (Vertical >> 1);

	Status = DisplayImage(&ImageOutput, XOfUpperLeftHand, YOfUpperLeftHand);
	if (EFI_ERROR(Status))
		Print(L"Unable to display loading.ico\n");

	FreePool(FileBuffer);
}

#endif

VOID AdjustGraphicsMode(IN LOADER_MACHINE_INFORMATION *MachineInfo)
{

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *GrahpicsOutputModeInformation;
	UINTN InfoSize = 0;
	UINT32 i = 0;
	UINT32 DefaultMode = 0;

	WpLocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&gGraphicsOutput, L"GraphicsOutputProtocol");

	/* ========================= Adjust the Graphics mode ====================== */

	// Max Screen
	DefaultMode = gGraphicsOutput->Mode->MaxMode - 1;

	/*  List Graphics Modes*/
	for (i = 0; i < gGraphicsOutput->Mode->MaxMode; i++)
	{
		gGraphicsOutput->QueryMode(gGraphicsOutput, i, &InfoSize, &GrahpicsOutputModeInformation);
#ifdef _DEBUG
		Print(L"Mode:%02d, Version:%x, Format:%d, Horizontal:%d, Vertical:%d, ScanLine:%d\n", i,
					GrahpicsOutputModeInformation->Version, GrahpicsOutputModeInformation->PixelFormat,
					GrahpicsOutputModeInformation->HorizontalResolution, GrahpicsOutputModeInformation->VerticalResolution,
					GrahpicsOutputModeInformation->PixelsPerScanLine);
#endif
		if (GrahpicsOutputModeInformation->HorizontalResolution == DEFAULT_HORIZONTAL_RESOLUTION && GrahpicsOutputModeInformation->VerticalResolution == DEFAULT_VERTICAL_RESOLUTION)
		{
			DefaultMode = i;
		}
	}

	gGraphicsOutput->SetMode(gGraphicsOutput, DefaultMode);
	WpLocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&gGraphicsOutput, L"GraphicsOutputProtocol");


#ifdef _DEBUG
	Print(L"Default Mode:%02d, Version:%x, Format:%d, Horizontal:%d, Vertical:%d, ScanLine:%d, FrameBufferBase:%018lx, FrameBufferSize:%018lx\n",
				gGraphicsOutput->Mode->Mode, gGraphicsOutput->Mode->Info->Version, gGraphicsOutput->Mode->Info->PixelFormat,
				gGraphicsOutput->Mode->Info->HorizontalResolution, gGraphicsOutput->Mode->Info->VerticalResolution,
				gGraphicsOutput->Mode->Info->PixelsPerScanLine,
				gGraphicsOutput->Mode->FrameBufferBase, gGraphicsOutput->Mode->FrameBufferSize);
#endif

	MachineInfo->GraphicsOutputInformation.HorizontalResolution = Horizontal = gGraphicsOutput->Mode->Info->HorizontalResolution;
	MachineInfo->GraphicsOutputInformation.VerticalResolution = Vertical = gGraphicsOutput->Mode->Info->VerticalResolution;
	MachineInfo->GraphicsOutputInformation.PixelsPerScanLine = PixelsPerScanLine = gGraphicsOutput->Mode->Info->PixelsPerScanLine;
	MachineInfo->GraphicsOutputInformation.FrameBufferBase = gGraphicsOutput->Mode->FrameBufferBase;
	MachineInfo->GraphicsOutputInformation.FrameBufferSize = gGraphicsOutput->Mode->FrameBufferSize;
}
