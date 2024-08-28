#include "RequestWorker.h"


RequestWorker::RequestWorker(QSslConfiguration ssl_configuration, qintptr socket, RequestWorkerParams params, bool is_s3_storage)
    : QRunnable()
    , ssl_configuration_(ssl_configuration)
	, socket_(socket)
    , params_(params)
    , is_s3_storage_(is_s3_storage)
	, is_terminated_(false)
{
}

void RequestWorker::run()
{
    QSslSocket *ssl_socket = new QSslSocket();

    try
    {
        ssl_socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
        if (!ssl_socket)
        {
            Log::error("Could not create a socket: " + ssl_socket->errorString());
            return;
        }
        ssl_socket->setSslConfiguration(ssl_configuration_);

        if (!ssl_socket->setSocketDescriptor(socket_))
        {
            Log::error("Could not set a socket descriptor: " + ssl_socket->errorString());
            return;
        }
    }
    catch (...)
    {
        Log::error("Could not configure the socket");
        return;
    }

	try
	{
        ssl_socket->startServerEncryption();

		QByteArray all_request_parts;

		if (!ssl_socket->isOpen())
		{
			Log::error("Could not open the socket for data exchange: " + ssl_socket->errorString());
			return;
		}

		bool finished_reading_headers = false;
		bool finished_reading_body = false;
		qint64 request_headers_size = 0;
		qint64 request_body_size = 0;

        while (ssl_socket->waitForReadyRead(params_.socket_read_timeout))
		{
            ssl_socket->waitForEncrypted(params_.socket_encryption_timeout);

			if (!ssl_socket->isEncrypted() || (ssl_socket->state() == QSslSocket::SocketState::UnconnectedState))
			{
				Log::error("Connection cannot be continued: " + ssl_socket->errorString());
				closeConnection(ssl_socket);
				return;
			}

			while(ssl_socket->bytesAvailable())
			{
				QByteArray line = ssl_socket->readLine();
				if ((!finished_reading_headers) && (line.toLower().startsWith("content-length")))
				{
					QList<QByteArray> header_parts = line.trimmed().split(':');
					if (header_parts.size() > 1) request_body_size = header_parts[1].toLongLong();
				}

				all_request_parts.append(line);
				if ((line.trimmed().size() == 0) && (!finished_reading_headers))
				{
					finished_reading_headers = true;
					request_headers_size = all_request_parts.size();
				}

				if ((finished_reading_headers) && ((all_request_parts.size() - request_headers_size) >= request_body_size))
				{
					finished_reading_body = true;
					break;
				}

			}

			if (finished_reading_body) break;
		}

		if (all_request_parts.size() == 0)
		{
			sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, ContentType::TEXT_PLAIN, "Request could not be processed"));
			Log::error("Was not able to read from the socket. Exiting. " + ssl_socket->errorString());
			return;
		}

		HttpRequest parsed_request;
		RequestParser *parser = new RequestParser();
		try
		{
			parsed_request = parser->parse(&all_request_parts);
		}
		catch (Exception& e)
		{
			Log::error("Could not parse the request: " + e.message());
			sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::BAD_REQUEST, ContentType::TEXT_HTML, e.message()));
			return;
		}

		ContentType error_type = HttpUtils::detectErrorContentType(parsed_request.getHeaderByName("User-Agent"));

		// Process the request based on the endpoint info
		Endpoint current_endpoint = EndpointManager::getEndpointByUrlAndMethod(parsed_request.getPath(), parsed_request.getMethod());
		if (current_endpoint.action_func == nullptr)
		{
			sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::BAD_REQUEST, error_type, "This action cannot be processed"));
			return;
		}

		try
		{
			EndpointManager::validateInputData(&current_endpoint, parsed_request);
		}
		catch (ArgumentException& e)
		{
            Log::warn(EndpointManager::formatResponseMessage(parsed_request, "Parameter validation has failed: " + e.message()));
            sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::BAD_REQUEST, error_type, EndpointManager::formatResponseMessage(parsed_request, e.message())));
			return;
		}

		QString user_token = EndpointManager::getTokenIfAvailable(parsed_request);
		QString user_info;
		if (!user_token.isEmpty())
		{
			Session user_session = SessionManager::getSessionBySecureToken(user_token);
			if (!user_session.isEmpty())
            {
                user_info = " - requested by " + user_session.user_login + " (" + user_session.user_name + ")";
			}
		}
		QString client_type = "Unknown client";
		if (!parsed_request.getHeaderByName("User-Agent").isEmpty())
		{
			if (parsed_request.getHeaderByName("User-Agent")[0].toLower().indexOf("igv") > -1)
			{
				client_type = "IGV";
			}
			else if ((parsed_request.getHeaderByName("User-Agent")[0].toLower().indexOf("gsvar") > -1) || (parsed_request.getHeaderByName("User-Agent")[0].toLower().indexOf("qt") > -1))
			{
				client_type = "GSvar";
			}
			else
			{
				client_type = "Browser";
			}
		}
		client_type = " - " + client_type;

		if (current_endpoint.authentication_type != AuthType::NONE)
		{
			HttpResponse auth_response;

			if (current_endpoint.authentication_type == AuthType::HTTP_BASIC_AUTH) auth_response = EndpointManager::getBasicHttpAuthStatus(parsed_request);
			if (current_endpoint.authentication_type == AuthType::USER_TOKEN) auth_response = EndpointManager::getUserTokenAuthStatus(parsed_request);
			if (current_endpoint.authentication_type == AuthType::DB_TOKEN) auth_response = EndpointManager::getDbTokenAuthStatus(parsed_request);

			if (auth_response.getStatus() != ResponseStatus::OK)
            {
                Log::error(EndpointManager::formatResponseMessage(parsed_request, "Token check failed: response code " + QString::number(HttpUtils::convertResponseStatusToStatusCodeNumber(auth_response.getStatus())) + user_info + client_type));
				sendEntireResponse(ssl_socket, auth_response);
				return;
			}
		}

		HttpResponse (*endpoint_action_)(const HttpRequest& request) = current_endpoint.action_func;
		HttpResponse response;

		try
		{
			response = (*endpoint_action_)(parsed_request);
		}
		catch (Exception& e)
        {
            Log::error(EndpointManager::formatResponseMessage(parsed_request, "Error while executing an action: " + e.message()));
            sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, error_type, EndpointManager::formatResponseMessage(parsed_request, "Could not process endpoint action: " + e.message())));
			return;
		}

        Log::info(EndpointManager::formatResponseMessage(parsed_request, current_endpoint.comment + user_info + client_type));

		if (response.isStream())
        {
            Log::info(EndpointManager::formatResponseMessage(parsed_request, "Initiating a stream: " + response.getFilename() + user_info + client_type));

			if (response.getFilename().isEmpty())
            {
                QString error_message = EndpointManager::formatResponseMessage(parsed_request, "Streaming request contains an empty file name");
				Log::error(error_message + user_info + client_type);
				sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::NOT_FOUND, error_type, error_message));
				return;
			}

            Log::info("Init S3 object");
            QString s3_file_name = response.getFilename().replace("/media/storage-500/", "");
            Log::info(s3_file_name);
            // AwsS3File aws_file();


            Aws::SDKOptions options;
            Aws::InitAPI(options);
            {




            AwsS3File aws_file("gsvars3storage", s3_file_name);

            // QFileInfo file_info = QFileInfo(response.getFilename());
            // QSharedPointer<QFile> streamed_file = QSharedPointer<QFile>(new QFile(response.getFilename()));

            Log::info("Work with S3 object");
            if (!aws_file.exists())
            {
                QString error_message = EndpointManager::formatResponseMessage(parsed_request, "Requested file does not exist: " + response.getFilename());
                Log::error(error_message + user_info + client_type);
                sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::NOT_FOUND, error_type, error_message));
                return;
            }
            Log::info("After work with S3 object");

   //          if (!streamed_file->exists())
   //          {
   //              QString error_message = EndpointManager::formatResponseMessage(parsed_request, "Requested file does not exist: " + response.getFilename());
            // 	Log::error(error_message + user_info + client_type);
            // 	sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::NOT_FOUND, error_type, error_message));
            // 	return;
            // }

            // Aws::IOStream& out_stream = aws_file->stream();
            // // streamed_file->open(QFile::ReadOnly);
            // if (!out_stream)
            // {
            //     QString error_message = EndpointManager::formatResponseMessage(parsed_request, "Could not open a file for streaming: " + response.getFilename());
            //     Log::error(error_message + user_info + client_type);
            //     sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, error_type, error_message));
            //     return;
            // }
   //          if (!streamed_file->open(QFile::ReadOnly))
   //          {
   //              QString error_message = EndpointManager::formatResponseMessage(parsed_request, "Could not open a file for streaming: " + response.getFilename());
            // 	Log::error(error_message + user_info + client_type);
            // 	sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, error_type, error_message));
            // 	return;
            // }

   //          if (!streamed_file->isOpen())
   //          {
   //              QString error_message = EndpointManager::formatResponseMessage(parsed_request, "File is not open: " + response.getFilename());
            // 	Log::error(error_message + user_info + client_type);
            // 	sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, error_type, error_message));
            // 	return;
            // }

			sendResponseDataPart(ssl_socket, response.getStatusLine());
			sendResponseDataPart(ssl_socket, response.getHeaders());

			quint64 pos = 0;
            // quint64 file_size = streamed_file->size();
            Log::info("Getting the size of the object");
            quint64 file_size = aws_file.size();
            Log::info(QString::number(file_size));
			bool transfer_encoding_chunked = false;

			if (!parsed_request.getHeaderByName("Transfer-Encoding").isEmpty())
			{
				if (parsed_request.getHeaderByName("Transfer-Encoding")[0].toLower() == "chunked")
				{
					transfer_encoding_chunked = true;
				}
			}

			long chunk_size = STREAM_CHUNK_SIZE;
            // QByteArray data;
			QList<ByteRange> ranges = response.getByteRanges();
			int ranges_count = ranges.count();

            if (ranges_count>0)
            {
                Log::info(EndpointManager::formatResponseMessage(parsed_request, QString::number(ranges_count) + " range(-s) found in request headers: " + response.getFilename() + user_info + client_type));
            }

            // Range request
			for (int i = 0; i < ranges_count; ++i)
            {
                Log::info(EndpointManager::formatResponseMessage(parsed_request, "Byte range [" + QString::number(ranges[i].start) + ", " + QString::number(ranges[i].end) + "] from " + QString::number(file_size) + " bytes in total: " + response.getFilename() + user_info + client_type));
                chunk_size = STREAM_CHUNK_SIZE;
                pos = ranges[i].start;
				if (ranges_count > 1)
				{
					sendResponseDataPart(ssl_socket, "--"+response.getBoundary()+"\r\n");
					sendResponseDataPart(ssl_socket, "Content-Type: application/octet-stream\r\n");
					sendResponseDataPart(ssl_socket, "Content-Range: bytes " + QByteArray::number(ranges[i].start) + "-" + QByteArray::number(ranges[i].end) + "/" + QByteArray::number(file_size) + "\r\n");
					sendResponseDataPart(ssl_socket, "\r\n");
				}

                if ((is_terminated_) || (ssl_socket->state() == QSslSocket::SocketState::UnconnectedState) || (ssl_socket->state() == QSslSocket::SocketState::ClosingState))
                {
                    Log::info(EndpointManager::formatResponseMessage(parsed_request, "Range streaming request process has been terminated: " + response.getFilename() + user_info + client_type));
                    return;
                }

                if (ranges[i].start >= (file_size-1)) break;


                Log::info("CHUNK " + QString::number(STREAM_CHUNK_SIZE));
                Log::info("Start a stream");

                Aws::Auth::AWSCredentials credentials;
                credentials.SetAWSAccessKeyId(Settings::string("aws_access_key_id", true).toUtf8().constData());
                credentials.SetAWSSecretKey(Settings::string("aws_secret_access_key", true).toUtf8().constData());


                Aws::Client::ClientConfiguration clientConfig;
                clientConfig.region = Aws::Region::EU_CENTRAL_1;;
                std::shared_ptr<Aws::S3::S3Client> s3Client = Aws::MakeShared<Aws::S3::S3Client>("AwsS3File", credentials, nullptr, clientConfig);
                QString s3_file_name = response.getFilename().replace("/media/storage-500/", "");
                Aws::S3::Model::GetObjectRequest object_request;
                object_request.SetBucket("gsvars3storage");
                object_request.SetKey(s3_file_name.toUtf8().data());

                Aws::String range = "bytes=" + std::to_string(ranges[i].start) + "-" + std::to_string(ranges[i].end);
                object_request.SetRange(range);

                auto get_object_outcome = s3Client->GetObject(object_request);

                if (get_object_outcome.IsSuccess()) {
                    Aws::IOStream& stream = get_object_outcome.GetResultWithOwnership().GetBody();
                    std::size_t chunkSize = STREAM_CHUNK_SIZE;
                    std::vector<char> buffer(chunkSize);

                    while (stream.good())
                    {

                        stream.read(buffer.data(), chunkSize);  // Read a chunk of data
                        std::streamsize bytesRead = stream.gcount();  // Get the actual number of bytes read

                        if (bytesRead > 0)
                        {
                            QByteArray byteArray;
                            byteArray.append(buffer.data(), static_cast<int>(bytesRead));
                            sendResponseDataPart(ssl_socket, byteArray);
                        }
                    }

                    if (stream.eof())
                    {
                        std::cout << "End of stream reached." << std::endl;
                    } else if (stream.fail())
                    {
                        std::cerr << "Stream read failed." << std::endl;
                    } else if (stream.bad())
                    {
                        std::cerr << "Stream is in a bad state." << std::endl;
                    }
                }


				if (is_terminated_) return;
				if (ranges_count > 1) sendResponseDataPart(ssl_socket, "\r\n");
				if ((i == (ranges_count-1)) && (ranges_count > 1))
				{
					sendResponseDataPart(ssl_socket, "--" + response.getBoundary() + "--\r\n");
				}

			}

            Log::info("Regular stream");
			// Regular stream
			if (ranges_count == 0)
			{
                Log::info("CHUNK " + QString::number(STREAM_CHUNK_SIZE));
                Log::info("Start a stream");

                Aws::Auth::AWSCredentials credentials;
                credentials.SetAWSAccessKeyId(Settings::string("aws_access_key_id", true).toUtf8().constData());
                credentials.SetAWSSecretKey(Settings::string("aws_secret_access_key", true).toUtf8().constData());


                Aws::Client::ClientConfiguration clientConfig;
                clientConfig.region = Aws::Region::EU_CENTRAL_1;;
                std::shared_ptr<Aws::S3::S3Client> s3Client = Aws::MakeShared<Aws::S3::S3Client>("AwsS3File", credentials, nullptr, clientConfig);
                QString s3_file_name = response.getFilename().replace("/media/storage-500/", "");
                Aws::S3::Model::GetObjectRequest object_request;
                object_request.SetBucket("gsvars3storage");
                object_request.SetKey(s3_file_name.toUtf8().data());

                auto get_object_outcome = s3Client->GetObject(object_request);

                if (get_object_outcome.IsSuccess()) {
                    Aws::IOStream& stream = get_object_outcome.GetResultWithOwnership().GetBody();
                    std::size_t chunkSize = STREAM_CHUNK_SIZE;
                    std::vector<char> buffer(chunkSize);

                    while (stream.good())
                    {

                        stream.read(buffer.data(), chunkSize);  // Read a chunk of data
                        std::streamsize bytesRead = stream.gcount();  // Get the actual number of bytes read

                        if (bytesRead > 0)
                        {
                            QByteArray byteArray;
                            byteArray.append(buffer.data(), static_cast<int>(bytesRead));
                            sendResponseDataPart(ssl_socket, byteArray);
                        }
                    }

                    if (stream.eof())
                    {
                        std::cout << "End of stream reached." << std::endl;
                    } else if (stream.fail())
                    {
                        std::cerr << "Stream read failed." << std::endl;
                    } else if (stream.bad())
                    {
                        std::cerr << "Stream is in a bad state." << std::endl;
                    }
                }

			}

            // streamed_file->close();

			// Should be used for chunked transfer (without content-lenght)
			if (transfer_encoding_chunked)
			{
				sendResponseDataPart(ssl_socket, "0\r\n");
				sendResponseDataPart(ssl_socket, "\r\n");
			}

			closeConnection(ssl_socket);
			return;

            }
            Aws::ShutdownAPI(options);
		}
		else if (!response.getPayload().isNull())
		{
			sendEntireResponse(ssl_socket, response);
			return;
		}
		// Returns headers with file size without fetching the file itself
		else if (parsed_request.getMethod() == RequestMethod::HEAD)
		{
			sendEntireResponse(ssl_socket, response);
			return;
		}
		else if ((response.getPayload().isNull()) && (parsed_request.getHeaders().contains("range")))
		{
			// Fetching non-existing range (e.g. larger than the file itself)


            QString s3_file_name = response.getFilename().replace("/media/storage-500/", "");
            AwsS3File* aws_file = new AwsS3File("gsvars3storage", s3_file_name.toUtf8().data());


			BasicResponseData response_data;
			response_data.filename = response.getFilename();
            response_data.file_size = aws_file->size();
			response.setStatus(ResponseStatus::RANGE_NOT_SATISFIABLE);
			response.setRangeNotSatisfiableHeaders(response_data);
			sendEntireResponse(ssl_socket, response);
			return;
		}
		else if (response.getPayload().isNull())
        {
            // send empty response
            Log::warn("Sending an empty response: " + QString::number(response.getStatusCode()) + user_info + client_type);
            sendEntireResponse(ssl_socket, response);
            return;
		}

        QString error_message = EndpointManager::formatResponseMessage(parsed_request, "The requested resource does not exist: " + parsed_request.getPath() + ". Check the URL and try again");
		Log::error(error_message + user_info + client_type);
		sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::NOT_FOUND, error_type, error_message));
	}
	catch (...)
    {
        QString error_message = "Unexpected error inside the request worker. See logs for more details";
		Log::error(error_message);
		sendEntireResponse(ssl_socket, HttpResponse(ResponseStatus::INTERNAL_SERVER_ERROR, ContentType::TEXT_PLAIN, error_message));
	}
}

