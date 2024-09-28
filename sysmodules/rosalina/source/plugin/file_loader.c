#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include "csvc.h"
#include "plugin.h"
#include "ifile.h"
#include "ifile.h"
#include "utils.h"
#include "draw.h"
#include "menu.h"

// Use a global to avoid stack overflow, those structs are quite heavy
static FS_DirectoryEntry   g_entries[10];
static PluginEntry         g_foundPlugins[10];

static char        g_path[256];
static const char *g_dirPath = "/luma/plugins/%016llX";
static const char *g_defaultPath = "/luma/plugins/default.3gx";
u64                g_titleId;

// pluginLoader.s
void        gamePatchFunc(void);

static u32     strlen16(const u16 *str)
{
    if (!str) return 0;

    const u16 *strEnd = str;

    while (*strEnd) ++strEnd;

    return strEnd - str;
}

static char *AskForFileName(PluginEntry *entries, u8 count)
{
    char *filename = NULL;
    u8 selected = 0;
    s8 holding = -1;

    if(count == 1)
        return entries[0].name;

    menuEnter();

    ClearScreenQuickly();

    do
    {
        u32 posY;

        Draw_Lock();
        Draw_ClearFramebuffer();
        Draw_DrawString(10, 10, COLOR_TITLE, "Plugin selector");
        posY = Draw_DrawString(20, 30, COLOR_WHITE, "Some 3gx files were found.");
        posY = Draw_DrawString(20, posY + 10, COLOR_WHITE, "[A] Select, [B] Cancel");
        posY = Draw_DrawString(20, posY + 15, COLOR_LIME, "Plugins:");

        for(u8 i = 0; i < count; i++)
        {
            if(holding == -1)
            {
                Draw_DrawCharacter(10, posY + 15, COLOR_TITLE, i == selected ? '>' : ' ');
            }
            if(i == holding)
            {
                posY = Draw_DrawString(33, posY + 14, COLOR_WHITE, entries[i].name);
            }
            else
            {
                char buf[256];

                sprintf(buf, "  %s", entries[i].name);
                posY = Draw_DrawString(30, posY + 15, COLOR_WHITE, buf);
            }
        }

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 keys;
        do {
            keys = waitComboWithTimeout(1000);
        } while(keys == 0 && !menuShouldExit);

        if(keys & KEY_A)
        {
            if(holding == -1)
            {
                filename = entries[selected].name;
                break;
            }
        }
        else if(keys & KEY_B)
        {
            break;
        }
        else if(keys & KEY_DOWN)
        {
            if(holding == -1 && ++selected >= count)
                selected = 0;
            else if(holding != -1 && holding < count - 1)
            {
                PluginEntry tmp = entries[holding];
                entries[holding] = entries[holding + 1];
                entries[holding + 1] = tmp;
                holding++;
            }
        }
        else if(keys & KEY_UP)
        {
            if(holding == -1 && selected-- <= 0)
                selected = count - 1;
            else if(holding > 0)
            {
                PluginEntry tmp = entries[holding];
                entries[holding] = entries[holding - 1];
                entries[holding - 1] = tmp;
                holding--;
            }
        }
    } while(!menuShouldExit);

    menuLeave();

    return filename;
}

static Result   FindPluginFile(u64 tid, u8 defaultFound)
{
    char                filename[256];
    u32                 entriesNb = 0;
    Handle              dir = 0;
    Result              res;
    FS_Archive          sdmcArchive = 0;
    FS_DirectoryEntry * entries = g_entries;
    u8                  foundPluginCount = 0;
    PluginEntry       * foundPlugins = g_foundPlugins;
    
    memset(entries, 0, sizeof(g_entries));
    memset(foundPlugins, 0, sizeof(g_foundPlugins));
    sprintf(g_path, g_dirPath, tid);
    strcat(g_path, "/");

    if (R_FAILED((res = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, "")))))
        goto exit;

    if (R_FAILED((res = FSUSER_OpenDirectory(&dir, sdmcArchive, fsMakePath(PATH_ASCII, g_path))))){
        if(defaultFound == 1)
        {
            res = 0;
            goto defaultplg;
        }
        else
            goto exit;
    }

    while (foundPluginCount < 10 && R_SUCCEEDED(FSDIR_Read(dir, &entriesNb, 10, entries)))
    {
        if (entriesNb == 0)
            break;

        static const u16 *   validExtension = u"3gx";

        for (u32 i = 0; i < entriesNb; ++i)
        {
            FS_DirectoryEntry *entry = &entries[i];
            char path[512];

            // If entry is a folder, skip it
            if (entry->attributes & 1)
                continue;

            // Check extension
            u32 size = strlen16(entry->name);
            if (size <= 5)
                continue;

            u16 *fileExt = entry->name + size - 3;

            if (memcmp(fileExt, validExtension, 3 * sizeof(u16)))
                continue;

            // Convert name from utf16 to utf8
            int units = utf16_to_utf8((u8 *)filename, entry->name, 100);
            if (units == -1)
                continue;
            filename[units] = 0;

            if(foundPluginCount >= 10) 
                break;

            strcpy(foundPlugins[foundPluginCount].name, filename);

            sprintf(path, "%s%s", g_path, filename);
            foundPluginCount++;
        }
    }

