#ifndef FILELOCATIONWORKER_H
#define FILELOCATIONWORKER_H

#include <QRunnable>
#include "GlobalServiceProvider.h"
#include "IgvDialog.h"

class FileLocationWorker
    : public QObject
    , public QRunnable
{
    Q_OBJECT

public:
    FileLocationWorker(IgvDialog& igv_dialog, PathType file_type, AnalysisType analysis_type);
    void run();


protected:
    IgvDialog& igv_dialog_;
    PathType file_type_;
    AnalysisType analysis_type_;
};

#endif // FILELOCATIONWORKER_H
