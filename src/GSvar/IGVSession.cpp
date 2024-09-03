#include "IGVSession.h"
#include "IGVCommandWorker.h"
#include "ClientHelper.h"
#include "GUIHelper.h"
#include "GSvarHelper.h"
#include "Settings.h"
#include "GlobalServiceProvider.h"
#include <QMessageBox>
#include "FileLocationWorker.h"

IGVSession::IGVSession(QWidget* parent, QString igv_name, QString igv_app, QString igv_host, int igv_port, QString genome)
	: parent_(parent)
	, igv_data_()
	, is_initialized_(false)
{
	igv_data_.name = igv_name;
	igv_data_.executable = igv_app;
	igv_data_.host = igv_host;
	igv_data_.port = igv_port;
	igv_data_.genome_file = genome;

    execution_pool_.setMaxThreadCount(1);
    location_execution_pool_.setMaxThreadCount(20);
}

const QString IGVSession::getName() const
{
	return igv_data_.name;
}

int IGVSession::getPort() const
{
	return igv_data_.port;
}

void IGVSession::setInitialized(bool is_initialized)
{
	if (is_initialized_!=is_initialized)
	{
		is_initialized_ = is_initialized;
		emit initializationStatusChanged(is_initialized);
	}
}

bool IGVSession::isInitialized() const
{
	return is_initialized_;
}

void IGVSession::execute(const QStringList& commands, bool init_if_not_done)
{
	//start IGV if not running
	QStringList commands_init;
	if (!isIgvRunning())
	{
		commands_init << "launch IGV";
		commands_init << "genome " + igv_data_.genome_file;
		setInitialized(false);
	}

	//initialize IGV with tracks from current sample
	if (!is_initialized_ && init_if_not_done && !hasRunningCommands())
	{
		Log::info("Initialzing IGV for the current sample");

		QStringList user_selected_commands;
		bool skip_init_for_session = false;
		if (igv_data_.name=="Default IGV")
		{
			user_selected_commands = initRegularIGV(skip_init_for_session);
		}
		if (igv_data_.name=="Virus IGV")
		{
			user_selected_commands = initVirusIGV(skip_init_for_session);
		}

		if (skip_init_for_session)
		{
			setInitialized(true);
		}
		else if (!user_selected_commands.isEmpty())
		{
			commands_init << "new";
			commands_init << user_selected_commands;
			setInitialized(true);
		}
	}

	//add commands to history and create worker commands
	QList<IgvWorkerCommand> worker_commands;
	foreach (const QString& command, QStringList() << commands_init << commands)
	{
		worker_commands << IgvWorkerCommand{next_id_, command};
		++next_id_;
	}

	//add commands to history
	command_history_mutex_.lock();
	foreach (const IgvWorkerCommand& command, worker_commands)
	{
		command_history_ << IGVCommand{command.id, command.text, IGVStatus::QUEUED, "", QDateTime(), 0.0};
	}
	emit historyUpdated(igv_data_.name, command_history_);
	command_history_mutex_.unlock();

	//start commands
	IGVCommandWorker* command_worker = new IGVCommandWorker(igv_data_, worker_commands);
	connect(command_worker, SIGNAL(commandStarted(int)), this, SLOT(updateHistoryStart(int)));
	connect(command_worker, SIGNAL(commandFailed(int, QString, double)), this, SLOT(updateHistoryFailed(int, QString, double)));
	connect(command_worker, SIGNAL(commandFinished(int, QString, double)), this, SLOT(updateHistoryFinished(int, QString, double)));
	connect(command_worker, SIGNAL(processingStarted()), this, SIGNAL(started()));
	connect(command_worker, SIGNAL(processingFinished()), this, SIGNAL(finished()));

    execution_pool_.start(command_worker);
}

void IGVSession::gotoInIGV(const QString& region, bool init_if_not_done)
{
	execute(QStringList() << ("goto " + region), init_if_not_done);
}

void IGVSession::loadFileInIGV(QString filename, bool init_if_not_done)
{
	QStringList commands;
	if (ClientHelper::isClientServerMode())
	{
		commands << ("SetAccessToken " + LoginManager::userToken() + " *" + Settings::string("server_host") + "*");
	}
	commands << ("load \"" + ClientHelper::stripSecureToken(Helper::canonicalPath(filename)) + "\"");

	execute(commands, init_if_not_done);
}

void IGVSession::waitForDone()
{
	execution_pool_.waitForDone();
}