defaultplg:

    if(foundPluginCount < 10 && defaultFound == 1){
        strcpy(foundPlugins[foundPluginCount].name, "<default.3gx>");
        foundPluginCount++;
    }

    if (!foundPluginCount)
        res = MAKERESULT(28, 4, 0, 1018);
    else
    {
        char *name = AskForFileName(foundPlugins, foundPluginCount);

        if (!name)
        {
            res = MAKERESULT(28, 4, 0, 1018);
        }
        else if(name[0] == '<') // This isn't the best way to check this, but this is fine because it is a illegal file name character
        {
            PluginLoaderCtx.pluginPath = g_defaultPath;
            PluginLoaderCtx.header.isDefaultPlugin = 1;
        }
        else
        {
            u32 len = strlen(g_path);
            name[sizeof(((PluginEntry *)NULL)->name) - len] = 0;
            strcat(g_path, name);
            PluginLoaderCtx.pluginPath = g_path;
        }
    }

exit:
    FSDIR_Close(dir);
    FSUSER_CloseArchive(sdmcArchive);

    return res;
}

static Result   OpenFile(IFile *file, const char *path)
{
    return IFile_Open(file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ);
}

static Result   OpenPluginFile(u64 tid, IFile *plugin)
{
    // Note: I didn't find a way to check if the file exists without opening it,
    // so I made this mess instead. If someone knows a better way, please do it.
    u8 defaultFound = 1;

    if (OpenFile(plugin, g_defaultPath))
        defaultFound = 0;

    if (R_FAILED(FindPluginFile(tid, defaultFound))){
        if(defaultFound == 1)
            IFile_Close(plugin);
        return -1;
    }

    if(PluginLoaderCtx.header.isDefaultPlugin == 0)
    {
        if(defaultFound == 1)
            IFile_Close(plugin);
        if(OpenFile(plugin, g_path)) return -1;
    }

    return 0;
}

static Result   CheckPluginCompatibility(_3gx_Header *header, u32 processTitle)
{
    static char   errorBuf[0x100];

    if (header->targets.count == 0)
        return 0;

    for (u32 i = 0; i < header->targets.count; ++i)
    {
        if (header->targets.titles[i] == processTitle)
            return 0;
    }

    sprintf(errorBuf, "The plugin - %s -\nis not compatible with this game.\n" \
                      "Contact \"%s\" for more infos.", header->infos.titleMsg, header->infos.authorMsg);
    
    PluginLoaderCtx.error.message = errorBuf;

    return -1;
}

