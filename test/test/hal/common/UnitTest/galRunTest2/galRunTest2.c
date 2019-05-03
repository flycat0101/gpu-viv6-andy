/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <galUtil.h>
#include "gutsystem.h"

//#define VIVANTE_DEBUG

/* Version info. */
static const char c_szFrameworkInfo[] = "\tGAL Test Framework Version 3.1 \n";

/* Test case. */
static char        g_caseName[MAX_BUFFER_SIZE + 1]    = "";
static char        g_simpleCaseName[MAX_BUFFER_SIZE + 1]  = "";

gctHANDLE   g_caseDll           = gcvNULL;
static GalTest     *g_testObject       = gcvNULL;

/* Base objects */
static GalRuntime g_Runtime;
static gctUINT s_frameNo;
static gctBOOL g_UseRange = gcvFALSE;

#if defined(LINUX) || defined(ANDROID)
/* Video memory mapping. */
#if ((gcvVERSION_MAJOR > 6) ||((gcvVERSION_MAJOR == 6) && (gcvVERSION_MINOR >= 3)))
static gctUINT32 g_InternalPhysical, g_ExternalPhysical, g_ContiguousPhysical;
#else
static gctPHYS_ADDR g_InternalPhysical, g_ExternalPhysical, g_ContiguousPhysical;
#endif
static gctSIZE_T    g_InternalSize,     g_ExternalSize,     g_ContiguousSize;
static gctPOINTER   g_Internal,         g_External,         g_Contiguous;
#endif

static gctSTRING        g_Argv2Test[MAX_ARG_COUNT] = {"",};

static void PrintUsage()
{
    sysOutput("Usage: galRunTest2 gltTestCase.dll [-option value] [...]\n"
            "options:\n"
#ifdef VIVANTE_DEBUG
            "\tw\tThe width of the target surface.\n"
            "\th\tThe height of the target surface.\n"
            "\tf\tThe format of the target surface.\n"
            "\tp\tThe pool of the target surface.\n"
#endif
            "\ts\tenable/disable. Enable/disable target saving.\n"
            "\tc\tThe config file.\n");
}

static const struct PoolInfo
{
    gctCONST_STRING name;
    gcePOOL         type;
} c_PoolInfos[] =
{
    {"DEFAULT", gcvPOOL_DEFAULT},
    {"VIRTUAL", gcvPOOL_VIRTUAL},
    {"SYSTEM",  gcvPOOL_SYSTEM},
};

static gcePOOL ConvToTargetPool(const char *name)
{
    int i;

    for (i = 0; i < sizeof(c_PoolInfos) / sizeof(c_PoolInfos[0]); i++)
    {
        if (strcmp(name, c_PoolInfos[i].name) == 0) {
            return c_PoolInfos[i].type;
        }
    }

    sysOutput("*WARN* Unsupport target surface pool: \"%s\", "
            "use default value: \"gcvPOOL_DEFAULT\"\n", name);
    return gcvPOOL_DEFAULT;
}

static const char * ConvToTargetPoolStr(gcePOOL pool)
{
    int i;

    for (i = 0; i < sizeof(c_PoolInfos) / sizeof(c_PoolInfos[0]); i++)
    {
        if (c_PoolInfos[i].type == pool) {
            return c_PoolInfos[i].name;
        }
    }

    return "Unknown Pool Type";
}


gctBOOL ConfigSetup(const char* cfgName)
{
    FILE* config;
    char strTemp[MAX_BUFFER_SIZE];

    config = fopen(cfgName, "r");
    if (config == gcvNULL)
    {
        sysOutput("There's no config file: %s\n", cfgName);
        return gcvFALSE;
    }

    fscanf (config, "Screen Width = %d\n", &g_Runtime.width);
    fscanf (config, "Screen Height = %d\n", &g_Runtime.height);
    fscanf (config, "Surface Format = %s\n", strTemp);
    g_Runtime.format = GalQueryFormat(strTemp);
    if (g_Runtime.format == gcvSURF_UNKNOWN)
    {
        sysOutput("Unknown format: %s\n", strTemp);
        return gcvFALSE;
    }

    fscanf (config, "Surface Pool = %s\n", strTemp);
    g_Runtime.pool = ConvToTargetPool(strTemp);

    fclose (config);
    return gcvTRUE;
}

