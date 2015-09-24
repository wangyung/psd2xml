#include <gtest/gtest.h>
#include <lib/psd_file.h>

class PsdFileHighLevelTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        file1 = new PsdFile("vectors/a.psd");
        file2 = new PsdFile("vectors/clock_original.psd");
    }

    virtual void TearDown()
    {
        delete file1;
        file1 = NULL;
        delete file2;
        file2 = NULL;
    }

    unsigned int getGroupCount(PsdLayers layers)
    {
        int groupCount = 0;
        vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
        for (; iter != end; iter++) {
            shared_ptr<PsdLayer> layer = *iter;
            if (layer.get() != NULL)
            {
                if (layer->getType() == PsdLayer::LayerTypeGroup) {
                    groupCount++;
                    groupCount += getGroupCount(((PsdLayerGroup *) layer.get())->getLayers());
                }
            }
        }

        return groupCount;
    }

    unsigned int getLayerCount(PsdLayers layers)
    {
        int layerCount= 0;
        vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
        for (; iter != end; iter++) {
            shared_ptr<PsdLayer> layer = *iter;
            if (layer.get() != NULL)
            {
                if (layer->getType() == PsdLayer::LayerTypeGroup) {
                    layerCount += getLayerCount(((PsdLayerGroup *) layer.get())->getLayers());
                }
                else
                    layerCount++;
            }
        }

        return layerCount;
    }

    PsdFile *file1;
    PsdFile *file2;
    PsdFile *file3; //reserve for later

};

TEST_F(PsdFileHighLevelTest, verifyOpenFile) {
    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());
}

TEST_F(PsdFileHighLevelTest, verifyDoubleOpenFile) {
    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());

    ASSERT_EQ(PsdStatusFileAlreadyOpened, file1->open());
    ASSERT_EQ(PsdStatusFileAlreadyOpened, file2->open());
}

TEST_F(PsdFileHighLevelTest, verifyColorMode) {
    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());

    ASSERT_EQ(PsdFile::ColorModeRGB, file1->getColorMode());
    ASSERT_EQ(PsdFile::ColorModeRGB, file2->getColorMode());
}

TEST_F(PsdFileHighLevelTest, verifyDimensions) {

    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());

    ASSERT_EQ(454, file1->getWidth());
    ASSERT_EQ(340, file1->getHeight());

    ASSERT_EQ(540, file2->getWidth());
    ASSERT_EQ(960, file2->getHeight());
}

TEST_F(PsdFileHighLevelTest, verifyLayerCount) {

    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());

    //file1 has 8 layers
    ASSERT_EQ(8, getLayerCount(file1->getLayers()));
    //file2 has 25 layers
    ASSERT_EQ(25, getLayerCount(file2->getLayers()));
}

TEST_F(PsdFileHighLevelTest, verifyGroupCount) {

    ASSERT_EQ(PsdStatusOK, file1->open());
    ASSERT_EQ(PsdStatusOK, file2->open());

    //file1 has 0 groups
    ASSERT_EQ(3, getGroupCount(file1->getLayers()));
    //file2 has 0 groups
    ASSERT_EQ(0, getGroupCount(file2->getLayers()));
}
