#ifndef SomaticReportHelper_H
#define SomaticReportHelper_H

#include "VariantList.h"
#include "GeneSet.h"
#include "CnvList.h"
#include "BedFile.h"
#include "NGSD.h"
#include "Settings.h"
#include "QCCollection.h"
#include "TSVFileStream.h"
#include "OntologyTermCollection.h"
#include "FilterCascade.h"
#include <QMultiMap>
#include <QDir>
#include "RtfDocument.h"
#include "BedpeFile.h"
#include "SomaticReportSettings.h"
#include "HttpRequestHandler.h"

struct CPPNGSDSHARED_EXPORT SomaticVirusInfo
{
	QByteArray chr;
	int start;
	int end;
	QByteArray name;
	int reads;
	double coverage;
	int mismatches;
	double idendity;

	///Virus gene, extracted from name_ if possible
	QByteArray virusGene() const
	{
		QByteArray gene_name = name;
		if(name.split('_').count() > 1)
		{
			QByteArray reduced_name = name.split('_').at(0) + "_";
			gene_name = gene_name.replace(reduced_name,"");
			return gene_name;
		}
		return "";
	}

	QByteArray virusName() const
	{
		QByteArray virus_name = "";
		if(name.split('_').count() > 0)
		{
			virus_name = name.split('_').at(0);
		}
		return virus_name;
	}
};

///creates a somatic RTF report
class CPPNGSDSHARED_EXPORT SomaticReportHelper
{
public:
	///Constructor loads data into class
	SomaticReportHelper(GenomeBuild build, const VariantList& variants, const CnvList &cnvs, const VariantList& germline_variants, const SomaticReportSettings& settings);

	///write Rtf File
	void storeRtf(const QByteArray& out_file);

	///write XML file
	void storeXML(QString file_name);

	///methods that create TSV files for QBIC
	void storeQbicData(QString path);

	///Returns CNV type, e.g. DEL (het) according copy number
	static QByteArray CnvTypeDescription(int tumor_cn, bool add_cn);
	///Returns true if germline variant file is valid (annotations and at least one germline variant)
	static bool checkGermlineSNVFile(const VariantList& germline_variants);

	///Returns maximum tumor clonailty in cnv file
	static double getCnvMaxTumorClonality(const CnvList& cnvs);

	double getTumorContentBySNVs();

	///Returns CNV burden, i.e. total cnv size divided by genome size in %
	static double cnvBurden(const CnvList& cnvs)
	{
		return cnvs.totalCnvSize() / 3101788170. * 100;
	}

	static QString trans(const QString& text);

	static QByteArray trans(QByteArray text)
	{
		return trans( QString(text) ).toUtf8();
	}

	///returns best matching transcript - or an empty transcript
	static VariantTranscript selectSomaticTranscript(const Variant& variant, const SomaticReportSettings& settings, int index_co_sp);


private:
	///transforms GSVar coordinates of Variants to VCF INDEL-standard
	VariantList gsvarToVcf(const VariantList& gsvar_list, const QString& orig_name);

	///Parses CN to description
	RtfSourceCode CnvDescription(const CopyNumberVariant& cnv, const SomaticGeneRole& role);

	///Parses annotated cytobands to text, "" if not annotation available
	QByteArray cytoband(const CopyNumberVariant& cnv);

	RtfTableRow overlappingCnv(const CopyNumberVariant& cnv, QByteArray gene, double snv_af, const QList<int>& col_widths);

	///parts of the report
	///Generates table with genral information
	RtfSourceCode partSummary();
	///Generates table with classified most relevant variants
	RtfSourceCode partRelevantVariants();
	///Generates table with variants of unknown significance
	RtfSourceCode partUnclearVariants();
	///Generates table with chromosomal aberratons
	RtfSourceCode partCnvTable();
	///Generates table for fusions
	RtfSourceCode partFusions();
	///generates table with germline SNPs
	RtfSourceCode partPharmacoGenetics();
	///generates meta data (e.g. coverage, depth)
	RtfSourceCode partMetaData();
	///generates pathway table
	RtfSourceCode partPathways();
	///generates with billing informationen (gene names in first table + OMIM numbers) for health funds according EBM
	RtfSourceCode partBillingTable();
	///Generates table with virus information
	RtfSourceCode partVirusTable();
	///Generates part with somatic IGV snapshot
	RtfSourceCode partIgvScreenshot();

	///creates table with SNVs, relevant germline SNPs (class 4/5) and overlapping CNVs
	RtfTable snvTable(const QSet<int>& indices, bool high_impact_table=true);

	//skipped amplifications in somaticalterationtable
	GeneSet skipped_amp_ = {};

	//Somatic SNVs/INDELs
	VariantList somatic_vl_;
	QSet<int> somatic_vl_high_impact_indices_; //small variants with high impact i.e. they are added to the pathway list in bold
	QSet<int> somatic_vl_low_impact_indices_; //small variants with low impact i.e. they are added to the pathway list, but not bold

	GenomeBuild build_;

	const SomaticReportSettings& settings_;
	//VariantList for relevant germline SNVs
	const VariantList& germline_vl_;

	//Microsatellite instability MANTIS step-wise-difference metric
	double mantis_msi_swd_value_;

	//CNVList for somatic (filtered) copy-number altered variants
	CnvList cnvs_;
	QHash<int, GeneSet> cnv_high_impact_indices_; //CNVs with high impact (i.e. they are added to the pathway list): CNV index => gene list

	//Somatic viruses (original file usually in tumor dir)
	QList<SomaticVirusInfo> validated_viruses_;

	NGSD db_;

	QCCollection tumor_qcml_data_;
	QCCollection normal_qcml_data_;

	//Processing system data
	ProcessingSystemData processing_system_data_;

	//genes that are printed on last report page for EBM billing. List is to be filled in snvTable()
	GeneSet ebm_genes_;

	//ICD10 text diagnosis tumor
	QString icd10_diagnosis_code_;
	//histologic tumor fraction according genlab
	double histol_tumor_fraction_;

	double mutation_burden_;

	//indices for somatic variant file
	int snv_index_coding_splicing_;


	//indices for somatic CNV file
	int cnv_index_cn_change_;
	int cnv_index_cnv_type_;
	int cnv_index_state_;
	int cnv_index_tumor_clonality_;
	int cnv_index_cytoband_;

	RtfDocument doc_;

	void saveReportData(QString filename, QString path, QString content);

	void somaticSnvForQbic(QString path);
	void germlineSnvForQbic(QString path);
	void somaticCnvForQbic(QString path);
	void germlineCnvForQbic(QString path);
	void somaticSvForQbic(QString path);
	void metaDataForQbic(QString path);
};

#endif // SomaticReportHelper_H