/*
 *  Name:       ParseArgs( int argc, char *argv[] )
 *  Returns:    0 when successed.
 *  Parameters: arguments, the command line arguments to parse.
 *  Description:This function parses the command line arguments and set the corresponding variables, e.g., the bitmap directory, the log file directory and so on.
*/
static int ParseArgs(int argc, char *argv[])
{
    int     i;
    int     result          = 0;
    char    *str;
    char strCfgName[MAX_BUFFER_SIZE]= {'\0',};
#ifdef VIVANTE_DEBUG
    gctUINT cmdWidth = 0, cmdHeight = 0;
    gceSURF_FORMAT cmdFormat = gcvSURF_UNKNOWN;
    gcePOOL cmdPool = -1;
#endif /* VIVANTE_DEBUG */

    /* Set g_caseName. */
    memset(g_caseName, 0, sizeof(g_caseName));
    strncpy(g_caseName, argv[1], strlen(argv[1]));

    /* Start from argv[2]. */
    i = 2;
    while (i < argc)
    {
        str = argv[i];
        if ((str[0] == '-' || str[0] == '/') && str[2] == '\0')
        {
            switch (str[1])
            {
#ifdef VIVANTE_DEBUG
                case 'w':
                    {
                        assert(argv[i + 1] != NULL);
                        cmdWidth = atoi(argv[i + 1]);
                        i += 2;
                    }
                    continue;

                case 'h':
                    {
                        assert(argv[i + 1] != NULL);
                        cmdHeight = atoi(argv[i + 1]);
                        i += 2;
                    }
                    continue;

                case 'f':
                    {
                        assert(argv[i + 1] != NULL);
                        cmdFormat = ConvToTargetFormat(argv[i + 1]);
                        i += 2;
                    }
                    continue;

                case 'p':
                    {
                        assert(argv[i + 1] != NULL);
                        cmdPool = ConvToTargetPool(argv[i + 1]);
                        i += 2;
                    }
                    continue;
#endif /* VIVNATE_DEBUG */
                case 'n':
                    {
                        assert(argv[i + 1] != NULL);
                        g_Runtime.startFrame = atoi(argv[i + 1]);
                        g_Runtime.endFrame = atoi(argv[i + 2]);
                        i += 3;
                        if (g_Runtime.startFrame > g_Runtime.endFrame)
                        {
                            sysOutput("start frame no.%d is bigger than end frame no.%d\n",
                                        g_Runtime.startFrame, g_Runtime.endFrame);
                            result = -1;
                        }
                        else
                        {
                            g_UseRange = gcvTRUE;
                        }
                    }
                    continue;

                case 'c':
                    {
                        if (argv[i + 1] == NULL)
                        {
                            sysOutput("please specify the config file following -c\n");
                            result = -1;
                        }
                        strncpy(strCfgName, argv[i + 1], strlen(argv[i + 1]));
                        i += 2;
                     }
                    continue;

                case 's':
                    {
                        if (argv[i + 1] == NULL)
                        {
                            sysOutput("please specify enable/disable following -s\n");
                            result = -1;
                        }

                        if (!strncmp("enable", argv[i + 1], 6))
                        {
                            g_Runtime.saveTarget = gcvTRUE;
                        }
                        else if (!strncmp("disable", argv[i + 1], 7))
                        {
                            g_Runtime.saveTarget = gcvFALSE;
                            g_Runtime.noSaveTargetNew = gcvTRUE;
                        }
                        else
                        {
                            sysOutput("please specify enable/disable following -s\n");
                            result = -1;
                        }

                        i += 2;
                     }
                    continue;

                case 't':
                    {
                        if (argv[i + 1] == NULL)
                        {
                            sysOutput("please specify enable/disable following -t\n");
                            result = -1;
                        }

                        if (!strncmp("enable", argv[i + 1], 6))
                        {
                            g_Runtime.createTarget = gcvTRUE;
                        }
                        else if (!strncmp("disable", argv[i + 1], 7))
                        {
                            g_Runtime.createTarget = gcvFALSE;
                        }
                        else
                        {
                            sysOutput("please specify enable/disable following -s\n");
                            result = -1;
                        }

                        i += 2;
                     }
                    continue;


                default:
                    break;
            }
        }

        {
            g_Argv2Test[g_Runtime.argc] = argv[i];
            i++;

            ++g_Runtime.argc;
            if (g_Runtime.argc > MAX_ARG_COUNT)
            {
                sysOutput("Arguments for test too much\n");
                result = -1;
            }
        }
    }

    /* Use cfg file to cover the default values. */
#ifdef VIVANTE_DEBUG
    if (strCfgName[0] != '\0')
    {
        ConfigSetup(strCfgName);
    }

    // use command value to cover the default values
    if (cmdWidth != 0)
    {
        g_Runtime.width = cmdWidth;
    }

    if (cmdHeight != 0)
    {
        g_Runtime.height = cmdHeight;
    }

    if (cmdFormat != gcvSURF_UNKNOWN)
    {
        g_Runtime.format = cmdFormat;
    }

    if (cmdPool != -1)
    {
        g_Runtime.pool = cmdPool;
    }
#else
    if (strCfgName[0] == '\0')
    {
        sysOutput("Please specify config file with -c\n");
        return -1;
    }

    if (!ConfigSetup(strCfgName))
        return -1;
#endif

    return result;
}

