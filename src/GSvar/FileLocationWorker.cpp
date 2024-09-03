#include "FileLocationWorker.h"
#include "Log.h"

FileLocationWorker::FileLocationWorker(PathType file_type, AnalysisType analysis_type, FileLocationList& data)
    : file_type_(file_type)
    , analysis_type_(analysis_type)
    , data_(data)
{
}

void FileLocationWorker::run()
{
    Log::info("File location worker started ");
    if (file_type_ == PathType::BAM)
    {
        data_ = GlobalServiceProvider::fileLocationProvider().getBamFiles(true);
        Log::info("Getting BAM files " + QString::number(data_.size()));
        return;
    }

    if (file_type_ == PathType::BAF)
    {
        data_ = GlobalServiceProvider::fileLocationProvider().getBafFiles(true);
        Log::info("Getting BAF files");
        return;
    }

    if (file_type_ == PathType::VCF)
    {
        //analysis VCF
        FileLocation vcf = GlobalServiceProvider::fileLocationProvider().getAnalysisVcf();
        data_.append(vcf);
        Log::info("Getting VCF files");
        return;
    }

    if (file_type_ == PathType::STRUCTURAL_VARIANTS)
    {
        //analysis SV file
        FileLocation bedpe = GlobalServiceProvider::fileLocationProvider().getAnalysisSvFile();
        data_.append(bedpe);
        Log::info("Getting SV files");
        return;
    }

    if (file_type_ == PathType::CNV_RAW_DATA_CALL_REGIONS)
    {
        //CNV files
        if (analysis_type_==SOMATIC_SINGLESAMPLE || analysis_type_==SOMATIC_PAIR)
        {            
            FileLocation file = GlobalServiceProvider::fileLocationProvider().getSomaticCnvCoverageFile();
            data_.append(file);

            FileLocation file2 = GlobalServiceProvider::fileLocationProvider().getSomaticCnvCallFile();
            data_.append(file2);
        }
        else
        {
            data_ = GlobalServiceProvider::fileLocationProvider().getCnvCoverageFiles(true);
        }
        Log::info("Getting CNV files");
        return;
    }

    if (file_type_ == PathType::MANTA_EVIDENCE)
    {
        //Manta evidence file(s)
        data_ = GlobalServiceProvider::fileLocationProvider().getMantaEvidenceFiles(true);
        Log::info("Getting MANTA files");
        return;
    }
}
