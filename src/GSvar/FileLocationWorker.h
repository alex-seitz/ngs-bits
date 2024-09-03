#ifndef FILELOCATIONWORKER_H
#define FILELOCATIONWORKER_H

#include <QRunnable>
#include "GlobalServiceProvider.h"

class FileLocationWorker
    : public QObject
    , public QRunnable
{
    Q_OBJECT

public:
    FileLocationWorker(PathType file_type, AnalysisType analysis_type, FileLocationList& data);
    void run();

signals:
    void bamsReceived(FileLocationList locations);
    void bafsReceived(FileLocationList locations);
    void vcfReceived(FileLocation location);
    void svReceived(FileLocation location);
    void cnvsReceived(FileLocationList locations);
    void mantaReceived(FileLocationList locations);

protected:
    PathType file_type_;
    AnalysisType analysis_type_;
    FileLocationList& data_;
};

#endif // FILELOCATIONWORKER_H