int ChipCheck(GalRuntime *rt)
{
    gceSTATUS status;
    gctUINT32 i;
    gctINT ret = 0;

    status = gcoHAL_QueryChipIdentity(rt->hal, &rt->ChipModel, &rt->ChipRevision, gcvNULL, gcvNULL);
    if (status < 0)
    {
        GalOutput(GalOutputType_Log, "*ERROR* Failed to query chip info (status = 0x%x)\n", status);
        ret = -1;
        goto EXIT;
    }

    status = gcoOS_ReadRegister(rt->os, 0x98, &rt->PatchRevision);
    if (status < 0)
    {
        GalOutput(GalOutputType_Log, "*ERROR* Failed to read patch version (status = 0x%x)\n", status);
        ret = -2;
        goto EXIT;
    }

    GalOutput(GalOutputType_Log, "=================== Chip Information ==================\n");
    GalOutput(GalOutputType_Log, "Chip : GC%x\n", rt->ChipModel);
    GalOutput(GalOutputType_Log, "Chip revision: 0x%08x\n", rt->ChipRevision);
    GalOutput(GalOutputType_Log, "Patch revision: 0x%08x\n", rt->PatchRevision);
    GalOutput(GalOutputType_Log, "Chip Features: 0x%08x\n", rt->ChipFeatures);

    {
        /* TODO: Because the latest driver has removed the feature register query.
                 Fake feature registers to compatible to old golden select script.
         *       Update script later to remove this fake.
         */
        memset(rt->ChipMinorFeatures, 0, gcmCOUNTOF(rt->ChipMinorFeatures));

        if (gcoHAL_IsFeatureAvailable(rt->hal, gcvFEATURE_2D_YUV_BLIT) == gcvTRUE)
            rt->ChipMinorFeatures[2] = 0x1 << 17;

        if (gcoHAL_IsFeatureAvailable(rt->hal, gcvFEATURE_2D_COLOR_SPACE_CONVERSION) == gcvTRUE)
            rt->ChipMinorFeatures[4] = 0x1 << 12;

        if (gcoHAL_IsFeatureAvailable(rt->hal, gcvFEATURE_2D_ONE_PASS_FILTER_TAP) == gcvTRUE)
            rt->ChipMinorFeatures[5] = 0x1 << 11;

        if (gcoHAL_IsFeatureAvailable(rt->hal, gcvFEATURE_ANDROID_ONLY) == gcvTRUE)
            rt->ChipMinorFeatures[5] = 0x1 << 16;

        for (i = 0; i < 6; i++)
        {
            GalOutput(GalOutputType_Log, "Chip MinorFeatures%d: 0x%08x\n", i, rt->ChipMinorFeatures[i]);
        }
    }

    GalOutput(GalOutputType_Log, "=================== Feature List ======================\n");

    for (i = 1;; i++)
    {
        static char name[FEATURE_NAME_LEN];
        static gceFEATURE feature;

        if (GalQueryFeatureByIndex(i, &feature, name, gcvNULL, gcvNULL) != gcvSTATUS_OK)
            break;

        GalOutput(GalOutputType_Log, "%s: %d\n",
                name, gcoHAL_IsFeatureAvailable(rt->hal, feature));
    }

    GalOutput(GalOutputType_Log, "=======================================================\n");

EXIT:

    return ret;
}

