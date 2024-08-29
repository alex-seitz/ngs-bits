#ifndef AWSS3FILE_H
#define AWSS3FILE_H

#include "cppREST_global.h"
#include <QObject>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Settings.h"
#include "Exceptions.h"
#include "Log.h"

class CPPRESTSHARED_EXPORT AwsS3File : public QObject
{
    Q_OBJECT

public:

    AwsS3File(QString bucket_name, QString key_name);

    bool exists();
    quint64 size();
    Aws::IOStream& stream();
    Aws::IOStream& stream(long long start, long long end);

    QByteArray readChunk(long long start, long long end);

    void setAccessKeyId(const char* access_key_id);
    void setSecretKey(const char* secret_key);
    void setRegion(const char* region);
    void setBucketName(const char* bucket_name);
    void setKeyName(const char* key_name);

private:
    Aws::S3::S3Client s3_client_;
    Aws::String access_key_id_;
    Aws::String secret_key_;
    Aws::String region_;
    QString bucket_name_;
    QString key_name_;

    bool exists_;
    quint64 size_;

    std::shared_ptr<Aws::S3::S3Client> s3Client;
};

#endif // AWSS3FILE_H