bool IGVSession::isIgvRunning()
{
	//commands are being executed > IGV is running
	if (hasRunningCommands()) return true;

	//no commands are executed > try to open a connection
	QTcpSocket socket;
	socket.connectToHost(igv_data_.host, igv_data_.port);
	if (!socket.waitForConnected(1500))
	{
		Log::info("Could not open socket to IGV");
		return false;
	}
	socket.abort();

	return true;
}

bool IGVSession::hasRunningCommands()
{
	return execution_pool_.activeThreadCount()>0;
}

QList<IGVCommand> IGVSession::getCommands() const
{
	command_history_mutex_.lock();
	QList<IGVCommand> history = command_history_;
	command_history_mutex_.unlock();
	return history;
}

void IGVSession::clear()
{
	command_history_mutex_.lock();
	command_history_.clear();
	command_history_mutex_.unlock();

	setInitialized(false);

	execute(QStringList() << "new", false);
}

QString IGVSession::statusToString(IGVStatus status)
{
	switch(status)
	{
		case IGVStatus::QUEUED:
			return "queued";
			break;
		case IGVStatus::STARTED:
			return "started";
			break;
		case IGVStatus::FINISHED:
			return "finished";
			break;
		case IGVStatus::FAILED:
			return "failed";
			break;
	}
	THROW(ProgrammingException, "Unknown IGV status " + QString::number(status));
}

QColor IGVSession::statusToColor(IGVStatus status)
{
	switch(status)
	{
		case IGVStatus::QUEUED:
			return Qt::lightGray;
			break;
		case IGVStatus::STARTED:
			return QColor("#90EE90");
			break;
		case IGVStatus::FINISHED:
			return QColor("#44BB44");
			break;
		case IGVStatus::FAILED:
			return QColor("#FF0000");
			break;
	}
	THROW(ProgrammingException, "Unknown IGV status " + QString::number(status));
}

