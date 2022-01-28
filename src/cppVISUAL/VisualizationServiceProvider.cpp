#include "VisualizationServiceProvider.h"

VisualizationServiceProvider::VisualizationServiceProvider()
{
}

VisualizationServiceProvider::~VisualizationServiceProvider()
{
}

VisualizationServiceProvider& VisualizationServiceProvider::instance()
{
	static VisualizationServiceProvider instance;
	return instance;
}

void VisualizationServiceProvider::checkGenomeSet()
{
	if (instance().genome_idx_.isNull())
	{
		THROW(ProgrammingException, "Genome file not set in VisualizationServiceProvider!");
	}
}

void VisualizationServiceProvider::checkTranscriptListSet()
{
	if (instance().transcript_list_==nullptr)
	{
		THROW(ProgrammingException, "Transcript list not set in VisualizationServiceProvider!");
	}
}

void VisualizationServiceProvider::setGenomeFile(QString filename)
{
	instance().genome_idx_ = QSharedPointer<FastaFileIndex>(new FastaFileIndex(filename));

	//init chromosome list (ordered correctly)
	instance().valid_chrs_ = instance().genome_idx_->names();
	std::sort(instance().valid_chrs_.begin(), instance().valid_chrs_.end(), [](const QString& s1, const QString& s2){ Chromosome c1(s1); Chromosome c2(s2); return (c1.num()<1004 || c2.num()<1004) ? c1<c2 : s1<s2;  });
}

void VisualizationServiceProvider::setTranscriptList(const TranscriptList& transcripts)
{
	instance().transcript_list_ = &transcripts;

	//init transcripts index
	instance().transcript_list_idx_ = QSharedPointer<ChromosomalIndex<TranscriptList>>(new ChromosomalIndex<TranscriptList>(transcripts));

	//init gene and transcript indices
	for(int i=0; i<transcripts.size(); ++i)
	{
		const Transcript& trans = transcripts[i];

		instance().gene_to_trans_indices_[trans.gene()] << i;
		instance().trans_to_index_[trans.name()] = i;
	}
}

const QByteArrayList& VisualizationServiceProvider::validChromosomes()
{
	checkGenomeSet();

	return instance().valid_chrs_;
}

const FastaFileIndex& VisualizationServiceProvider::genomeIndex()
{
	checkGenomeSet();

	return *(instance().genome_idx_.data());
}


bool VisualizationServiceProvider::isTranscriptName(const QByteArray& name)
{
	checkTranscriptListSet();

	return instance().trans_to_index_.contains(name);
}

const Transcript& VisualizationServiceProvider::transcript(const QByteArray& name)
{
	checkTranscriptListSet();

	if (!instance().trans_to_index_.contains(name))
	{
		THROW(ArgumentException, "Invalid transcript name '" + name + "'!");
	}

	int index = instance().trans_to_index_[name];
	return instance().transcript_list_->operator[](index);
}

QByteArrayList VisualizationServiceProvider::transcripts(const Chromosome& chr, int start, int end)
{
	checkTranscriptListSet();

	QByteArrayList output;
	QVector<int> indices  = instance().transcript_list_idx_->matchingIndices(chr, start, end);
	foreach(int index, indices)
	{
		output << instance().transcript_list_->operator[](index).name();
	}
	return output;
}

bool VisualizationServiceProvider::isGeneName(const QByteArray& name)
{
	checkTranscriptListSet();

	return instance().gene_to_trans_indices_.contains(name);
}

QByteArrayList VisualizationServiceProvider::geneTranscripts(const QByteArray& name)
{
	checkTranscriptListSet();

	if (!instance().gene_to_trans_indices_.contains(name))
	{
		THROW(ArgumentException, "Invalid gene name '" + name + "'!");
	}

	QByteArrayList output;
	foreach(int index, instance().gene_to_trans_indices_[name])
	{
		output << instance().transcript_list_->operator[](index).name();
	}
	return output;
}
