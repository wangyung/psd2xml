#include "PsdHtml.h"

#define JQUERY_URI "http://ajax.googleapis.com/ajax/libs/jquery/1.7.0/jquery.min.js"

class PsdHtml
{
    int mOptions;
    scoped_ptr<PsdFile> mPsdFile;
    FILE *mOutStream;
    string mBaseName;
    string mOutputRootDir;
    string mOutputFilesDir;
    string mOutputFilesPath;
    uint32_t mLayersProcessed;
    uint32_t mGroupsProcessed;
    uint32_t mRowProcessed;
    vector<string> mFileNames;
    vector<string> mLayerNames;
    vector<string> mGroupNames;
    char mDimension[512];
    char mBackgroundOverview[512];

    //const char *mOutputDir;

public:
    PsdHtml(int argc, char **argv)
        : mOutStream(NULL)
        , mOptions(0)
    {
        if (argc < 2) {
            puts("Usage: psdhtml FILE.psd");
            exit(2);
        }

        if (!dumpHtml(argv[1]))
            exit(1);

        exit(0);
#if 0
        int c, longind, options = 0;
        struct option long_options[] = {
            {"help", no_argument, 0, 'h'}
        };

        while ((c = getopt_long(argc, argv, "o:h", long_options, &longind)) != -1) {
            switch (c) {
                case 0:
                    break;
                case 'o':
                    mOutputDir = optarg;
                    break;
                default:
                    exit(1);
            }
        }
#endif
    }
    ~PsdHtml()
    {
    }

public:
    bool dumpHtml(const char *filePath)
    {
        mPsdFile.reset(new PsdFile(filePath));
        if (mPsdFile->open() != PsdStatusOK)
            return false;

        const char *fileName, *ext;
        fileName = strrchr(filePath, PATH_SEP);
        if (!fileName)
            fileName = filePath;
        else
            fileName++;
        ext = strrchr(fileName, '.');
        if (ext)
            mBaseName.assign(fileName, (ptrdiff_t) ext - (ptrdiff_t) fileName);
        else
            mBaseName.assign(fileName);
        mOutputRootDir = mBaseName + "_html";
        mOutputFilesDir = mBaseName + ".files";
        mOutputFilesPath = mOutputRootDir + PATH_SEP + mOutputFilesDir;
        string htmlPath = mOutputRootDir + PATH_SEP + mBaseName + ".html";

        mkdir(mOutputRootDir.c_str());
        mkdir(mOutputFilesPath.c_str());

        mOutStream = fopen(htmlPath.c_str(), "w");
        if (!mOutStream) {
            fprintf(stderr, "Failed to open %s\n", htmlPath.c_str());
            return false;
        }

        printf("Source: %s\nOutput: %s\n", filePath, mOutputRootDir.c_str());

#if 0
        system(("rsync -aP --inplace --delete /home/jackie/htc/psdfile/html/static/ " + mOutputFilesPath + '/').c_str());
#endif

        fprintf(mOutStream,
                "<!DOCTYPE html>\n"
                "<html><head>"
                "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"%s\" />"
                "<style type=\"text/css\">"
                ".layer-canvas { width: %upx; height: %upx; }"
                "</style>"
                "<title>%s</title></head><body>"
                "<div id=\"toolbox\"><div id=\"go-toc\" class=\"button\"><a href=\"#toc\">TOC</a></div></div>"
                "<div id=\"scroll-content\">"
                "<h1>%s</h1>"
                ,
                (mOutputFilesDir + "/psdhtml_styles.css").c_str(),
                mPsdFile->getWidth(), mPsdFile->getHeight(),
                fileName,
                fileName
        );

        PsdLayers layers = mPsdFile->getLayers();
        if (layers && !layers->empty()) {
            mFileNames.reserve(layers->size());
            mLayerNames.reserve(layers->size());
            generateIndex();
            generateContent();
        }

        fprintf(mOutStream,
            "</div>"
            "<script type=\"text/javascript\" src=\"" JQUERY_URI "\"></script>"
            "<script type=\"text/javascript\" src=\"%s\"></script>"
            "</body></html>\n"
            ,
            (mOutputFilesDir + "/psdhtml.js").c_str()
        );

        fclose(mOutStream);
        mOutStream = NULL;
        return true;
    }

    void generateIndex()
    {
        mLayersProcessed = mGroupsProcessed = 0;
        fputs(
              "<ul id=\"toc\" class=\"toc-group\">"
              "<li><a href=\"#layer-name-__overview__.png\" id=\"toc-overview\">Overview</a></li>",
              mOutStream
        );
        generateIndex(mPsdFile->getLayers());
        fputs("</ul>", mOutStream);
    }

    void generateIndex(const PsdLayers &layers)
    {
        if (!layers || layers->empty())
            return;

        vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
        for (; iter != end; iter++) {
            shared_ptr<PsdLayer> layer = *iter;
            if (layer->getType() == PsdLayer::LayerTypeGroup) {
                mGroupsProcessed++;
                string name = toSafeHtmlString(layer->getName());
                mGroupNames.push_back(name);
                fprintf(mOutStream, "<li class=\"toc-group\">%s<ul>", name.c_str());
                generateIndex(((PsdLayerGroup *) layer.get())->getLayers());
                fputs("</ul></li>", mOutStream);
            } else {
                mLayersProcessed++;
                string fileName = toIdealFileName(layer->getName(), mLayersProcessed);
                mFileNames.push_back(fileName);
                string name = toSafeHtmlString(layer->getName());
                mLayerNames.push_back(name);
                fprintf(mOutStream, "<li><a href=\"#layer-name-%s\">%s</a></li>", fileName.c_str(), name.c_str());
            }
        }
    }

