#ifndef AUXILARY_H
#define AUXILARY_H

#include <QByteArray>
#include <QByteArrayList>
#include <QString>

//Analysis status
enum AnalysisStatus
{
    TO_BE_PROCESSED,
    TO_BE_WRITTEN,
    DONE
};

//Analysis data for worker thread
struct AnalysisJob
{
    QList<QByteArray> current_chunk;
    QList<QByteArray> current_chunk_processed;
    QByteArray annotation;
    QString error_message;

    //id used to keep the vcf file in order
    int chunk_id = 0;
    AnalysisStatus status = DONE;

    void clear()
    {
        status = DONE;

    }
};

#endif // AUXILARY_H


