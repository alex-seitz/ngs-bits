#include <QApplication>
#include <QMessageBox>
#include "DatabaseServiceRemote.h"
#include "Settings.h"
#include "ApiCaller.h"

DatabaseServiceRemote::DatabaseServiceRemote()
	: enabled_(true)
{
}

bool DatabaseServiceRemote::enabled() const
{
	return enabled_;
}

QString DatabaseServiceRemote::checkPassword(const QString user_name, const QString password) const
{
	checkEnabled(__PRETTY_FUNCTION__);
	return makePostApiCall("validate_credentials", RequestUrlParams(), QString("name="+user_name+"&password="+password).toLocal8Bit(), false);
}

BedFile DatabaseServiceRemote::processingSystemRegions(int sys_id, bool ignore_if_missing) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	BedFile output;
	RequestUrlParams params;
	params.insert("sys_id", QString::number(sys_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("ps_regions", params, ignore_if_missing);

	if ((reply.length() == 0) && (!ignore_if_missing))
	{
		THROW(Exception, "Could not get the processing system regions for " + QString::number(sys_id));
	}

	output = output.fromText(reply);

	return output;
}

BedFile DatabaseServiceRemote::processingSystemAmplicons(int sys_id, bool ignore_if_missing) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	BedFile output;
	RequestUrlParams params;
	params.insert("sys_id", QString::number(sys_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("ps_amplicons", params, ignore_if_missing);

	if ((reply.length() == 0) && (!ignore_if_missing))
	{
		THROW(Exception, "Could not get the processing system amplicons for " + QString::number(sys_id));
	}

	output = output.fromText(reply);

	return output;
}

GeneSet DatabaseServiceRemote::processingSystemGenes(int sys_id, bool ignore_if_missing) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	GeneSet output;
	RequestUrlParams params;
	params.insert("sys_id", QString::number(sys_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("ps_genes", params, ignore_if_missing);

	if ((reply.length() == 0) && (!ignore_if_missing))
	{
		THROW(Exception, "Could not get the processing system genes for " + QString::number(sys_id));
	}

	output = GeneSet::createFromText(reply);

	return output;
}

QStringList DatabaseServiceRemote::secondaryAnalyses(QString processed_sample_name, QString analysis_type) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	QStringList list;
	RequestUrlParams params;
	params.insert("ps_name", processed_sample_name.toLocal8Bit());
	params.insert("type", analysis_type.toLocal8Bit());
	QByteArray reply = makeGetApiCall("secondary_analyses", params, true);
	if (reply.length() == 0)
	{
		THROW(Exception, "Could not get the processing system genes for " + processed_sample_name);
	}

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonArray json_array = json_doc.array();
	QStringList analyses;
	for (int i = 0; i < json_array.count(); i++)
	{
		if (!json_array.at(i).isString()) break;
		list.append(json_array.at(i).toString());
	}

	return list;
}

FileLocation DatabaseServiceRemote::processedSamplePath(const QString& processed_sample_id, PathType type) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	FileLocation output;
	RequestUrlParams params;
	params.insert("ps_id", processed_sample_id.toLocal8Bit());
	params.insert("type", FileLocation::typeToString(type).toLocal8Bit());
	QByteArray reply = makeGetApiCall("processed_sample_path", params, true);

	if (reply.length() == 0)
	{
		THROW(Exception, "Could not get a GSvar file for the processed sample " + processed_sample_id);
	}

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonArray json_array = json_doc.array();
	QStringList analyses;
	for (int i = 0; i < json_array.count(); i++)
	{
		if (!json_array.at(i).isObject()) break;

		if (json_array.at(i).toObject().contains("id") && json_array.at(i).toObject().contains("type")
			&& json_array.at(i).toObject().contains("filename") && json_array.at(i).toObject().contains("exists"))
		{
			return FileLocation(json_array.at(i).toObject().value("id").toString(), FileLocation::stringToType(json_array.at(i).toObject().value("type").toString()),
								json_array.at(i).toObject().value("filename").toString(), json_array.at(i).toObject().value("exists").toBool());
		}
	}

	return output;
}

FileInfo DatabaseServiceRemote::analysisJobLatestLogInfo(const int& job_id) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	FileInfo output;
	RequestUrlParams params;
	params.insert("job_id", QString::number(job_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("analysis_job_last_update", params, true);
	if (reply.length() == 0)
	{
		THROW(Exception, "Could not get the latest update info for the job id " + QString::number(job_id));
	}

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonObject json_object = json_doc.object();

	if (json_object.contains("latest_file") && json_object.contains("latest_file_with_path") && json_object.contains("latest_created") && json_object.contains("latest_mod"))
	{
		return FileInfo(json_object.value("latest_file").toString(),
						json_object.value("latest_file_with_path").toString(),
						QDateTime().fromSecsSinceEpoch(json_object.value("latest_created").toString().toLongLong()),
						QDateTime().fromSecsSinceEpoch(json_object.value("latest_mod").toString().toLongLong()));
	}

	return output;
}

FileLocation DatabaseServiceRemote::analysisJobGSvarFile(const int& job_id) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	RequestUrlParams params;
	params.insert("job_id", QString::number(job_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("analysis_job_gsvar_file", params, true);
	if (reply.length() == 0)
	{
		THROW(Exception, "Could not get a GSvar file for the job id " + QString::number(job_id));
	}

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonObject json_object = json_doc.object();

	if (json_object.contains("id") && json_object.contains("type") && json_object.contains("filename") && json_object.contains("exists"))
	{
		return FileLocation(json_object.value("id").toString(), FileLocation::stringToType(json_object.value("type").toString()), json_object.value("filename").toString(), json_object.value("exists").toBool());
	}

	return FileLocation{};
}

FileLocation DatabaseServiceRemote::analysisJobLogFile(const int& job_id) const
{
	checkEnabled(__PRETTY_FUNCTION__);

	RequestUrlParams params;
	params.insert("job_id", QString::number(job_id).toLocal8Bit());
	QByteArray reply = makeGetApiCall("analysis_job_log", params, true);
	if (reply.length() == 0)
	{
		THROW(Exception, "Could not get a log file for the job id " + QString::number(job_id));
	}

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonObject json_object = json_doc.object();

	if (json_object.contains("id") && json_object.contains("type") && json_object.contains("filename") && json_object.contains("exists"))
	{
		return FileLocation(json_object.value("id").toString(), FileLocation::stringToType(json_object.value("type").toString()), json_object.value("filename").toString(), json_object.value("exists").toBool());
	}

	return FileLocation{};
}

QByteArray DatabaseServiceRemote::makeGetApiCall(QString api_path, RequestUrlParams params, bool ignore_if_missing) const
{		
	try
	{
		HttpHeaders headers;
		headers.insert("Content-Type", "text/plain");
		return ApiCaller().get(api_path, params, headers, true, false, true);
	}
	catch (Exception& e)
	{
		if (!ignore_if_missing) QMessageBox::warning(QApplication::activeWindow(), "Database API GET call error", e.message());
	}

	return QByteArray{};
}

QByteArray DatabaseServiceRemote::makePostApiCall(QString api_path, RequestUrlParams params, QByteArray content, bool ignore_if_missing) const
{
	try
	{
		 return ApiCaller().post(api_path, params, HttpHeaders(), content, true, false, true);
	}
	catch (Exception& e)
	{
		if (!ignore_if_missing) QMessageBox::warning(QApplication::activeWindow(), "Database API POST call error", e.message());
	}

	return QByteArray{};
}