    void generateContent()
    {
        mLayersProcessed = mGroupsProcessed = mRowProcessed = 0;
        sprintf(mDimension, "%u x %u",
                mPsdFile->getWidth(), mPsdFile->getHeight());

        fputs("<div id=\"content-container\"><table id=\"content\"><tbody>", mOutStream);

        mBackgroundOverview[0] = 0;
        PsdLayers layers = mPsdFile->getLayers();
        PsdRect rect;
        rect.mLeft = rect.mTop = 0;
        rect.mRight = mPsdFile->getWidth();
        rect.mBottom = mPsdFile->getHeight();
        generateContentRow(PsdImageData::fromLayers(layers), rect, "Overview", "__overview__.png");

        sprintf(mBackgroundOverview,
                "<img src=\"%s/__overview__.png\" class=\"background-overview\" />",
                mOutputFilesDir.c_str());
        generateContent(layers, string());
        fputs("</tbody></table></div>", mOutStream);
    }

    void generateContent(const PsdLayers &layers, const string &path)
    {
        if (!layers || layers->empty())
            return;

        vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();

        for (; iter != end; iter++) {
            shared_ptr<PsdLayer> layer = *iter;
            if (layer->getType() == PsdLayer::LayerTypeGroup) {
                generateContent(((PsdLayerGroup *) layer.get())->getLayers(),
                                path + mGroupNames[mGroupsProcessed++] + " &gt; ");
            } else {
                generateContentRow(layer->getImageData(), layer->getAdjustedRect(),
                                   mLayerNames[mLayersProcessed].c_str(),
                                   mFileNames[mLayersProcessed].c_str(),
                                   path.c_str()
                );
                mLayersProcessed++;
            }
        }
    }

    void generateContentRow(const shared_ptr<PsdImageData> &imageData, const PsdRect &rect,
                            const char *title, const char *fileName, const char *layerPath = "")
    {
        const char *oddOrEven = ++mRowProcessed ? "row-odd" : "row-even";
        PsdStatus status = PsdStatusNoInit;

        fprintf(mOutStream,
                "<tr id=\"layer-name-%s\" class=\"layer-row %s\">"
                "<td class=\"layer-image-container\">"
                "<div class=\"layer-canvas-header\">%s%s</div>"
                ,
                fileName, oddOrEven,
                layerPath, title
        );

        if (imageData)
            status = imageData->storePNGFile((mOutputFilesPath + PATH_SEP + fileName).c_str());

        if (status == PsdStatusOK)
            fprintf(mOutStream,
                    "<div class=\"layer-canvas %s\">"
                    "%s"
                    "<img src=\"%s\" class=\"layer-image\" style=\"position: absolute; top: %upx; left: %upx;\" title=\"%s\" alt=\"%s\" />"
                    "</div>"
                    ,
                    *mBackgroundOverview ? "is-a-layer" : "background-grid",
                    mBackgroundOverview,
                    (mOutputFilesDir + '/' + fileName).c_str(),
                    rect.mTop, rect.mLeft,
                    fileName, title
            );
        else
            fputs("<div class=\"layer-no-image\">No Image</div>", mOutStream);

        fprintf(mOutStream,
                "</td>"
                "<td class=\"layer-info-container\">"
                "<div class=\"layer-name\">%s</div>"
                "<dl class=\"layer-properties\">"
                "<dt>position (left, top, right, bottom)</dt>"
                "<dd>(%u, %u, %u, %u)<dd>"
                "<dt>dimension (width x height)</dt>"
                "<dd>%u x %u @ %s</dd>"
                "</td>"
                "</tr>"
                ,
                title,
                rect.mLeft, rect.mTop, rect.mRight, rect.mBottom,
                rect.getWidth(), rect.getHeight(), mDimension
        );
    }

    static string toSafeHtmlString(const char *s)
    {
        string result;

        result.reserve(128);
        for (; *s; s++) {
            // replace the characters are not suitable for HTML text
            switch (*s) {
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '&':
                    result += "&amp;";
                    break;
                default:
                    result += *s;
                    break;
            }
        }

        return result;
    }

    static string toIdealFileName(const char *s, int i)
    {
        string result;
        char suffix[20];

        result.reserve(128);
        for (; *s; s++) {
            // replace the characters are not suitable for file name
            switch (*s) {
                case '/':
                case '\\':
                case ':':
                case '?':
                case '*':
                case '<':
                case '>':
                case '|':
                case '"':
                case '#':
                case '&':
                    result += '_';
                    break;
                default:
                    result += *s;
                    break;
            }
        }

        sprintf(suffix, ".%d.png", i);
        result += suffix;

        return result;
    }

    static bool mkdir(const char *dir)
    {
#ifndef _WIN32
        return ::mkdir(dir, 00777) == 0;
#else
        return !!CreateDirectoryA(dir, NULL);
#endif
    }
};


int main(int argc, char **argv)
{
#ifdef _WIN32
    wchar_t path[512], *p;
    GetModuleFileNameW(NULL, path, 512);
    p = wcsrchr(path, L'\\');
    if (p) {
        wcscpy(p + 1, L"libexec");
        SetDllDirectoryW(path);
    }
#endif

    PsdHtml h(argc, argv);
    return 0;
}