QStringList IGVSession::initRegularIGV(bool& skip_init_for_session)
{


    MainWindow* main_window = GlobalServiceProvider::mainWindow();

    IgvDialog dlg(parent_);
	AnalysisType analysis_type = main_window->getCurrentAnalysisType();

    FileLocationList bam_files;
    FileLocationWorker* file_location_bam_worker = new FileLocationWorker(PathType::BAM, analysis_type, bam_files);
    location_execution_pool_.start(file_location_bam_worker);

    //sample BAF file(s)
    FileLocationList baf_files;
    FileLocationWorker* file_location_baf_worker = new FileLocationWorker(PathType::BAF, analysis_type, baf_files);
    location_execution_pool_.start(file_location_baf_worker);

    //analysis VCF
    FileLocationList vcf_file;
    FileLocationWorker* file_location_vcf_worker = new FileLocationWorker(PathType::VCF, analysis_type, vcf_file);
    location_execution_pool_.start(file_location_vcf_worker);

    //analysis SV file
    FileLocationList bedpe_file;
    FileLocationWorker* file_location_sv_worker = new FileLocationWorker(PathType::STRUCTURAL_VARIANTS, analysis_type, bedpe_file);
    location_execution_pool_.start(file_location_sv_worker);

    //CNV files
    FileLocationList cnv_files;
    FileLocationWorker* file_location_cnv_worker = new FileLocationWorker(PathType::CNV_RAW_DATA_CALL_REGIONS, analysis_type, cnv_files);
    location_execution_pool_.start(file_location_cnv_worker);

    //Manta evidence file(s)
    FileLocationList manta_evidence_files;
    FileLocationWorker* file_location_manta_worker = new FileLocationWorker(PathType::MANTA_EVIDENCE, analysis_type, manta_evidence_files);
    location_execution_pool_.start(file_location_manta_worker);

    location_execution_pool_.waitForDone();
    Log::info("BAM files count: " + QString::number(bam_files_.size()));
    //sample BAM file(s)
    foreach(const FileLocation& file, bam_files)
    {
        dlg.addFile(file, true);
    }
    // BAFs
    foreach(const FileLocation& file, baf_files)
    {
        if(analysis_type == SOMATIC_PAIR && !file.id.contains("somatic")) continue;
        dlg.addFile(file, true);
    }

    // VCFs
    if (vcf_file.size()>0)
    {
        bool igv_default_small = Settings::boolean("igv_default_small", true);
        dlg.addFile(vcf_file[0], igv_default_small);
    }

    // BEDPE
    if (bedpe_file.size()>0)
    {
        bool igv_default_sv = Settings::boolean("igv_default_sv", true);
        dlg.addFile(bedpe_file[0], igv_default_sv);
    }

    // CNVs
    foreach(const FileLocation& file, cnv_files)
    {
        dlg.addFile(file, true);
    }
    // Manta evidence
    foreach(const FileLocation& file, manta_evidence_files)
    {
        dlg.addFile(file, false);
    }


    // KEEP AS IS

    //target region
	if (main_window->targetRegion().isValid())
    {
		QString roi_file = GSvarHelper::localRoiFolder() + main_window->targetRegion().name + ".bed";
		main_window->targetRegion().regions.store(roi_file);

        dlg.addFile(FileLocation{"target region (selected in GSvar)", PathType::OTHER, roi_file, true}, true);
    }

    //amplicon file (of processing system)
    try
    {
        NGSD db;
		int sys_id = db.processingSystemIdFromProcessedSample(main_window->germlineReportSample());
        BedFile ampilicons = GlobalServiceProvider::database().processingSystemAmplicons(sys_id, true);
        if (!ampilicons.isEmpty())
        {
            QString amp_file = GSvarHelper::localRoiFolder() + db.getProcessingSystemData(sys_id).name_short + "_amplicons.bed";
            ampilicons.store(amp_file);

            dlg.addFile(FileLocation{"amplicons track (of processing system)", PathType::OTHER, amp_file, true}, true);
        }
    }
    catch(...) {} //Nothing to do here

    //sample low-coverage
	bool igv_default_lowcov = Settings::boolean("igv_default_lowcov", true);
	if (analysis_type==SOMATIC_SINGLESAMPLE || analysis_type==SOMATIC_PAIR)
    {
        FileLocationList som_low_cov_files = GlobalServiceProvider::fileLocationProvider().getSomaticLowCoverageFiles(false);
        foreach(const FileLocation& loc, som_low_cov_files)
        {
            if(loc.filename.contains("somatic_custom_panel_stat"))
            {
				dlg.addFile(FileLocation{loc.id + " (somatic custom panel)", PathType::LOWCOV_BED, loc.filename, QFile::exists(loc.filename)}, igv_default_lowcov);
            }
            else
            {
				dlg.addFile(loc, igv_default_lowcov);
            }
        }
    }
    else
    {
        FileLocationList low_cov_files = GlobalServiceProvider::fileLocationProvider().getLowCoverageFiles(true);
        foreach(const FileLocation& file, low_cov_files)
        {
			dlg.addFile(file, igv_default_lowcov);
        }
    }

	//custom tracks
	QStringList entries = Settings::stringList("igv_menu");
	foreach(QString entry, entries)
	{
		QStringList parts = entry.trimmed().split("\t");
		if(parts.count()!=3) continue;

		QString name = parts[0];
		bool checked_by_default = parts[1]=="1";
		QString filename = parts[2];
		dlg.addFile(FileLocation{name, PathType::OTHER, filename, QFile::exists(filename)}, checked_by_default);
	}

    //related RNA tracks
    if (LoginManager::active())
    {
        NGSD db;

		QString sample_id = db.sampleId(main_window->getCurrentFileName(), false);
        if (sample_id!="")
        {
            foreach (int rna_sample_id, db.relatedSamples(sample_id.toInt(), "same sample", "RNA"))
            {
                // iterate over all processed RNA samples
                foreach (const QString& rna_ps_id, db.getValues("SELECT id FROM processed_sample WHERE sample_id=:0", QString::number(rna_sample_id)))
                {
                    //add RNA BAM
                    FileLocation rna_bam_file = GlobalServiceProvider::database().processedSamplePath(rna_ps_id, PathType::BAM);
                    if (rna_bam_file.exists) dlg.addFile(rna_bam_file, false);

                    //add fusions BAM
                    FileLocation rna_fusions_bam_file = GlobalServiceProvider::database().processedSamplePath(rna_ps_id, PathType::FUSIONS_BAM);
                    if (rna_fusions_bam_file.exists) dlg.addFile(rna_fusions_bam_file, false);

                    //add splicing BED
                    FileLocation rna_splicing_bed_file = GlobalServiceProvider::database().processedSamplePath(rna_ps_id, PathType::SPLICING_BED);
                    if (rna_splicing_bed_file.exists) dlg.addFile(rna_splicing_bed_file, false);
                }
            }
        }
    }

	// switch to MainWindow to prevent dialog to appear behind other widgets
	parent_->raise();
	parent_->activateWindow();
	parent_->setFocus();

	//execute dialog
	if (!dlg.exec())
	{
		skip_init_for_session = dlg.skipInitialization();
		return QStringList();
	}

	return filesToCommands(dlg.filesToLoad());
}