/*
 *  Name:       Initialize()
 *  Parameters: case_name, the name of the dll of the test case.
*/

static gctBOOL Initialize()
{
    PGalCreateTestObject createTestObject;
    gceSTATUS status;

    /* Load test dll */
    g_caseDll = sysLoadModule(g_caseName);
    if (g_caseDll == gcvNULL)
    {
        sysOutput("*ERROR* Failed to load: \"%s\"\n", g_caseName);
        return gcvFALSE;
    }

    createTestObject = (PGalCreateTestObject)sysGetProcAddress(g_caseDll, "GalCreateTestObject");
    if (createTestObject == gcvNULL)
    {
        sysOutput("*ERROR* Failed to find: \"GalCreateTestObject\" in %s\n", g_caseName);
        return gcvFALSE;
    }

    /* Setup log file objects. */
    if (!sysSetupLog(g_simpleCaseName))
    {
        sysOutput("*ERROR* Failed to set up log\n");
        return gcvFALSE;
    }

    /* Construct the gcoOS object. */
    status = gcoOS_Construct(gcvNULL, &g_Runtime.os);
    if (status < 0)
    {
        sysOutput("*ERROR* Failed to construct OS object (status = %d)\n", status);
        return gcvFALSE;
    }

    /* Construct the gcoHAL object. */
    status = gcoHAL_Construct(gcvNULL, g_Runtime.os, &g_Runtime.hal);
    if (status < 0)
    {
        sysOutput("*ERROR* Failed to construct GAL object (status = %d)\n", status);
        return gcvFALSE;
    }

    if (!gcoHAL_IsFeatureAvailable(g_Runtime.hal, gcvFEATURE_2DPE20))
    {
        switch (g_Runtime.format)
        {
        /* PE1.0 support. */
        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_X1R5G5B5:
        case gcvSURF_A1R5G5B5:
        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
        case gcvSURF_R5G6B5:
            break;

        default:
            sysOutput("*ERROR* the target format %d is not supported by the hardware.\n",
                g_Runtime.format);
            return gcvFALSE;
        }
    }

#if defined(LINUX) || defined(ANDROID)
    /* Query the amount of video memory. */
    status = gcoHAL_QueryVideoMemory(g_Runtime.hal,
                                     &g_InternalPhysical, &g_InternalSize,
                                     &g_ExternalPhysical, &g_ExternalSize,
                                     &g_ContiguousPhysical, &g_ContiguousSize);
    if (gcmIS_ERROR(status))
    {
        sysOutput("gcoHAL_QueryVideoMemory failed %d.", status);
        return gcvFALSE;
    }

    /* Map the local internal memory. */
    if (g_InternalSize > 0)
    {
        status = gcoHAL_MapMemory(g_Runtime.hal,
                                  g_InternalPhysical, g_InternalSize,
                                  &g_Internal);
        if (gcmIS_ERROR(status))
        {
            sysOutput("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }

    /* Map the local external memory. */
    if (g_ExternalSize > 0)
    {
        status = gcoHAL_MapMemory(g_Runtime.hal,
                                  g_ExternalPhysical, g_ExternalSize,
                                  &g_External);
        if (gcmIS_ERROR(status))
        {
            sysOutput("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }

    /* Map the contiguous memory. */
    if (g_ContiguousSize > 0)
    {
        status = gcoHAL_MapMemory(g_Runtime.hal,
                                  g_ContiguousPhysical, g_ContiguousSize,
                                  &g_Contiguous);
        if (gcmIS_ERROR(status))
        {
            sysOutput("gcoHAL_MapMemory failed %d.", status);
            return gcvFALSE;
        }
    }
#endif

    status = gcoHAL_Get2DEngine(g_Runtime.hal, &g_Runtime.engine2d);
    if (status < 0)
    {
        sysOutput("*ERROR* Failed to get 2D engine object (status = %d)\n", status);
        return gcvFALSE;
    }

    if (g_Runtime.createTarget)
    {
        status = gcoSURF_Construct(g_Runtime.hal,
                                  g_Runtime.width,
                                  g_Runtime.height,
                                  1,
                                  gcvSURF_BITMAP,
                                  g_Runtime.format,
                                  g_Runtime.pool,
                                  &g_Runtime.target);
        if (status < 0)
        {
            sysOutput("*ERROR* Failed to construct SURFACE object (status = %d)\n", status);
            return gcvFALSE;
        }
    }
    else
        g_Runtime.target = gcvNULL;


    g_Runtime.pe20      = gcoHAL_IsFeatureAvailable(g_Runtime.hal, gcvFEATURE_2DPE20);
#if gcvVERSION_MAJOR >= 4
    g_Runtime.fullDFB     = gcoHAL_IsFeatureAvailable(g_Runtime.hal, gcvFEATURE_FULL_DIRECTFB);
#else
    g_Runtime.fullDFB     = gcvFALSE;
#endif

    // log chip info
    if (ChipCheck(&g_Runtime) < 0)
    {
        sysOutput("*ERROR* Check chip info failed!\n");
        return gcvFALSE;
    }

    // output framework cfg info
    GalOutput(GalOutputType_Log, c_szFrameworkInfo);

    GalOutput(GalOutputType_Log,
        "===================== Test Config =====================\n"
        "\tscreen window width : %d\n"
        "\tscreen window height: %d\n"
        "\tsurface format: %s\n"
        "\tsurface pool: %s\n"
        "=======================================================\n",
        g_Runtime.width,
        g_Runtime.height,
        GalQueryFormatStr(g_Runtime.format),
        ConvToTargetPoolStr(g_Runtime.pool));

    g_testObject = createTestObject(&g_Runtime);

    /* output test case description */
    if (g_Runtime.wholeDescription != gcvNULL)
    {
        GalOutput(GalOutputType_Log, g_Runtime.wholeDescription);
    }

    if (g_testObject == gcvNULL)
    {
        if (!g_Runtime.notSupport)
            sysOutput("*ERROR* Failed to create test object for %s\n", g_caseName);
        return gcvFALSE;
    }

    s_frameNo = g_Runtime.startFrame; /* Frame Number. */

    return gcvTRUE;
}

/*
 *  Name:       Finalize()
 *  Returns:    None.
 *  Parameters: None.
 *  Description:Free all resource that the framework has used. These may include the memory resource it used, and the egl system resource and so on.
 *              Here it includes "finalize the test object(case)", "free egl resource", "free library resource", "finalize output file resource" and
 *              "destroy win32 resource".
*/
static void Finalize()
{
    if (g_testObject != gcvNULL)
    {
        if (g_testObject->destroy != gcvNULL) g_testObject->destroy(g_testObject);
    }

    if (g_Runtime.wholeDescription != gcvNULL)
    {
        free(g_Runtime.wholeDescription);
    }

    if (g_caseDll != gcvNULL)
        sysUnloadModule(g_caseDll);

    if (g_Runtime.hal != gcvNULL)
    {
        gcoHAL_Commit(g_Runtime.hal, gcvTRUE);
    }

    if (g_Runtime.target != gcvNULL)
    {
        gcoSURF_Destroy(g_Runtime.target);
    }

#if defined(LINUX) || defined(ANDROID)
    if (g_Internal != gcvNULL)
    {
        /* Unmap the local internal memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(g_Runtime.hal,
                                        g_InternalPhysical, g_InternalSize,
                                        g_Internal));
    }

    if (g_External != gcvNULL)
    {
        /* Unmap the local external memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(g_Runtime.hal,
                                        g_ExternalPhysical, g_ExternalSize,
                                        g_External));
    }

    if (g_Contiguous != gcvNULL)
    {
        /* Unmap the contiguous memory. */
        gcmVERIFY_OK(gcoHAL_UnmapMemory(g_Runtime.hal,
                                        g_ContiguousPhysical, g_ContiguousSize,
                                        g_Contiguous));
    }
#endif

    if (g_Runtime.hal != gcvNULL)
    {
        gcoHAL_Commit(g_Runtime.hal, gcvTRUE);
        gcoHAL_Destroy(g_Runtime.hal);
    }

    if (g_Runtime.os != gcvNULL)
    {
        gcoOS_Destroy(g_Runtime.os);
    }

    GalFinalizeOutput();
}

static gctBOOL MakeSimpleCaseName(char simpleCaseName[MAX_BUFFER_SIZE + 1], char *caseName)
{
    char *start = caseName;
    size_t len = strlen(caseName);
    assert(len < MAX_BUFFER_SIZE);

    memset(simpleCaseName, 0, sizeof(simpleCaseName));

    do {
        char *end;
        int i;

        start = strstr(start, PREFIX);
        if (start != '\0')
        {
            strcpy(simpleCaseName, start);
            start += sizeof(PREFIX);
        }
        else
        {
            return gcvFALSE;
        }

        end = strstr(simpleCaseName, SURFFIX);
        if (end != gcvNULL)
        {
            *end = '\0';
        }

        for (i = 0; simpleCaseName[i] != '\0'; i++)
        {
            if (!((simpleCaseName[i] >= 'a' && simpleCaseName[i] <= 'z')
                || (simpleCaseName[i] >= 'A' && simpleCaseName[i] <= 'Z')
                || (simpleCaseName[i] >= '0' && simpleCaseName[i] <= '9')
                || (simpleCaseName[i] == '_')))
            {
                break;
            }
        }

        if (simpleCaseName[i] == '\0')
        {
            return gcvTRUE;
        }
    } while (start);

    return gcvFALSE;
}

/*
 *  Name:       Run()
 *  Returns:    None.
 *  Parameters: None.
*/
static gctBOOL Run()
{
    do {
        char bmpFileName[MAX_BUFFER_SIZE * 2];

        if (g_testObject == gcvNULL || g_testObject->render == gcvNULL ||
           (g_Runtime.createTarget && g_Runtime.target == gcvNULL))
        {
            break;
        }

        /* Notes: Some cases use accumulated vaule by frame number inside test body.
         *        You may be not expected to get the result images by rendering only one/some frames.
         */
        if (s_frameNo+1 > g_testObject->frameCount)
        {
            GalOutput(GalOutputType_Console,
                "Frame %d is beyend range. Total frames are %d.\n", s_frameNo, g_testObject->frameCount);
            break;
        }

        GalOutput(GalOutputType_Result|GalOutputType_Console,
            "Begin to render %s frame %d ...\n", g_simpleCaseName, s_frameNo);

        /* clear target surface to black. */
        if (g_Runtime.target != gcvNULL && g_Runtime.cleanTarget)
        {
            if(gcmIS_ERROR(Gal2DCleanSurface(g_Runtime.hal, g_Runtime.target, COLOR_ARGB8(0x00, 0x00, 0x00, 0x00))))
            {
                break;
            }
        }

        /* generate bmp file name */
        sprintf(bmpFileName, "%s%s_%03d.bmp", g_bmpPath, g_simpleCaseName, s_frameNo);
        if (g_Runtime.saveTarget)
        {
            g_Runtime.saveFullName = gcvNULL;
        }
        else
        {
            g_Runtime.saveFullName = bmpFileName;
        }

        if (g_testObject->render(g_testObject, s_frameNo) == gcvTRUE)
        {
            /* Rendering succeed. */
            GalOutput(GalOutputType_Result|GalOutputType_Console,
                "Rendering %s frame %d ... succeed\n", g_simpleCaseName, s_frameNo);
        }
        else
        {
            /* Failed. */
            GalOutput(GalOutputType_Result|GalOutputType_Console,
                "Rendering %s frame %d ... failed\n", g_simpleCaseName, s_frameNo);
            break;
        }

        if (g_Runtime.saveTarget)
        {
            // save rendering result
            if (!GalSaveSurface2DIB(g_Runtime.target, bmpFileName))
            {
                GalOutput(GalOutputType_Error|GalOutputType_Console,
                    "*ERROR* Failed to save bmp file: %s\n", bmpFileName);
                break;
            }
        }

        /* next frame */
        s_frameNo++;

        return (s_frameNo < g_testObject->frameCount && (!g_UseRange || s_frameNo <= g_Runtime.endFrame));

    } while (gcvFALSE);

    GalOutput(GalOutputType_Error|GalOutputType_Console,
                    "*ERROR* Rendering failed in frame %d!!!\n", s_frameNo);
    /* stop rendering. */
    return gcvFALSE;
}


/*******************************************************************************
**
**  main
**
**  Test entry point.
**
**  INPUT:
**
**      Command lines.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURN:
**
**      int
**          Exit value.
*/
int main(int argc, char *argv[])
{
    int i;

    /* Assume failure. */
    int result = -1;

    if (argc < 4)
    {
        sysOutput("The parameters are too less!!!");
        PrintUsage();
        return result;
    }

    // echo cmd line
    for (i = 0; i < argc; i++)
    {
        sysOutput("%s ", argv[i]);
        if (i == argc - 1)
            sysOutput("\n");
    }

    /* default arguements */
    memset(&g_Runtime, 0, sizeof(g_Runtime));

    /* target surface. */
    g_Runtime.target    = gcvNULL;
    g_Runtime.width   = 640;
    g_Runtime.height  = 480;
    g_Runtime.format  = gcvSURF_A8R8G8B8;
    g_Runtime.pool  = gcvPOOL_DEFAULT;

    g_Runtime.argc      = 0;
    g_Runtime.argv      = g_Argv2Test;
    g_Runtime.saveTarget    = gcvTRUE;
    g_Runtime.noSaveTargetNew = gcvFALSE;
    g_Runtime.cleanTarget = gcvTRUE;
    g_Runtime.createTarget = gcvTRUE;
    g_Runtime.notSupport = gcvFALSE;
    g_Runtime.startFrame = 0;
    g_Runtime.endFrame = 0;

    /* check command arguements */
    if (ParseArgs(argc, argv) != 0)
    {
        PrintUsage();
        return 1;
    }

    if (!MakeSimpleCaseName(g_simpleCaseName, g_caseName))
    {
        sysOutput("Invalid case name:%s. Should include string "PREFIX"*"SURFFIX"\n", g_caseName);
        return 2;
    }

    do
    {
        /* Initialize the test. */
        if (!Initialize())
        {
            if (!g_Runtime.notSupport)
                sysOutput("Initialize test failed\n");
            else
                sysOutput("Cannot initialize test because hw feature is not supported.\n");
            result = 0;
            break;
        }

        while (Run())
        {
        }

        result = 0;
    }
    while (gcvFALSE);

    /* Cleanup the test environment. */
    Finalize();

    return result;
}

