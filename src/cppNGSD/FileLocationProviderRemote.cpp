#include "FileLocationProviderRemote.h"
#include "HttpRequestHandler.h"
#include "NGSHelper.h"

FileLocationProviderRemote::FileLocationProviderRemote(const QString sample_id)
	: sample_id_(sample_id)
{
}

bool FileLocationProviderRemote::isLocal() const
{
	return false;
}

FileLocation FileLocationProviderRemote::getAnalysisVcf() const
{
	return getOneFileLocationByType(PathType::VCF, "");
}

FileLocation FileLocationProviderRemote::getAnalysisMosaicFile() const
{
	return getOneFileLocationByType(PathType::MOSAIC_VARIANTS, "");
}

FileLocation FileLocationProviderRemote::getAnalysisSvFile() const
{
	return getOneFileLocationByType(PathType::STRUCTURAL_VARIANTS, "");
}

FileLocation FileLocationProviderRemote::getAnalysisCnvFile() const
{
	return getOneFileLocationByType(PathType::COPY_NUMBER_CALLS, "");
}

FileLocation FileLocationProviderRemote::getAnalysisMosaicCnvFile() const
{
	return getOneFileLocationByType(PathType::COPY_NUMBER_CALLS_MOSAIC, "");
}

FileLocation FileLocationProviderRemote::getAnalysisUpdFile() const
{
	return getOneFileLocationByType(PathType::UPD, "");
}

FileLocation FileLocationProviderRemote::getRepeatExpansionImage(QString locus) const
{
	return getOneFileLocationByType(PathType::REPEAT_EXPANSION_IMAGE, locus);
}

FileLocationList FileLocationProviderRemote::getQcFiles() const
{
	return getFileLocationsByType(PathType::QC, false);
}

FileLocationList FileLocationProviderRemote::getFileLocationsByType(PathType type, bool return_if_missing) const
{
	FileLocationList output;
	if (sample_id_.isEmpty())
	{
		THROW(ArgumentException, "File name has not been specified")
		return output;
	}

	QStringList gsvar_filename_parts = sample_id_.split("/");
	QString file_id;
	if (gsvar_filename_parts.size()<2)
	{
		return output;
	}
	file_id = gsvar_filename_parts[gsvar_filename_parts.size()-2].trimmed();

	HttpHeaders add_headers;
	add_headers.insert("Accept", "application/json");
	QByteArray reply = HttpRequestHandler(HttpRequestHandler::NONE).get(
				NGSHelper::serverApiUrl()
				+ "file_location?ps_url_id=" + file_id + "&type=" + FileLocation::typeToString(type)
				+ "&multiple_files=1"
				+ "&return_if_missing=" +(return_if_missing ? "1" : "0")
				+ "&token="+LoginManager::token(), add_headers);

	QJsonDocument json_doc = QJsonDocument::fromJson(reply);
	QJsonArray file_list = json_doc.array();

	output = mapJsonArrayToFileLocationList(file_list, return_if_missing);
	return output;
}

FileLocation FileLocationProviderRemote::getOneFileLocationByType(PathType type, QString locus) const
{
	FileLocation output;
	if (sample_id_.isEmpty())
	{
		THROW(ArgumentException, "File name has not been specified")
		return output;
	}

	QStringList gsvar_filename_parts = sample_id_.split("/");
	QString file_id;
	if (gsvar_filename_parts.size()<2)
	{
		return output;
	}
	file_id = gsvar_filename_parts[gsvar_filename_parts.size()-2].trimmed();

	HttpHeaders add_headers;
	add_headers.insert("Accept", "application/json");
	QString reply = HttpRequestHandler(HttpRequestHandler::NONE).get(
				NGSHelper::serverApiUrl()
				+ "file_location?ps_url_id=" + file_id + "&type=" +  FileLocation::typeToString(type)
				+ "&multiple_files=0"
				+ (locus.isEmpty() ? "" : "&locus=" + locus)
				+ "&token="+LoginManager::token(), add_headers);

	QJsonDocument json_doc = QJsonDocument::fromJson(reply.toLatin1());
	QJsonArray file_list = json_doc.array();
	QJsonObject file_object;
	if (!file_list.isEmpty()) file_object = file_list[0].toObject();

	if (file_object.isEmpty())
	{
		THROW(Exception, "Could not find file info: " + FileLocation::typeToString(type));
	}

	output = mapJsonObjectToFileLocation(file_object);
	return output;
}

FileLocation FileLocationProviderRemote::mapJsonObjectToFileLocation(QJsonObject obj) const
{	
	return FileLocation {
		obj.value("id").toString(),
		FileLocation::stringToType(obj.value("type").toString()),
		obj.value("filename").toString(),
		obj.value("exists").toBool()
	};
}

FileLocationList FileLocationProviderRemote::mapJsonArrayToFileLocationList(QJsonArray array, bool return_if_missing) const
{
	FileLocationList output {};
	for (int i = 0; i < array.count(); ++i)
	{
		FileLocation loc = mapJsonObjectToFileLocation(array[i].toObject());
		if (loc.exists || return_if_missing)
		{
			output.append(loc);
		}
	}
	return output;
}

FileLocationList FileLocationProviderRemote::getBamFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::BAM, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getCnvCoverageFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::COPY_NUMBER_RAW_DATA, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getBafFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::BAF, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getMantaEvidenceFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::MANTA_EVIDENCE, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getCircosPlotFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::CIRCOS_PLOT, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getExpressionFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::EXPRESSION, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getVcfFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::VCF, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getRepeatExpansionFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::REPEAT_EXPANSIONS, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getPrsFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::PRS, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getLowCoverageFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::LOWCOV_BED, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getCopyNumberCallFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::COPY_NUMBER_CALLS, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getRohFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::ROH, return_if_missing);
}

FileLocationList FileLocationProviderRemote::getSomaticLowCoverageFiles(bool return_if_missing) const
{
	return getFileLocationsByType(PathType::LOWCOV_BED, return_if_missing);
}

FileLocation FileLocationProviderRemote::getSomaticCnvCoverageFile() const
{
	return getOneFileLocationByType(PathType::COPY_NUMBER_RAW_DATA, "");
}

FileLocation FileLocationProviderRemote::getSomaticCnvCallFile() const
{
	return getOneFileLocationByType(PathType::CNV_RAW_DATA_CALL_REGIONS, "");
}

FileLocation FileLocationProviderRemote::getSomaticLowCoverageFile() const
{
	return getOneFileLocationByType(PathType::LOWCOV_BED, "");
}

FileLocation FileLocationProviderRemote::getSomaticMsiFile() const
{
	return getOneFileLocationByType(PathType::MSI, "");
}

FileLocation FileLocationProviderRemote::getSomaticIgvScreenshotFile() const
{
	return getOneFileLocationByType(PathType::IGV_SCREENSHOT, "");
}