QString RequestWorker::intToHex(const int& input)
{
	return QString("%1").arg(input, 10, 16, QLatin1Char('0')).toUpper();
}

void RequestWorker::closeConnection(QSslSocket* socket)
{
	is_terminated_ = true;

    if ((socket->state() == QSslSocket::SocketState::UnconnectedState) || (socket->state() == QSslSocket::SocketState::ClosingState))
    {
        return;
	}

    if (socket->bytesToWrite()) socket->waitForBytesWritten(params_.socket_write_timeout); //5000
    socket->abort();
}

void RequestWorker::sendResponseDataPart(QSslSocket* socket, const QByteArray& data)
{
	if (socket->state() != QSslSocket::SocketState::UnconnectedState)
	{
        qDebug() << data;
        socket->write(data, data.size());
        if (socket->bytesToWrite()) socket->waitForBytesWritten(params_.socket_write_timeout);
	}
}

void RequestWorker::sendEntireResponse(QSslSocket* socket, const HttpResponse& response)
{
	if (response.getStatusCode() > 200) Log::warn("The server returned " + QString::number(response.getStatusCode()) + " - " + HttpUtils::convertResponseStatusToReasonPhrase(response.getStatus()));
	if (socket->state() != QSslSocket::SocketState::UnconnectedState)
	{
		socket->write(response.getStatusLine());
		socket->write(response.getHeaders());
		socket->write(response.getPayload());
	}
	closeConnection(socket);
}

