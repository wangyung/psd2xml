#include "psdFileHdl.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <QtDebug>

psdHandle::psdHandle()
{
    mPsdFile=NULL;
}

psdHandle::~psdHandle()
{
    if(NULL!=mPsdFile)
    {
        mPsdFile->close();
        delete mPsdFile;
        mPsdFile=NULL;
    }
}

void
psdHandle::CreateHandler(string p_StrPath)
{
    mPsdFile = new PsdFile(p_StrPath.data());
    if(NULL!= mPsdFile)
    {
        mPsdFile->open();
       // _printLayers(mPsdFile->getLayers());
#ifndef _WIN32
        mkdir("exportedPng.d", 00777);
#else
        CreateDirectoryA("exportedPng.d", NULL);
#endif
    }
}

bool
psdHandle::PrepareItem(QStringList& p_rStrList,QStringList& p_rImageList)
{
    if(NULL == mPsdFile)
        return FALSE;
    if(NULL == mPsdFile->getLayers())
        return FALSE;

    setLayer(p_rStrList,mPsdFile->getLayers(),p_rImageList);

    return TRUE;

    /*items << tr("Widgets")
            << tr("  QWidget")
            << tr("    QWidget")
            << tr("  QDialog")
            << tr("Tools")
            << tr("  Qt Designer")
            << tr("  Qt Assistant");*/
}
/*************************************************************************/
void
psdHandle::concate(QString& p_rStr,const char* p_pcChar ,int p_nLayer)
{
    QString rStr,rTmpStr;

    if(0 == p_nLayer)
    {
        p_rStr = p_pcChar;
        return;
    }

    int nIdx=0;
    for(nIdx=0; nIdx<p_nLayer; nIdx++)
        rStr=rStr+" ";

    rTmpStr = p_pcChar;
    p_rStr=rStr+rTmpStr;

}

static char sFilePathPng[512] = "exportedPng.d/";
static char *sFileNamePng = sFilePathPng + sizeof("exportedPng.d/") - 1;

static
void setPngFileName(const char *name)
{
    char *p = sFileNamePng;
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
    strcpy(p, ".png");
}

void
psdHandle::layerImage(PsdLayers layers,QStringList& p_rStrList)
{
    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    PsdStatus status;
    for (; iter != end; iter++)
    {
        shared_ptr<PsdLayer> layer = *iter;
        if (layer->getType() == PsdLayer::LayerTypeGroup)
        {
            layerImage(((PsdLayerGroup *) layer.get())->getLayers(),p_rStrList);
        }
        else
        {
            shared_ptr<PsdImageData> imageData = layer->getImageData();
            if(NULL == imageData)
                continue;

            setPngFileName(layer->getName());
            if(PsdStatusOK == imageData->storePNGFile(sFilePathPng))
            {
                p_rStrList << imageData->getStoredPNGFilePath();
                //qDebug() << "Pic path:" << imageData->getStoredPNGFilePath();
            }
        }
    }
}

void
psdHandle::setLayer(QStringList& p_rStrList,PsdLayers layers, QStringList& p_rImageList, int level)
{
    if(NULL == layers)
        return;

    QString rResult;
    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    for (; iter != end; iter++)
    {
        shared_ptr<PsdLayer> layer = *iter;
        if (layer->getType() == PsdLayer::LayerTypeGroup)
        {
            concate(rResult,layer->getName(),level);
            //printf("%*s+ %s(%d)\n", level, "", layer->getName(),level);
            p_rStrList << rResult;
            setLayer(p_rStrList,((PsdLayerGroup *) layer.get())->getLayers(), p_rImageList,level + 2);
        }
        else
        {
            layerImage(layers,p_rImageList);
            concate(rResult,layer->getName(),level);
            p_rStrList << rResult;
            //printf("%*s- %s(%d)\n", level, "", layer->getName(),level);
        }
    }
}
