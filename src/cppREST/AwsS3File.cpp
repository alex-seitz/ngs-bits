#include "AwsS3File.h"

AwsS3File::AwsS3File(QString bucket_name, QString key_name)
    : bucket_name_(bucket_name)
    , key_name_(key_name)
    , access_key_id_(Settings::string("aws_access_key_id", true).toUtf8().data())
    , secret_key_(Settings::string("aws_secret_access_key", true).toUtf8().data())
    , region_(Aws::Region::EU_CENTRAL_1)
{
    // Aws::SDKOptions options;
    // Aws::InitAPI(options);

    // Log::info("Aws-> reading config");



    // Log::info("Aws-> init options");


    // Log::info("Aws-> setting credentials");
    // Aws::Auth::AWSCredentials credentials;
    // credentials.SetAWSAccessKeyId(access_key_id_);
    // credentials.SetAWSSecretKey(secret_key_);

    // // Log::info("Aws-> setting region");
    // Aws::Client::ClientConfiguration clientConfig;
    // clientConfig.region = region_;

    // Log::info("Aws-> creating a client");
    // s3_client_ = Aws::S3::S3Client(credentials, nullptr, clientConfig);

    // // s3_client_ = Aws::S3::S3Client(clientConfig);

    // Log::info("key_name_constr");
    // Log::info(key_name_);

    Aws::Auth::AWSCredentials credentials(access_key_id_, secret_key_);

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::EU_CENTRAL_1;;
    s3Client = Aws::MakeShared<Aws::S3::S3Client>("AwsS3File", credentials, nullptr, clientConfig);
}

bool AwsS3File::exists()
{
    Aws::S3::Model::HeadObjectRequest object_request;
    object_request.SetBucket(bucket_name_.toUtf8().data());
    object_request.SetKey(key_name_.toUtf8().data());
    Log::info("key_name_");
    Log::info(key_name_);
    auto get_object_outcome = s3Client->HeadObject(object_request);
    Log::error(QString::fromStdString(get_object_outcome.GetError().GetMessage()));
    if (get_object_outcome.IsSuccess()) return true;
    // Aws::ShutdownAPI(options);

    return false;
}

quint64 AwsS3File::size()
{
    Aws::S3::Model::HeadObjectRequest object_request;
    object_request.SetBucket(bucket_name_.toUtf8().data());
    object_request.SetKey(key_name_.toUtf8().data());
    auto get_object_outcome = s3Client->HeadObject(object_request);

    if (get_object_outcome.IsSuccess())
    {
        return get_object_outcome.GetResultWithOwnership().GetContentLength();
    }

   THROW(Exception, "Could not access the size of a S3 object: " + QString(key_name_));
}

Aws::IOStream& AwsS3File::stream()
{
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(bucket_name_.toUtf8().data());
    object_request.SetKey(key_name_.toUtf8().data());

    auto get_object_outcome = s3_client_.GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        return get_object_outcome.GetResultWithOwnership().GetBody();
    }

    THROW(Exception, "Could not access the S3 object: " + QString(key_name_));
}

Aws::IOStream& AwsS3File::stream(long long start, long long end)
{
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(bucket_name_.toUtf8().data());
    object_request.SetKey(key_name_.toUtf8().data());

    Aws::String range = "bytes=" + std::to_string(start) + "-" + std::to_string(end);
    object_request.SetRange(range);

    auto get_object_outcome = s3_client_.GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        return get_object_outcome.GetResultWithOwnership().GetBody();
    }

    THROW(Exception, "Could not access the S3 object [" + QString::number(start) + ", " + QString::number(end) + "]: " + QString(key_name_));
}


QByteArray AwsS3File::readChunk(long long start, long long end)
{
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(bucket_name_.toUtf8().data());
    object_request.SetKey(key_name_.toUtf8().data());

    Aws::String range = "bytes=" + std::to_string(start) + "-" + std::to_string(end);
    object_request.SetRange(range);

    auto get_object_outcome = s3_client_.GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        Aws::IOStream& out_regular_stream = get_object_outcome.GetResultWithOwnership().GetBody();

        out_regular_stream.seekg(0, std::ios::end);
        std::streamsize size = out_regular_stream.tellg();

        // Move the stream back to the beginning
        out_regular_stream.seekg(0, std::ios::beg);

        // Create a QByteArray with the same size as the stream content
        QByteArray byteArray(size, 0);
        // Log::info("Buffer size = " + QString::number(byteArray.size()));
        Log::info(key_name_);

        // Read the content of the stream into the QByteArray's buffer
        if (size > 0) {
            out_regular_stream.read(byteArray.data(), size);
        }

        return byteArray;
    }

    THROW(Exception, "Could not access the S3 object [" + QString::number(start) + ", " + QString::number(end) + "]: " + QString(key_name_));
}

void AwsS3File::setAccessKeyId(const char* access_key_id)
{
    access_key_id_ = access_key_id;
}

void AwsS3File::setSecretKey(const char* secret_key)
{
    secret_key_ = secret_key;
}

void AwsS3File::setRegion(const char* region)
{
    region_ = region;
}

void AwsS3File::setBucketName(const char* bucket_name)
{
    bucket_name_ = bucket_name;
}

void AwsS3File::setKeyName(const char* key_name)
{
    key_name_ = key_name;
}