QStringList IGVSession::initVirusIGV(bool& skip_init_for_session)
{
    IgvDialog dlg(parent_);

    //sample BAM file(s)
    FileLocationList bams = GlobalServiceProvider::fileLocationProvider().getViralBamFiles(false);
    if (bams.isEmpty())
    {
        QMessageBox::information(parent_, "BAM files not found", "There are no BAM files associated with the virus!");
		return QStringList();
    }

    foreach(const FileLocation& file, bams)
    {
        dlg.addFile(file, true);
    }

	// switch to MainWindow to prevent dialog to appear behind other widgets
	parent_->raise();
	parent_->activateWindow();
	parent_->setFocus();

	//execute dialog
	if (!dlg.exec())
	{
		skip_init_for_session = dlg.skipInitialization();
		return QStringList();
	}

	return filesToCommands(dlg.filesToLoad());
}

QStringList IGVSession::filesToCommands(QStringList files_to_load)
{
    QStringList init_commands;

	init_commands.append("new");

    if (ClientHelper::isClientServerMode()) init_commands.append("SetAccessToken " + LoginManager::userToken() + " *" + Settings::string("server_host") + "*");

    //load non-BAM files
    foreach(QString file, files_to_load)
    {
        if (!ClientHelper::isBamFile(file)) init_commands.append("load \"" + Helper::canonicalPath(file) + "\"");
    }

    //collapse tracks
    init_commands.append("setSleepInterval 0");
    init_commands.append("collapse");

    //load BAM files
    foreach(QString file, files_to_load)
    {
        if (ClientHelper::isBamFile(file)) init_commands.append("load \"" + Helper::canonicalPath(file) + "\"");
    }
    init_commands.append("viewaspairs");
    init_commands.append("colorBy UNEXPECTED_PAIR");

	return init_commands;
}

void IGVSession::updateHistoryStart(int id)
{
	//ignore commands without valid ID (we used them e.g. for checking if IGV runs)
	if (id==-1) return;

	command_history_mutex_.lock();

	for (int i=0; i<command_history_.count(); ++i)
	{
		IGVCommand& command = command_history_[i];
		if (command.id==id)
		{
			command.status = IGVStatus::STARTED;
			command.execution_start_time = QDateTime::currentDateTime();
		}
	}

	emit historyUpdated(igv_data_.name, command_history_);

	command_history_mutex_.unlock();
}

void IGVSession::updateHistoryFinished(int id, QString answer, double sec_elapsed)
{
	//ignore commands without valid ID (we used them e.g. for checking if IGV runs)
	if (id==-1) return;

	command_history_mutex_.lock();

	for (int i=0; i<command_history_.count(); ++i)
	{
		IGVCommand& command = command_history_[i];
		if (command.id==id)
		{
			command.status = IGVStatus::FINISHED;
			command.answer = answer;
			command.execution_duration_sec = sec_elapsed;
		}
	}

	emit historyUpdated(igv_data_.name, command_history_);

	command_history_mutex_.unlock();
}

void IGVSession::updateHistoryFailed(int id, QString error, double sec_elapsed)
{
	//ignore commands without valid ID (we used them e.g. for checking if IGV runs)
	if (id==-1) return;

	command_history_mutex_.lock();

	for (int i=0; i<command_history_.count(); ++i)
	{
		IGVCommand& command = command_history_[i];
		if (command.id==id)
		{
			command.status = IGVStatus::FAILED;
			command.answer = error;
			command.execution_duration_sec = sec_elapsed;
		}
	}

	emit historyUpdated(igv_data_.name, command_history_);

	command_history_mutex_.unlock();
}

void IGVSession::addBamFiles(FileLocationList bam_files)
{
    Log::info("addBamFiles");
    bam_files_ = bam_files;
}

void IGVSession::addBafFiles(FileLocationList baf_files)
{
    Log::info("addBafFiles");
    baf_files_ = baf_files;
}

void IGVSession::addVcfFile(FileLocation vcf_file)
{
    Log::info("addVcfFile");
    vcf_file_ = vcf_file;
}

void IGVSession::addBedpeFile(FileLocation bedpe_file)
{
    Log::info("addBedpeFile");
    bedpe_file_ = bedpe_file;
}

void IGVSession::addCnvFiles(FileLocationList cnv_files)
{
    Log::info("addCnvFiles");
    cnv_files_ = cnv_files;
}

void IGVSession::addMantaEvidenceFiles(FileLocationList manta_evidence_files)
{
    Log::info("addMantaEvidenceFiles");
    manta_evidence_files_ = manta_evidence_files;
}
