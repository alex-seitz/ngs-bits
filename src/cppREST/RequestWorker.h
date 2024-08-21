#ifndef REQUESTWORKER_H
#define REQUESTWORKER_H

#include "cppREST_global.h"
#include <QRunnable>
#include <QSslSocket>
#include <QSslError>
#include <QSslConfiguration>
#include <QHostAddress>
#include <QList>

#include "Log.h"
#include "Exceptions.h"
#include "HttpResponse.h"
#include "RequestParser.h"
#include "EndpointManager.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <QFile>
#include <QDebug>
#include <fstream>

class CPPRESTSHARED_EXPORT RequestWorker
    : public QRunnable
{

public:
    explicit RequestWorker(QSslConfiguration ssl_configuration, qintptr socket, RequestWorkerParams params, bool is_s3_storage);
    void run() override;

private:
	const int STREAM_CHUNK_SIZE = 1024*10;
	QString intToHex(const int &input);

	void closeConnection(QSslSocket* socket);
    void sendResponseDataPart(QSslSocket *socket, const QByteArray& data);
    void sendEntireResponse(QSslSocket *socket, const HttpResponse& response);

    bool fileInS3BucketExists(const Aws::String& bucket_name, const Aws::String& object_key);
    long long getS3BucketFileSize(const Aws::String& bucket_name, const Aws::String& object_key);
    void getS3BucketFileInChunks(const Aws::String& bucket_name, const Aws::String& object_key, QSslSocket* socket);

	QSslConfiguration ssl_configuration_;
	qintptr socket_;
    RequestWorkerParams params_;
    bool is_s3_storage_;
	bool is_terminated_;
};

#endif // REQUESTWORKER_H
