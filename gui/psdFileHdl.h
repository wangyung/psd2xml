#ifndef PSD_FILE_HANDLE_H
#define PSD_FILE_HANDLE_H
#include "../lib/psd_file.h"
#include <string>
#include <QStringList>

class psdHandle
{
public:
    psdHandle();
    ~psdHandle();
    void CreateHandler(string p_StrPath);
    bool PrepareItem(QStringList& p_rStrList,QStringList& p_rImageList);

private:
    PsdFile *mPsdFile;

    void concate(QString& p_rStr,const char* p_pcChar ,int p_nLayer);
    void setLayer(QStringList& p_rStrList,PsdLayers layers,QStringList& p_rImageList, int level = 0);
    void layerImage(PsdLayers layers,QStringList& p_rStrList);

};

#endif // PSD_FILE_HANDLE_H