bool RequestWorker::fileInS3BucketExists(const Aws::String& bucket_name, const Aws::String& object_key)
{

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(Settings::string("aws_access_key_id", true).toUtf8().constData());
    credentials.SetAWSSecretKey(Settings::string("aws_secret_access_key", true).toUtf8().constData());

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::EU_CENTRAL_1;

    Aws::S3::S3Client s3_client(credentials, nullptr, clientConfig);
    Aws::S3::Model::HeadObjectRequest object_request;
    object_request.SetBucket(bucket_name);
    object_request.SetKey(object_key);

    auto get_object_outcome = s3_client.HeadObject(object_request);
    get_object_outcome.IsSuccess();
    if (get_object_outcome.IsSuccess())
    {
        return true;
    }

    return false;
}

long long RequestWorker::getS3BucketFileSize(const Aws::String& bucket_name, const Aws::String& object_key)
{

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(Settings::string("aws_access_key_id", true).toUtf8().constData());
    credentials.SetAWSSecretKey(Settings::string("aws_secret_access_key", true).toUtf8().constData());

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::EU_CENTRAL_1;

    Aws::S3::S3Client s3_client(credentials, nullptr, clientConfig);
    Aws::S3::Model::HeadObjectRequest object_request;
    object_request.SetBucket(bucket_name);
    object_request.SetKey(object_key);

    auto get_object_outcome = s3_client.HeadObject(object_request);
    if (get_object_outcome.IsSuccess())
    {
        return get_object_outcome.GetResultWithOwnership().GetContentLength();
    }

    return -1;
}

