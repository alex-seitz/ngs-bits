#include "FileLocationWorker.h"
#include "Log.h"

FileLocationWorker::FileLocationWorker(IgvDialog& igv_dialog, PathType file_type, AnalysisType analysis_type)
    : igv_dialog_(igv_dialog)
    , file_type_(file_type)
    , analysis_type_(analysis_type)
{
}

void FileLocationWorker::run()
{
    Log::info("File location worker started ");
    if (file_type_ == PathType::BAM)
    {
        FileLocationList bams = GlobalServiceProvider::fileLocationProvider().getBamFiles(true);
        foreach(const FileLocation& file, bams)
        {
            igv_dialog_.addFile(file, true);
        }
        Log::info("Getting BAM files");
    }

    if (file_type_ == PathType::BAF)
    {
        FileLocationList bafs = GlobalServiceProvider::fileLocationProvider().getBafFiles(true);
        foreach(const FileLocation& file, bafs)
        {
            if(analysis_type_ == SOMATIC_PAIR && !file.id.contains("somatic")) continue;
            igv_dialog_.addFile(file, true);
        }
        Log::info("Getting BAF files");
    }

    if (file_type_ == PathType::VCF)
    {
        //analysis VCF
        FileLocation vcf = GlobalServiceProvider::fileLocationProvider().getAnalysisVcf();
        bool igv_default_small = Settings::boolean("igv_default_small", true);
        igv_dialog_.addFile(vcf, igv_default_small);
        Log::info("Getting VCF files");
    }

    if (file_type_ == PathType::STRUCTURAL_VARIANTS)
    {
        //analysis SV file
        FileLocation bedpe = GlobalServiceProvider::fileLocationProvider().getAnalysisSvFile();
        bool igv_default_sv = Settings::boolean("igv_default_sv", true);
        igv_dialog_.addFile(bedpe, igv_default_sv);
        Log::info("Getting SV files");
    }

    if (file_type_ == PathType::CNV_RAW_DATA_CALL_REGIONS)
    {
        //CNV files
        if (analysis_type_==SOMATIC_SINGLESAMPLE || analysis_type_==SOMATIC_PAIR)
        {
            FileLocation file = GlobalServiceProvider::fileLocationProvider().getSomaticCnvCoverageFile();
            igv_dialog_.addFile(file, true);

            FileLocation file2 = GlobalServiceProvider::fileLocationProvider().getSomaticCnvCallFile();
            igv_dialog_.addFile(file2, true);
        }
        else
        {
            FileLocationList segs = GlobalServiceProvider::fileLocationProvider().getCnvCoverageFiles(true);
            foreach(const FileLocation& file, segs)
            {
                igv_dialog_.addFile(file, true);
            }
        }
        Log::info("Getting CNV files");
    }

    if (file_type_ == PathType::MANTA_EVIDENCE)
    {
        //Manta evidence file(s)
        FileLocationList evidence_files = GlobalServiceProvider::fileLocationProvider().getMantaEvidenceFiles(true);
        foreach(const FileLocation& file, evidence_files)
        {
            igv_dialog_.addFile(file, false);
        }
        Log::info("Getting MANTA files");
    }


}
