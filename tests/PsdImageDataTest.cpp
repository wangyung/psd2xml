#include <lib/psd_win32.h>
#include <gtest/gtest.h>
#include <lib/psd_file.h>

static const char *inputFile;

class PsdImageDataTest : public testing::Test
{
protected:
    static int GlobalSetUp()
    {
        inputFile = g_argc > 1 ? g_argv[1] : "vectors/clock_original.psd";
#ifndef _WIN32
        system("rm -rf PsdImageDataTest.d");
        mkdir("PsdImageDataTest.d", 00777);
#else
        system("rd /s /q PsdImageDataTest.d");
        CreateDirectoryA("PsdImageDataTest.d", NULL);
#endif
        return 0;
    }

    virtual void SetUp()
    {
        static volatile int _gs = GlobalSetUp();
        _gs;

        file = new PsdFile(inputFile);
    }

    virtual void TearDown()
    {
        delete file;
        file = NULL;
    }

    PsdFile *file;
};

static char sFilePath[512] = "PsdImageDataTest.d/";
static char *sFileName = sFilePath + sizeof("PsdImageDataTest.d/") - 1;

static
void setFileName(const char *name, const char *suffix = NULL)
{
    char *p = sFileName;
    while (1) {
        switch (*name) {
            case 0:
                goto end;
            case '/':
            case '\\':
            case ':':
            case '?':
            case '*':
            case '>':
            case '<':
                *p++ = '_';
                break;
            default:
                *p++ = *name;
        }
        name++;
    }
end:
    if (suffix)
        sprintf(p, "%s.png", suffix);
    else
        strcpy(p, ".png");
}

static
void storePNG(const shared_ptr<PsdImageData> &imageData, const char *fileName)
{
    ASSERT_NE((void *) NULL, imageData.get());
    PsdStatus status;
    setFileName(fileName);
    ASSERT_LE(PsdStatusOK, (status = imageData->storePNGFile(sFilePath)));
    if (status == PsdStatusOK)
        ASSERT_STREQ(sFilePath, imageData->getStoredPNGFilePath());
    else
        return;

    setFileName(fileName, "_ALPHA");
    imageData->setAlpha(0.5);
    ASSERT_LE(PsdStatusOK, (status = imageData->storePNGFile(sFilePath)));
    if (status == PsdStatusOK)
        ASSERT_STREQ(sFilePath, imageData->getStoredPNGFilePath());
}

static
void storePNGs(PsdLayers layers)
{
    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    for (; iter != end; iter++) {
        shared_ptr<PsdLayer> layer = *iter;
        ASSERT_NE((void *) NULL, layer.get());
        if (layer->getType() == PsdLayer::LayerTypeGroup) {
            storePNGs(((PsdLayerGroup *) layer.get())->getLayers());
        } else {
            storePNG(layer->getImageData(), layer->getName());
        }
    }
}

TEST_F(PsdImageDataTest, storePNGFile) {
    ASSERT_EQ(PsdStatusOK, file->open());

    if (file->getLayers())
        storePNGs(file->getLayers());
    else
        fputs("No layers\n", stderr);
}

TEST_F(PsdImageDataTest, storePreMergedPNGFile) {
    ASSERT_EQ(PsdStatusOK, file->open());

    storePNG(file->getImageData(), "PRE_MERGED_IMAGE");
}

TEST_F(PsdImageDataTest, storeMergedPNGFile) {
    ASSERT_EQ(PsdStatusOK, file->open());

    PsdLayers layers = file->getLayers();
    shared_ptr<PsdImageData> imageData = PsdImageData::fromLayers(layers);
    if (!layers || layers->empty()) {
        ASSERT_EQ((void *) NULL, imageData.get());
        return;
    }

    storePNG(imageData, "MERGED_IMAGE");
}
