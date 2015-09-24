#include <lib/psd_win32.h>
#include <gtest/gtest.h>
#include <lib/psd_file.h>

class PsdFileTest : public testing::Test
{
protected:
    PsdFileTest()
        : inputFile(g_argc > 1 ? g_argv[1] : "vectors/a.psd")
    {
    }

    virtual void SetUp()
    {
        file = new PsdFile(inputFile);
    }

    virtual void TearDown()
    {
        delete file;
        file = NULL;
    }

    PsdFile *file;
    const char *inputFile;
};

TEST_F(PsdFileTest, open) {
    ASSERT_EQ(PsdStatusOK, file->open());
}

static
void printLayers(PsdLayers layers, int level = 0)
{
    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    for (; iter != end; iter++) {
        shared_ptr<PsdLayer> layer = *iter;
        ASSERT_NE((void *) NULL, layer.get());
        if (layer->getType() == PsdLayer::LayerTypeGroup) {
            printf("%*s+ %s\n", level, "", layer->getName());
            printLayers(((PsdLayerGroup *) layer.get())->getLayers(), level + 2);
        } else {
            printf("%*s- %s\n", level, "", layer->getName());
        }
    }
}

TEST_F(PsdFileTest, getLayers) {
    ASSERT_EQ((void *) NULL, file->getLayers().get());
    ASSERT_EQ(PsdStatusOK, file->open());
    ASSERT_NE((void *) NULL, file->getLayers().get());

    printLayers(file->getLayers());
}

TEST_F(PsdFileTest, getImageData) {
    ASSERT_EQ((void *) NULL, file->getImageData().get());
    ASSERT_EQ(PsdStatusOK, file->open());
    ASSERT_NE((void *) NULL, file->getImageData().get());
}

TEST_F(PsdFileTest, getImageResources) {
    ASSERT_EQ((void *) NULL, file->getImageResources().get());
    ASSERT_EQ(PsdStatusOK, file->open());
    ASSERT_NE((void *) NULL, file->getImageResources().get());
}

TEST_F(PsdFileTest, openDirectory) {
    PsdFile f(".");
    ASSERT_EQ(PsdStatusIOError, f.open());
    ASSERT_FALSE(f.isOpen());
}