void RequestWorker::getS3BucketFileInChunks(const Aws::String& bucket_name, const Aws::String& object_key, QSslSocket* ssl_socket)
{
    std::size_t chunk_size = STREAM_CHUNK_SIZE;

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(Settings::string("aws_access_key_id", true).toUtf8().constData());
    credentials.SetAWSSecretKey(Settings::string("aws_secret_access_key", true).toUtf8().constData());

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::EU_CENTRAL_1;

    Aws::S3::S3Client s3_client(credentials, nullptr, clientConfig);

    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(bucket_name);
    object_request.SetKey(object_key);

    auto get_object_outcome = s3_client.GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        auto& retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();

        // std::ofstream output_file(output_file_path, std::ios::binary);
        // if (!output_file.is_open()) {
        //     std::cerr << "Failed to open local file for writing." << std::endl;
        //     return;
        // }

        std::vector<char> buffer(chunk_size);  // Buffer to hold chunks of data
        while (retrieved_file) {
            retrieved_file.read(buffer.data(), chunk_size);  // Read up to chunk_size bytes
            std::streamsize bytes_read = retrieved_file.gcount();  // Get the actual number of bytes read

            sendResponseDataPart(ssl_socket, QByteArray(buffer.data(), buffer.size()));

            // output_file.write(buffer.data(), bytes_read);  // Write the bytes to the file
        }

        // output_file.close();
        // std::cout << "Range downloaded successfully to: " << output_file_path << std::endl;
    } else {
        std::cerr << "Failed to download range from S3. Error: "
                  << get_object_outcome.GetError().GetMessage() << std::endl;
    }

}
