#ifndef VISUALIZATIONSERVICEPROVIDER_H
#define VISUALIZATIONSERVICEPROVIDER_H

#include "cppVISUAL_global.h"
#include <QWidget>
#include "Transcript.h"
#include "FastaFileIndex.h"

//Service provider for visualization
class CPPVISUALSHARED_EXPORT VisualizationServiceProvider
	: QWidget
{
public:
	//Sets the genome file location.
	static void setGenomeFile(QString filename);
	//Sets the *sorted* transcript list. Attention: only a reference to the transcript list is stored. Make sure that transcript list exists during the whole execution of the visualization program.
	static void setTranscriptList(const TranscriptList& transcripts);

	//Returns the genome index.
	static const FastaFileIndex& genomeIndex();
	//Returns a ordered list of chromosome names with 'chr' prefix.
	static const QByteArrayList& validChromosomes();

	//Returns if the name is a valid transcript name
	static bool isTranscriptName(const QByteArray& name);
	//Return the given transcript, or throws a ArgumentException if it is not a valid transcript.
	static const Transcript& transcript(const QByteArray& name);
	//Return the transcript list overlapping a certain genomic region.
	static QByteArrayList transcripts(const Chromosome& chr, int start, int end);

	//Returns if the name is a valid gene name
	static bool isGeneName(const QByteArray& name);
	//Return the transcript list of a gene, or throws a ArgumentException if it is not a valid gene.
	static QByteArrayList geneTranscripts(const QByteArray& name);

protected:
	VisualizationServiceProvider();
	~VisualizationServiceProvider();
	static VisualizationServiceProvider& instance();
	static void checkGenomeSet();
	static void checkTranscriptListSet();

	QSharedPointer<FastaFileIndex> genome_idx_;
	QByteArrayList valid_chrs_;

	const TranscriptList* transcript_list_ = nullptr;
	QSharedPointer<ChromosomalIndex<TranscriptList>> transcript_list_idx_;
	QHash<QByteArray, int> trans_to_index_;
	QHash<QByteArray, QSet<int>> gene_to_trans_indices_;
};


#endif // VISUALIZATIONSERVICEPROVIDER_H