bool     TryToLoadPlugin(Handle process)
{
    u64             tid;
    u64             fileSize;
    IFile           plugin;
    Result          res;
    _3gx_Header     fileHeader;
    _3gx_Header     *header = NULL;
    _3gx_Executable *exeHdr = NULL;
    PluginLoaderContext *ctx = &PluginLoaderCtx;
    PluginHeader        *pluginHeader = &ctx->header;
    const u32           memRegionSizes[] = 
    {
        5 * 1024 * 1024, // 5 MiB
        2 * 1024 * 1024, // 2 MiB
        10 * 1024 * 1024,  // 10 MiB
        5 * 1024 * 1024, // 5 MiB (Reserved)
    };

    // Get title id
    svcGetProcessInfo((s64 *)&tid, process, 0x10001);

    memset(pluginHeader, 0, sizeof(PluginHeader));
    pluginHeader->magic = HeaderMagic;

    // Try to open plugin file
    if (ctx->useUserLoadParameters && (ctx->userLoadParameters.lowTitleId == 0 || (u32)tid == ctx->userLoadParameters.lowTitleId))
    {
        if (!ctx->userLoadParameters.persistent) ctx->useUserLoadParameters = false;
        ctx->pluginMemoryStrategy = ctx->userLoadParameters.pluginMemoryStrategy;
        if (OpenFile(&plugin, ctx->userLoadParameters.path))
            return false;

        ctx->pluginPath = ctx->userLoadParameters.path;

        memcpy(pluginHeader->config, ctx->userLoadParameters.config, 32 * sizeof(u32));
    }
    else
    {
        if (R_FAILED(OpenPluginFile(tid, &plugin)))
            return false;
    }

    if (R_FAILED((res = IFile_GetSize(&plugin, &fileSize))))
        ctx->error.message = "Couldn't get file size";

    if (!res && R_FAILED(res = Check_3gx_Magic(&plugin)))
    {
        const char * errors[] = 
        {
            "Couldn't read file.",
            "Invalid plugin file\nNot a valid 3GX plugin format!",
            "Outdated plugin file\nCheck for an updated plugin.",
            "Outdated plugin loader\nCheck for Luma3DS updates."   
        };

        ctx->error.message = errors[R_MODULE(res) == RM_LDR ? R_DESCRIPTION(res) : 0];
    }

    // Read header
    if (!res && R_FAILED((res = Read_3gx_Header(&plugin, &fileHeader))))
        ctx->error.message = "Couldn't read file";

    // Check compatibility
    if (!res && fileHeader.infos.compatibility == PLG_COMPAT_EMULATOR) {
        ctx->error.message = "Plugin is only compatible with emulators";
        return false;
    }

    // Flags
    if (!res) {
        ctx->eventsSelfManaged = fileHeader.infos.eventsSelfManaged;
        ctx->isMemPrivate = fileHeader.infos.usePrivateMemory;
        if (ctx->pluginMemoryStrategy == PLG_STRATEGY_SWAP && fileHeader.infos.swapNotNeeded)
            ctx->pluginMemoryStrategy = PLG_STRATEGY_NONE;
    }

    // Set memory region size according to header
    if (!res && R_FAILED((res = MemoryBlock__SetSize(memRegionSizes[fileHeader.infos.memoryRegionSize])))) {
        ctx->error.message = "Couldn't set memblock size.";
    }
    
    // Ensure memory block is mounted
    if (!res && R_FAILED((res = MemoryBlock__IsReady())))
        ctx->error.message = "Failed to allocate memory.";

    // Plugins will not exceed 5MB so this is fine
    if (!res) {
        header = (_3gx_Header *)(ctx->memblock.memblock + g_memBlockSize - (u32)fileSize);
        memcpy(header, &fileHeader, sizeof(_3gx_Header));
    }

    // Parse rest of header
    if (!res && R_FAILED((res = Read_3gx_ParseHeader(&plugin, header))))
        ctx->error.message = "Couldn't read file";

    // Read embedded save/load functions
    if (!res && R_FAILED((res = Read_3gx_EmbeddedPayloads(&plugin, header))))
        ctx->error.message = "Invalid save/load payloads.";
    
    // Save exe checksum
    if (!res)
        ctx->exeLoadChecksum = header->infos.exeLoadChecksum;
    
    // Check titles compatibility
    if (!res) res = CheckPluginCompatibility(header, (u32)tid);

    // Read code
    if (!res && R_FAILED(res = Read_3gx_LoadSegments(&plugin, header, ctx->memblock.memblock + sizeof(PluginHeader)))) {
        if (res == MAKERESULT(RL_PERMANENT, RS_INVALIDARG, RM_LDR, RD_NO_DATA)) ctx->error.message = "This plugin requires a loading function.";
        else if (res == MAKERESULT(RL_PERMANENT, RS_INVALIDARG, RM_LDR, RD_INVALID_ADDRESS)) ctx->error.message = "This plugin file is corrupted.";
        else ctx->error.message = "Couldn't read plugin's code";
    }

    if (R_FAILED(res))
    {
        ctx->error.code = res;
        goto exitFail;
    }

    pluginHeader->version = header->version;
    // Code size must be page aligned
    exeHdr = &header->executable;
    pluginHeader->exeSize = (sizeof(PluginHeader) + exeHdr->codeSize + exeHdr->rodataSize + exeHdr->dataSize + exeHdr->bssSize + 0x1000) & ~0xFFF;
    pluginHeader->heapVA = 0x06000000;
    pluginHeader->heapSize = g_memBlockSize - pluginHeader->exeSize;
    pluginHeader->plgldrEvent = ctx->plgEventPA;
    pluginHeader->plgldrReply = ctx->plgReplyPA;

    // Clear old event data
    PLG__NotifyEvent(PLG_OK, false);

    // Copy header to memblock
    memcpy(ctx->memblock.memblock, pluginHeader, sizeof(PluginHeader));
    // Clear heap
    memset(ctx->memblock.memblock + pluginHeader->exeSize, 0, pluginHeader->heapSize);

    // Enforce RWX mmu mapping
    svcControlProcess(process, PROCESSOP_SET_MMU_TO_RWX, 0, 0);
    // Ask the kernel to signal when the process is about to be terminated
    svcControlProcess(process, PROCESSOP_SIGNAL_ON_EXIT, 0, 0);

    // Mount the plugin memory in the process
    if (R_SUCCEEDED(MemoryBlock__MountInProcess()))
    // Install hook
    {
        u32  procStart = 0x00100000;
        u32 *game = (u32 *)procStart;

        extern u32  g_savedGameInstr[2];

        if (R_FAILED((res = svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, procStart, process, procStart, 0x1000, 0))))
        {
            ctx->error.message = "Couldn't map process";
            ctx->error.code = res;
            goto exitFail;
        }

        g_savedGameInstr[0] = game[0];
        g_savedGameInstr[1] = game[1];

        game[0] = 0xE51FF004; // ldr pc, [pc, #-4]
        game[1] = (u32)PA_FROM_VA_PTR(gamePatchFunc);
        svcFlushEntireDataCache();
        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, procStart, 0x1000);
    }
    else
        goto exitFail;


    IFile_Close(&plugin);
    return true;

exitFail:
    IFile_Close(&plugin);
    MemoryBlock__Free();

    return false;
}
