#include "TestFramework.h"
#include "VariantList.h"
#include "QCCollection.h"
#include "Statistics.h"
#include "Settings.h"

TEST_CLASS(Statistics_Test)
{
	Q_OBJECT
	private slots:

	void somatic()
	{
		QString tumor_bam = TESTDATA("data_in/tumor.bam");
		QString normal_bam = TESTDATA("data_in/normal.bam");
		QString somatic_vcf = TESTDATA("data_in/Statistics_somatic_tmb.vcf");

		BedFile target_test_file;
		target_test_file.load(TESTDATA("data_in/Statistics_somatic_tmb_target.bed")); //exons of ssscv4, sorted and merged

		QCCollection stats = Statistics::somatic("hg19", tumor_bam, normal_bam, somatic_vcf, QString(),target_test_file, true);

		S_EQUAL(stats[0].name(), QString("sample correlation"));
		S_EQUAL(stats[0].accession(), QString("QC:2000040"));
		S_EQUAL(stats[0].toString(), QString("n/a (too few variants)"));
		S_EQUAL(stats[1].name(), QString("variant count"));
		S_EQUAL(stats[1].accession(), QString("QC:2000013"));
		S_EQUAL(stats[1].toString(), QString("77"));
		S_EQUAL(stats[2].name(), QString("somatic variant count"));
		S_EQUAL(stats[2].accession(), QString("QC:2000041"));
		S_EQUAL(stats[2].toString(), QString("64"));
		S_EQUAL(stats[3].name(), QString("known somatic variants percentage"));
		S_EQUAL(stats[3].accession(), QString("QC:2000045"));
		S_EQUAL(stats[3].toString(), QString("n/a (no gnomAD_AF annotation in CSQ info field)"));
		S_EQUAL(stats[4].name(), QString("somatic indel variants percentage"));
		S_EQUAL(stats[4].accession(), QString("QC:2000042"));
		S_EQUAL(stats[4].toString(), QString("0.00"));
		S_EQUAL(stats[5].name(), QString("somatic transition/transversion ratio"));
		S_EQUAL(stats[5].accession(), QString("QC:2000043"));
		S_EQUAL(stats[5].toString(), QString("6.11"));
		S_EQUAL(stats[6].accession(), QString("QC:2000054"));
		S_EQUAL(stats[6].toString(), QString("n/a (too few variants)"));
		I_EQUAL(stats.count(), 7);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
			IS_TRUE(stats[i].accession()!="");
		}

	}

	void somatic_mutation_burden()
	{
		QString somatic_vcf = TESTDATA("../tools-TEST/data_in/SomaticQC_in7.vcf");
		QString exons = TESTDATA("../tools-TEST/data_in/SomaticQC_tmb_exons.bed");
		QString target = TESTDATA("../tools-TEST/data_in/SomaticQC_in8.bed");
		QString tsg = TESTDATA("../tools-TEST/data_in/SomaticQC_tmb_tsg.bed");
		QString blacklist = TESTDATA("../tools-TEST/data_in/SomaticQC_tmb_blacklist.bed");

		//Use TSG BED-file which does not overlap with a variant in somatic VCF file
		QCValue tmb = Statistics::mutationBurden(somatic_vcf, exons, target, tsg, blacklist);
		S_EQUAL(tmb.name(),QString("somatic variant rate"));
		S_EQUAL(tmb.accession(),QString("QC:2000053"));
		S_EQUAL(tmb.toString(),QString("4.41"));

		//use TSG BED-file which overlaps with a variant in somatic VCF file
		tsg = TESTDATA("data_in/Statistics_somatic_tmb_tsg.bed");
		tmb = Statistics::mutationBurden(somatic_vcf, exons, target, tsg, blacklist);
		S_EQUAL(tmb.name(),QString("somatic variant rate"));
		S_EQUAL(tmb.accession(),QString("QC:2000053"));
		S_EQUAL(tmb.toString(),QString("2.23"));

		//only target region, blacklist file is empty
		blacklist = TESTDATA("data_in/Statistics_somatic_tmb_blacklist.bed"); // file is empty
		tmb = Statistics::mutationBurden(somatic_vcf, exons, target, tsg, blacklist);
		S_EQUAL(tmb.name(),QString("somatic variant rate"));
		S_EQUAL(tmb.accession(),QString("QC:2000053"));
		S_EQUAL(tmb.toString(),QString("n/a"));
	}

	void variantList_panel_filter()
	{
		VcfFile vl;
		vl.load(TESTDATA("data_in/panel_vep.vcf"));

		QCCollection stats = Statistics::variantList(vl, true);
		S_EQUAL(stats[0].name(), QString("variant count"));
		S_EQUAL(stats[0].toString(), QString("153"));
		S_EQUAL(stats[0].accession(), QString("QC:2000013"));
		S_EQUAL(stats[1].name(), QString("known variants percentage"));
		S_EQUAL(stats[1].accession(), QString("QC:2000014"));
		S_EQUAL(stats[1].toString(), QString("100.00"));
		I_EQUAL(stats.count(), 6);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
			IS_TRUE(stats[i].accession()!="");
		}
	}

	void variantList_panel_nofilter()
	{
		VcfFile vl;
		vl.load(TESTDATA("data_in/panel_vep.vcf"));

		QCCollection stats = Statistics::variantList(vl, false);
		S_EQUAL(stats[0].name(), QString("variant count"));
		S_EQUAL(stats[0].toString(), QString("329"));
		S_EQUAL(stats[0].accession(), QString("QC:2000013"));
		S_EQUAL(stats[1].name(), QString("known variants percentage"));
		S_EQUAL(stats[1].accession(), QString("QC:2000014"));
		S_EQUAL(stats[1].toString(), QString("99.70"));
		S_EQUAL(stats[2].name(), QString("high-impact variants percentage"));
		S_EQUAL(stats[2].accession(), QString("QC:2000015"));
		S_EQUAL(stats[2].toString(), QString("0.61"));
		S_EQUAL(stats[3].name(), QString("homozygous variants percentage"));
		S_EQUAL(stats[3].toString(), QString("34.65"));
		S_EQUAL(stats[4].name(), QString("indel variants percentage"));
		S_EQUAL(stats[4].toString(), QString("13.68"));
		S_EQUAL(stats[5].name(), QString("transition/transversion ratio"));
		S_EQUAL(stats[5].toString(), QString("2.12"));
		I_EQUAL(stats.count(), 6);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
			IS_TRUE(stats[i].accession()!="");
		}
	}

	void variantList_empty()
	{
		VcfFile vl;

		QCCollection stats = Statistics::variantList(vl, true);
		S_EQUAL(stats[0].name(), QString("variant count"));
		S_EQUAL(stats[0].toString(), QString("0"));
		S_EQUAL(stats[1].name(), QString("known variants percentage"));
		IS_TRUE(stats[1].toString().startsWith("n/a "));
		S_EQUAL(stats[2].name(), QString("high-impact variants percentage"));
		IS_TRUE(stats[2].toString().startsWith("n/a "));
		S_EQUAL(stats[3].name(), QString("homozygous variants percentage"));
		IS_TRUE(stats[3].toString().startsWith("n/a "));
		S_EQUAL(stats[4].name(), QString("indel variants percentage"));
		IS_TRUE(stats[4].toString().startsWith("n/a "));
		S_EQUAL(stats[5].name(), QString("transition/transversion ratio"));
		IS_TRUE(stats[5].toString().startsWith("n/a "));
		I_EQUAL(stats.count(), 6);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
			IS_TRUE(stats[i].accession()!="");
		}
	}

	void mapping_panel()
	{
		QString ref_file = Settings::string("reference_genome", true);
		if (ref_file=="") SKIP("Test needs the reference genome!");

		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/panel.bed"));
		bed_file.merge();

		QCCollection stats = Statistics::mapping(bed_file, TESTDATA("data_in/panel.bam"), ref_file, 20);
		I_EQUAL(stats.count(), 23);
		S_EQUAL(stats[0].name(), QString("trimmed base percentage"));
		S_EQUAL(stats[0].toString(), QString("10.82"));
		S_EQUAL(stats[1].name(), QString("clipped base percentage"));
		S_EQUAL(stats[1].toString(), QString("0.00"));
		S_EQUAL(stats[2].name(), QString("mapped read percentage"));
		S_EQUAL(stats[2].toString(), QString("98.93"));
		S_EQUAL(stats[3].name(), QString("on-target read percentage"));
		S_EQUAL(stats[3].toString(), QString("87.02"));
		S_EQUAL(stats[4].name(), QString("near-target read percentage"));
		S_EQUAL(stats[4].toString(), QString("95.10"));
		S_EQUAL(stats[5].name(), QString("properly-paired read percentage"));
		S_EQUAL(stats[5].toString(), QString("97.86"));
		S_EQUAL(stats[6].name(), QString("insert size"));
		S_EQUAL(stats[6].toString(), QString("180.02"));
		S_EQUAL(stats[7].name(), QString("duplicate read percentage"));
		S_EQUAL(stats[7].toString(), QString("n/a (no duplicates marked or duplicates removed during data analysis)"));
		S_EQUAL(stats[8].name(), QString("bases usable (MB)"));
		S_EQUAL(stats[8].toString(), QString("34.11"));
		S_EQUAL(stats[9].name(), QString("target region read depth"));
		S_EQUAL(stats[9].toString(), QString("125.63"));
		S_EQUAL(stats[10].name(), QString("target region 10x percentage"));
		S_EQUAL(stats[10].toString(), QString("97.08"));
		S_EQUAL(stats[11].name(), QString("target region 20x percentage"));
		S_EQUAL(stats[11].toString(), QString("94.06"));
		S_EQUAL(stats[12].name(), QString("target region 30x percentage"));
		S_EQUAL(stats[12].toString(), QString("90.25"));
		S_EQUAL(stats[13].name(), QString("target region 50x percentage"));
		S_EQUAL(stats[13].toString(), QString("80.81"));
		S_EQUAL(stats[14].name(), QString("target region 100x percentage"));
		S_EQUAL(stats[14].toString(), QString("55.51"));
		S_EQUAL(stats[15].name(), QString("target region 200x percentage"));
		S_EQUAL(stats[15].toString(), QString("18.22"));
		S_EQUAL(stats[16].name(), QString("target region 500x percentage"));
		S_EQUAL(stats[16].toString(), QString("0.06"));
		S_EQUAL(stats[17].name(), QString("target region half depth percentage"));
		S_EQUAL(stats[17].toString(), QString("74.42"));
		S_EQUAL(stats[18].name(), QString("AT dropout"));
		S_EQUAL(stats[18].toString(), QString("8.55"));
		S_EQUAL(stats[19].name(), QString("GC dropout"));
		S_EQUAL(stats[19].toString(), QString("1.15"));
		S_EQUAL(stats[20].name(), QString("depth distribution plot"));
		IS_TRUE(stats[20].type()==QVariant::ByteArray);
		S_EQUAL(stats[21].name(), QString("insert size distribution plot"));
		IS_TRUE(stats[21].type()==QVariant::ByteArray);
		S_EQUAL(stats[22].name(), QString("GC bias plot"));
		IS_TRUE(stats[22].type()==QVariant::ByteArray);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
			IS_TRUE(stats[i].accession()!="");
		}
	}

	void contamination()
	{
		QCCollection stats = Statistics::contamination("hg19", TESTDATA("data_in/panel.bam"));
		I_EQUAL(stats.count(), 1);
		S_EQUAL(stats[0].name(), QString("SNV allele frequency deviation"));
		S_EQUAL(stats[0].toString(), QString("1.57"));
	}

	void mapping_close_exons()
	{
		QString ref_file = Settings::string("reference_genome", true);
		if (ref_file=="") SKIP("Test needs the reference genome!");

		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/close_exons.bed"));
		bed_file.merge();

		QCCollection stats = Statistics::mapping(bed_file, TESTDATA("data_in/close_exons.bam"), ref_file);
		I_EQUAL(stats.count(), 23);
		S_EQUAL(stats[0].name(), QString("trimmed base percentage"));
		S_EQUAL(stats[0].toString(), QString("19.10"));
		S_EQUAL(stats[1].name(), QString("clipped base percentage"));
		S_EQUAL(stats[1].toString(), QString("0.00"));
		S_EQUAL(stats[2].name(), QString("mapped read percentage"));
		S_EQUAL(stats[2].toString(), QString("100.00"));
		S_EQUAL(stats[3].name(), QString("on-target read percentage"));
		S_EQUAL(stats[3].toString(), QString("99.94"));
		S_EQUAL(stats[4].name(), QString("near-target read percentage"));
		S_EQUAL(stats[4].toString(), QString("100.00"));
		S_EQUAL(stats[5].name(), QString("properly-paired read percentage"));
		S_EQUAL(stats[5].toString(), QString("100.00"));
		S_EQUAL(stats[6].name(), QString("insert size"));
		S_EQUAL(stats[6].toString(), QString("138.06"));
		S_EQUAL(stats[7].name(), QString("duplicate read percentage"));
		S_EQUAL(stats[7].toString(), QString("n/a (no duplicates marked or duplicates removed during data analysis)"));
		S_EQUAL(stats[8].name(), QString("bases usable (MB)"));
		S_EQUAL(stats[8].toString(), QString("0.06"));
		S_EQUAL(stats[9].name(), QString("target region read depth"));
		S_EQUAL(stats[9].toString(), QString("415.05"));
		S_EQUAL(stats[10].name(), QString("target region 10x percentage"));
		S_EQUAL(stats[10].toString(), QString("100.00"));
		S_EQUAL(stats[11].name(), QString("target region 20x percentage"));
		S_EQUAL(stats[11].toString(), QString("100.00"));
		S_EQUAL(stats[12].name(), QString("target region 30x percentage"));
		S_EQUAL(stats[12].toString(), QString("100.00"));
		S_EQUAL(stats[13].name(), QString("target region 50x percentage"));
		S_EQUAL(stats[13].toString(), QString("100.00"));
		S_EQUAL(stats[14].name(), QString("target region 100x percentage"));
		S_EQUAL(stats[14].toString(), QString("93.51"));
		S_EQUAL(stats[15].name(), QString("target region 200x percentage"));
		S_EQUAL(stats[15].toString(), QString("79.87"));
		S_EQUAL(stats[16].name(), QString("target region 500x percentage"));
		S_EQUAL(stats[16].toString(), QString("39.61"));
		S_EQUAL(stats[17].name(), QString("target region half depth percentage"));
		S_EQUAL(stats[17].toString(), QString("77.92"));
		S_EQUAL(stats[18].name(), QString("AT dropout"));
		S_EQUAL(stats[18].toString(), QString("0.00"));
		S_EQUAL(stats[19].name(), QString("GC dropout"));
		S_EQUAL(stats[19].toString(), QString("10.46"));
		S_EQUAL(stats[20].name(), QString("depth distribution plot"));
		IS_TRUE(stats[20].type()==QVariant::ByteArray);
		S_EQUAL(stats[21].name(), QString("insert size distribution plot"));
		IS_TRUE(stats[21].type()==QVariant::ByteArray);
		S_EQUAL(stats[22].name(), QString("GC bias plot"));
		IS_TRUE(stats[22].type()==QVariant::ByteArray);
	}

	void mapping_wgs()
	{
		QCCollection stats = Statistics::mapping(TESTDATA("data_in/close_exons.bam"));
		S_EQUAL(stats[0].name(), QString("trimmed base percentage"));
		S_EQUAL(stats[0].toString(), QString("19.10"));
		S_EQUAL(stats[1].name(), QString("clipped base percentage"));
		S_EQUAL(stats[1].toString(), QString("0.00"));
		S_EQUAL(stats[2].name(), QString("mapped read percentage"));
		S_EQUAL(stats[2].toString(), QString("100.00"));
		S_EQUAL(stats[3].name(), QString("on-target read percentage"));
		S_EQUAL(stats[3].toString(), QString("100.00"));
		S_EQUAL(stats[4].name(), QString("properly-paired read percentage"));
		S_EQUAL(stats[4].toString(), QString("100.00"));
		S_EQUAL(stats[5].name(), QString("insert size"));
		S_EQUAL(stats[5].toString(), QString("138.06"));
		S_EQUAL(stats[6].name(), QString("duplicate read percentage"));
		S_EQUAL(stats[6].toString(), QString("n/a (duplicates not marked or removed during data analysis)"));
		S_EQUAL(stats[7].name(), QString("bases usable (MB)"));
		S_EQUAL(stats[7].toString(), QString("0.21"));
		S_EQUAL(stats[8].name(), QString("target region read depth"));
		S_EQUAL(stats[8].toString(8), QString("0.00006626"));
		S_EQUAL(stats[9].name(), QString("insert size distribution plot"));
		IS_TRUE(stats[9].type()==QVariant::ByteArray);
		I_EQUAL(stats.count(), 10);
	}

	void region1()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/demo_unmerged.bed"));

		QCCollection stats = Statistics::region(bed_file, true);
		S_EQUAL(stats[0].name(), QString("roi_bases"));
		S_EQUAL(stats[0].toString(0), QString("92168"));
		S_EQUAL(stats[1].name(), QString("roi_fragments"));
		S_EQUAL(stats[1].toString(), QString("590"));
		S_EQUAL(stats[2].name(), QString("roi_chromosomes"));
		S_EQUAL(stats[2].toString(), QString("13 (1, 2, 3, 4, 6, 7, 8, 10, 12, 16, 17, 18, 22)"));
		S_EQUAL(stats[3].name(), QString("roi_is_sorted"));
		S_EQUAL(stats[3].toString(), QString("yes"));
		S_EQUAL(stats[4].name(), QString("roi_is_merged"));
		S_EQUAL(stats[4].toString(), QString("yes"));
		S_EQUAL(stats[5].name(), QString("roi_fragment_min"));
		S_EQUAL(stats[5].toString(), QString("21"));
		S_EQUAL(stats[6].name(), QString("roi_fragment_max"));
		S_EQUAL(stats[6].toString(), QString("1000"));
		S_EQUAL(stats[7].name(), QString("roi_fragment_mean"));
		S_EQUAL(stats[7].toString(), QString("156.22"));
		S_EQUAL(stats[8].name(), QString("roi_fragment_stdev"));
		S_EQUAL(stats[8].toString(), QString("106.31"));
		I_EQUAL(stats.count(), 9);
		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
		}
	}

	void region2()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/demo_unmerged.bed"));

		QCCollection stats = Statistics::region(bed_file, false);
		S_EQUAL(stats[0].name(), QString("roi_bases"));
		S_EQUAL(stats[0].toString(0), QString("92369"));
		S_EQUAL(stats[1].name(), QString("roi_fragments"));
		S_EQUAL(stats[1].toString(), QString("592"));
		S_EQUAL(stats[2].name(), QString("roi_chromosomes"));
		S_EQUAL(stats[2].toString(), QString("13 (1, 2, 3, 4, 6, 7, 8, 10, 12, 16, 17, 18, 22)"));
		S_EQUAL(stats[3].name(), QString("roi_is_sorted"));
		S_EQUAL(stats[3].toString(), QString("no"));
		S_EQUAL(stats[4].name(), QString("roi_is_merged"));
		S_EQUAL(stats[4].toString(), QString("no"));
		S_EQUAL(stats[5].name(), QString("roi_fragment_min"));
		S_EQUAL(stats[5].toString(), QString("21"));
		S_EQUAL(stats[6].name(), QString("roi_fragment_max"));
		S_EQUAL(stats[6].toString(), QString("965"));
		S_EQUAL(stats[7].name(), QString("roi_fragment_mean"));
		S_EQUAL(stats[7].toString(), QString("156.03"));
		S_EQUAL(stats[8].name(), QString("roi_fragment_stdev"));
		S_EQUAL(stats[8].toString(), QString("102.36"));
		I_EQUAL(stats.count(), 9);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
		}
	}

	void region3()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/WGS_hg19.bed"));

		QCCollection stats = Statistics::region(bed_file, false);
		S_EQUAL(stats[1].name(), QString("roi_fragments"));
		S_EQUAL(stats[1].toString(), QString("25"));
		S_EQUAL(stats[0].name(), QString("roi_bases"));
		S_EQUAL(stats[0].toString(0), QString("3095693958"));
		S_EQUAL(stats[3].name(), QString("roi_is_sorted"));
		S_EQUAL(stats[3].toString(), QString("yes"));
		S_EQUAL(stats[4].name(), QString("roi_is_merged"));
		S_EQUAL(stats[4].toString(), QString("yes"));
		S_EQUAL(stats[5].name(), QString("roi_fragment_min"));
		S_EQUAL(stats[5].toString(), QString("16570"));
		S_EQUAL(stats[6].name(), QString("roi_fragment_max"));
		S_EQUAL(stats[6].toString(), QString("249250620"));
		S_EQUAL(stats[7].name(), QString("roi_fragment_mean"));
		S_EQUAL(stats[7].toString(2), QString("123827758.32"));
		S_EQUAL(stats[8].name(), QString("roi_fragment_stdev"));
		S_EQUAL(stats[8].toString(2), QString("61027437.29"));
		I_EQUAL(stats.count(), 9);

		//check that there is a description for each term
		for (int i=0; i<stats.count(); ++i)
		{
			IS_TRUE(stats[i].description()!="");
		}
	}

	void lowCoverage_roi_mapq20()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/panel.bed"));
		bed_file.merge();
		I_EQUAL(bed_file.baseCount(), 271536);

		BedFile low_cov =  Statistics::lowCoverage(bed_file, TESTDATA("data_in/panel.bam"), 20, 20);
		I_EQUAL(low_cov.count(), 441);
		I_EQUAL(low_cov.baseCount(), 16116);
	}

	void lowCoverage_roi_closeExons()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/close_exons.bed"));
		bed_file.merge();
		I_EQUAL(bed_file.baseCount(), 154);

		BedFile low_cov =  Statistics::lowCoverage(bed_file, TESTDATA("data_in/close_exons.bam"), 20, 1);
		I_EQUAL(low_cov.baseCount(), 0);
	}

	void lowCoverage_roi_bug_case1()
	{
		BedFile low_cov =  Statistics::lowCoverage(BedFile("chr13", 32931869, 32931970), TESTDATA("data_in/lowcov_bug_case1.bam"), 20, 1);
		I_EQUAL(low_cov.baseCount(), 0);
	}

	void lowCoverage_roi_bug_case2()
	{
		BedFile low_cov =  Statistics::lowCoverage(BedFile("chr13", 32931869, 32931970), TESTDATA("data_in/lowcov_bug_case2.bam"), 20, 1);
		I_EQUAL(low_cov.baseCount(), 0);
	}

	void lowCoverage_wgs_mapq20()
	{
		BedFile low_cov =  Statistics::lowCoverage(TESTDATA("data_in/panel.bam"), 20, 20);
		I_EQUAL(low_cov.count(), 1806);
		I_EQUAL(low_cov.baseCount(), 3095115275ll);
	}

	void highCoverage_roi_mapq20()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/panel.bed"));
		bed_file.merge();
		I_EQUAL(bed_file.baseCount(), 271536);

		BedFile low_cov =  Statistics::highCoverage(bed_file, TESTDATA("data_in/panel.bam"), 20, 20);
		I_EQUAL(low_cov.count(), 1700);
		I_EQUAL(low_cov.baseCount(), 255420);
	}

	void highCoverage_wgs_mapq20()
	{
		BedFile high_cov = Statistics::highCoverage(TESTDATA("data_in/panel.bam"), 20, 20);
		I_EQUAL(high_cov.count(), 1781);
		I_EQUAL(high_cov.baseCount(), 578708);
	}

	void avgCoverage_default()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/panel.bed"));
		bed_file.merge();

		Statistics::avgCoverage(bed_file, TESTDATA("data_in/panel.bam"), 20);

		I_EQUAL(bed_file.count(), 1532);
		X_EQUAL(bed_file[0].chr(), Chromosome("chr1"));
		I_EQUAL(bed_file[0].start(), 11073775);
		I_EQUAL(bed_file[0].end(), 11074032);
		S_EQUAL(bed_file[0].annotations()[0], QString("105.12"));
	}

	void avgCoverage_panel_mode_1decimal()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/close_exons.bed"));
		bed_file.merge();

		Statistics::avgCoverage(bed_file, TESTDATA("data_in/close_exons.bam"), 20, false, true, 1);

		I_EQUAL(bed_file.count(), 2);
		X_EQUAL(bed_file[0].chr(), Chromosome("chr1"));
		I_EQUAL(bed_file[0].start(), 45798425);
		I_EQUAL(bed_file[0].end(), 45798516);
		S_EQUAL(bed_file[0].annotations()[0], QString("475.8"));
		X_EQUAL(bed_file[1].chr(), Chromosome("chr1"));
		I_EQUAL(bed_file[1].start(), 45798580);
		I_EQUAL(bed_file[1].end(), 45798641);
		S_EQUAL(bed_file[1].annotations()[0], QString("324.9"));
	}

	void avgCoverage_with_duplicates()
	{
		BedFile bed_file;
		bed_file.load(TESTDATA("data_in/close_exons.bed"));
		bed_file.merge();

		Statistics::avgCoverage(bed_file, TESTDATA("data_in/close_exons.bam"), 20, true, true);

		I_EQUAL(bed_file.count(), 2);
		X_EQUAL(bed_file[0].chr(), Chromosome("chr1"));
		I_EQUAL(bed_file[0].start(), 45798425);
		I_EQUAL(bed_file[0].end(), 45798516);
		S_EQUAL(bed_file[0].annotations()[0], QString("475.80"));
		X_EQUAL(bed_file[1].chr(), Chromosome("chr1"));
		I_EQUAL(bed_file[1].start(), 45798580);
		I_EQUAL(bed_file[1].end(), 45798641);
		S_EQUAL(bed_file[1].annotations()[0], QString("324.90"));
	}

	void genderXY()
	{
		GenderEstimate estimate = Statistics::genderXY(TESTDATA("data_in/panel.bam"));
		S_EQUAL(estimate.gender, "female");
	}

	void genderHetX()
	{
		GenderEstimate estimate = Statistics::genderHetX(TESTDATA("data_in/panel.bam"), "hg19");
		S_EQUAL(estimate.gender, QString("unknown (too few SNPs)"));
	}

	void genderSRY()
	{
		GenderEstimate estimate = Statistics::genderSRY(TESTDATA("data_in/panel.bam"), "hg19");
		S_EQUAL(estimate.gender, "female");

		estimate = Statistics::genderSRY(TESTDATA("data_in/sry.bam"), "hg19");
		S_EQUAL(estimate.gender, "male");
	}

	void ancestry()
	{
		//default
		AncestryEstimates ancestry = Statistics::ancestry("hg19", TESTDATA("data_in/ancestry.vcf.gz"));
		I_EQUAL(ancestry.snps, 3096);
		F_EQUAL2(ancestry.afr, 0.0114, 0.001);
		F_EQUAL2(ancestry.eur, 0.3088, 0.001);
		F_EQUAL2(ancestry.sas, 0.1636, 0.001);
		F_EQUAL2(ancestry.eas, 0.0572, 0.001);
		S_EQUAL(ancestry.population, "EUR");

		//not enough SNPs
		ancestry = Statistics::ancestry("hg19", TESTDATA("data_in/ancestry.vcf.gz"), 10000);
		I_EQUAL(ancestry.snps, 3096);
		S_EQUAL(ancestry.population, "NOT_ENOUGH_SNPS");

		//not enough popultation distance
		ancestry = Statistics::ancestry("hg19", TESTDATA("data_in/ancestry.vcf.gz"), 1000, 0.0, 2.0);
		I_EQUAL(ancestry.snps, 3096);
		F_EQUAL2(ancestry.afr, 0.0114, 0.001);
		F_EQUAL2(ancestry.eur, 0.3088, 0.001);
		F_EQUAL2(ancestry.sas, 0.1636, 0.001);
		F_EQUAL2(ancestry.eas, 0.0572, 0.001);
		S_EQUAL(ancestry.population, "ADMIXED/UNKNOWN");
	}

	void ancestry_hg38()
	{
		AncestryEstimates ancestry = Statistics::ancestry("hg38", TESTDATA("data_in/ancestry_hg38.vcf.gz"));
		I_EQUAL(ancestry.snps, 2126);
		F_EQUAL2(ancestry.afr, 0.4984, 0.001);
		F_EQUAL2(ancestry.eur, 0.0241, 0.001);
		F_EQUAL2(ancestry.sas, 0.1046, 0.001);
		F_EQUAL2(ancestry.eas, 0.0742, 0.001);
		S_EQUAL(ancestry.population, "AFR");
	}


	void hrdScore()
	{
		const QList<BedLine> tmp_centromeres = {
			BedLine("chr1", 121535434, 124535434),
			BedLine("chr2", 92326171, 95326171),
			BedLine("chr3", 90504854, 93504854),
			BedLine("chr4", 49660117, 52660117),
			BedLine("chr5", 46405641, 49405641),
			BedLine("chr6", 58830166, 61830166),
			BedLine("chr7", 58054331, 61054331),
			BedLine("chr8", 43838887, 46838887),
			BedLine("chr9", 47367679, 50367679),
			BedLine("chr10", 39254935, 42254935),
			BedLine("chr11", 51644205, 54644205),
			BedLine("chr12", 34856694, 37856694),
			BedLine("chr13", 16000000, 19000000),
			BedLine("chr14", 16000000, 19000000),
			BedLine("chr15", 17000000, 20000000),
			BedLine("chr16", 35335801, 38335801),
			BedLine("chr17", 22263006, 25263006),
			BedLine("chr18", 15460898, 18460898),
			BedLine("chr19", 24681782, 27681782),
			BedLine("chr20", 26369569, 29369569),
			BedLine("chr21", 11288129, 14288129),
			BedLine("chr22", 13000000, 16000000),
			BedLine("chrX", 58632012, 61632012),
			BedLine("chrY", 10104553, 13104553)
		};
		BedFile centromeres;
		for(const auto& line : tmp_centromeres) centromeres.append(line);

		const QList<BedLine> tmp_telomeres = {
			BedLine("chr1", 1, 10000),
			BedLine("chr1", 249240621, 249250621),
			BedLine("chr2", 1, 10000),
			BedLine("chr2", 243189373, 243199373),
			BedLine("chr3", 1, 10000),
			BedLine("chr3", 198012430, 198022430),
			BedLine("chr4", 1, 10000),
			BedLine("chr4", 191144276, 191154276),
			BedLine("chr5", 1, 10000),
			BedLine("chr5", 180905260, 180915260),
			BedLine("chr6", 1, 10000),
			BedLine("chr6", 171105067, 171115067),
			BedLine("chr7", 1, 10000),
			BedLine("chr7", 159128663, 159138663),
			BedLine("chr8", 1, 10000),
			BedLine("chr8", 146354022, 146364022),
			BedLine("chr9", 1, 10000),
			BedLine("chr9", 141203431, 141213431),
			BedLine("chr10", 1, 10000),
			BedLine("chr10", 135524747, 135534747),
			BedLine("chr11", 1, 10000),
			BedLine("chr11", 134996516, 135006516),
			BedLine("chr12", 1, 10000),
			BedLine("chr12", 133841895, 133851895),
			BedLine("chr13", 1, 10000),
			BedLine("chr13", 115159878, 115169878),
			BedLine("chr14", 1, 10000),
			BedLine("chr14", 107339540, 107349540),
			BedLine("chr15", 1, 10000),
			BedLine("chr15", 102521392, 102531392),
			BedLine("chr16", 1, 10000),
			BedLine("chr16", 90344753, 90354753),
			BedLine("chr18", 1, 10000),
			BedLine("chr18", 78067248, 78077248),
			BedLine("chr19", 1, 10000),
			BedLine("chr19", 59118983, 59128983),
			BedLine("chr20", 1, 10000),
			BedLine("chr20", 63015520, 63025520),
			BedLine("chr21", 1, 10000),
			BedLine("chr21", 48119895, 48129895),
			BedLine("chr22", 1, 10000),
			BedLine("chr22", 51294566, 51304566),
			BedLine("chrX", 1, 10000),
			BedLine("chrX", 155260560, 155270560),
			BedLine("chrY", 1, 10000),
			BedLine("chrY", 59363566, 59373566)
		};

		BedFile telomeres;
		for(const auto& line : tmp_telomeres) telomeres.append(line);

		CnvList cnvs;
		cnvs.load( TESTDATA("data_in/hrdScore_lst_cnvs.tsv") );

		QCCollection hrd_results = Statistics::hrdScore(cnvs, centromeres, telomeres);

		I_EQUAL(hrd_results.value("QC:2000062", true).asInt() , 2);
		I_EQUAL(hrd_results.value("QC:2000063", true).asInt() , 3);
		I_EQUAL(hrd_results.value("QC:2000064", true).asInt() , 3);

	}


};

