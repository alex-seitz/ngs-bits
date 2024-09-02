
QT       += gui widgets network sql xml xmlpatterns printsupport charts svg
QTPLUGIN += QSQLMYSQL

TARGET = GSvar
TEMPLATE = app
RC_FILE	 = icon.rc

SOURCES += main.cpp\
    Background/BackgroundJobDialog.cpp \
    Background/BackgroundWorkerBase.cpp \
    Background/NGSDCacheInitializer.cpp \
    Background/VariantAnnotator.cpp \
    CfDNAPanelBatchImport.cpp \
    DatabaseServiceRemote.cpp \
    FileLocationWorker.cpp \
    GHGAUploadDialog.cpp \
    GlobalServiceProvider.cpp \
    IGVCommandWorker.cpp \
    IGVSession.cpp \
    IgvLogWidget.cpp \
    IgvSessionManager.cpp \
    ImportDialog.cpp \
    MainWindow.cpp \
    ExternalToolDialog.cpp \
    MaintenanceDialog.cpp \
    PathogenicWtDialog.cpp \
    ReSearchWidget.cpp \
    RepeatExpansionWidget.cpp \
    ReportDialog.cpp \
    Background/ReportWorker.cpp \
    SettingsDialog.cpp \
    TrioDialog.cpp \
    HttpHandler.cpp \
    UserPermissionsEditor.cpp \
    ValidationDialog.cpp \
    ClassificationDialog.cpp \
    ApprovedGenesDialog.cpp \
    GeneWidget.cpp \
    PhenoToGenesDialog.cpp \
    PhenotypeSelector.cpp \
    GenesToRegionsDialog.cpp \
    VariantDetailsDockWidget.cpp \
    SubpanelDesignDialog.cpp \
    SubpanelArchiveDialog.cpp \
    IgvDialog.cpp \
    GapDialog.cpp \
    CnvWidget.cpp \
    GeneSelectorDialog.cpp \
    MultiSampleDialog.cpp \
    SmallVariantSearchWidget.cpp \
    PhenotypeSelectionWidget.cpp \
    RohWidget.cpp \
    DiagnosticStatusWidget.cpp \
    SvWidget.cpp \
    SingleSampleAnalysisDialog.cpp \
    SomaticDialog.cpp \
    FilterEditDialog.cpp \
    SampleDiseaseInfoWidget.cpp \
    DBTableWidget.cpp \
    ProcessedSampleWidget.cpp \
    DBQCWidget.cpp \
    DBComboBox.cpp \
    DBSelector.cpp \
    DiseaseInfoWidget.cpp \
    SequencingRunWidget.cpp \
    AnalysisStatusWidget.cpp \
    VariantTable.cpp \
    SampleSearchWidget.cpp \
    SampleRelationDialog.cpp \
    ProcessedSampleSelector.cpp \
    FilterWidgetCNV.cpp \
    FilterCascadeWidget.cpp \
    FilterWidget.cpp \
    ReportVariantDialog.cpp \
    GSvarHelper.cpp \
    ProcessedSampleDataDeletionDialog.cpp \
    VariantWidget.cpp \
    ProcessingSystemWidget.cpp \
    ProjectWidget.cpp \
    DBEditor.cpp \
    FilterWidgetSV.cpp \
    DBTableAdministration.cpp \
    SequencingRunOverview.cpp \
    SomaticReportVariantDialog.cpp \
    SomaticReportDialog.cpp \
    MidCheckWidget.cpp \
    CnvSearchWidget.cpp \
    VariantValidationWidget.cpp \
    GeneOmimInfoWidget.cpp \
    LoginDialog.cpp \
    GeneInfoDBs.cpp \
    VariantConversionWidget.cpp \
    PasswordDialog.cpp \
    CircosPlotWidget.cpp \
    EmailDialog.cpp \
    CytobandToRegionsDialog.cpp\
    SvSearchWidget.cpp \
    PRSWidget.cpp \
    SomaticDataTransferWidget.cpp \
    EvaluationSheetEditDialog.cpp \
    PublishedVariantsWidget.cpp \
    PreferredTranscriptsWidget.cpp\
    TumorOnlyReportDialog.cpp \
    CfDNAPanelDesignDialog.cpp \
    DiseaseCourseWidget.cpp \
    CfDNAPanelWidget.cpp \
    SomaticVariantInterpreterWidget.cpp \
    AlleleBalanceCalculator.cpp \
    GapClosingDialog.cpp \
    DatabaseServiceLocal.cpp \
    NGSDReplicationWidget.cpp \
    CohortAnalysisWidget.cpp \
    VirusDetectionWidget.cpp \
    cfDNARemovedRegions.cpp \
    ClinvarUploadDialog.cpp \
    LiftOverWidget.cpp \
    BlatWidget.cpp \
    AnalysisInformationWidget.cpp \
    FusionWidget.cpp \
    CohortExpressionDataWidget.cpp \
    GapClosingEditDialog.cpp \
    CausalVariantEditDialog.cpp \
    VariantOpenDialog.cpp \
    PhenotypeSettingsDialog.cpp \
    GenLabImportDialog.cpp \
    ExpressionExonWidget.cpp \
    GeneSelectionDialog.cpp \
    ExpressionOverviewWidget.cpp \
    SplicingWidget.cpp \
    ExpressionGeneWidget.cpp \
    ReportVariantSelectionDialog.cpp \
    BurdenTestWidget.cpp \
    OncoTreeSelector.cpp

HEADERS += MainWindow.h \
    Background/BackgroundJobDialog.h \
    Background/BackgroundWorkerBase.h \
    Background/NGSDCacheInitializer.h \
    Background/VariantAnnotator.h \
    CfDNAPanelBatchImport.h \
    DatabaseServiceRemote.h \
    ExternalToolDialog.h \
    FileLocationWorker.h \
    GHGAUploadDialog.h \
    GlobalServiceProvider.h \
    IGVCommandWorker.h \
    IGVSession.h \
    IgvLogWidget.h \
    IgvSessionManager.h \
    ImportDialog.h \
    MaintenanceDialog.h \
    PathogenicWtDialog.h \
    ReSearchWidget.h \
    RepeatExpansionWidget.h \
    ReportDialog.h \
    Background/ReportWorker.h \
    SettingsDialog.h \
    TrioDialog.h \
    HttpHandler.h \
    UserPermissionsEditor.h \
    ValidationDialog.h \
    ClassificationDialog.h \
    ApprovedGenesDialog.h \
    GeneWidget.h \
    PhenoToGenesDialog.h \
    PhenotypeSelector.h \
    GenesToRegionsDialog.h \
    VariantDetailsDockWidget.h \
    SubpanelDesignDialog.h \
    SubpanelArchiveDialog.h \
    IgvDialog.h \
    GapDialog.h \
    CnvWidget.h \
    GeneSelectorDialog.h \
    MultiSampleDialog.h \
    SmallVariantSearchWidget.h \
    PhenotypeSelectionWidget.h \
    RohWidget.h \
    DiagnosticStatusWidget.h \
    SvWidget.h \
    SingleSampleAnalysisDialog.h \
    SomaticDialog.h \
    FilterEditDialog.h \
    SampleDiseaseInfoWidget.h \
    DBTableWidget.h \
    ProcessedSampleWidget.h \
    DBQCWidget.h \
    DBComboBox.h \
    DBSelector.h \
    DiseaseInfoWidget.h \
    SequencingRunWidget.h \
    AnalysisStatusWidget.h \
    VariantTable.h \
    SampleSearchWidget.h \
    SampleRelationDialog.h \
    ProcessedSampleSelector.h \
    FilterWidgetCNV.h \
    FilterCascadeWidget.h \
    FilterWidget.h \
    ReportVariantDialog.h \
    GSvarHelper.h \
    ProcessedSampleDataDeletionDialog.h \
    VariantWidget.h \
    ProcessingSystemWidget.h \
    ProjectWidget.h \
    DBEditor.h \
    FilterWidgetSV.h \
    DBTableAdministration.h \
    SequencingRunOverview.h \
    SomaticReportVariantDialog.h \
    SomaticReportDialog.h \
    MidCheckWidget.h \
    CnvSearchWidget.h \
    VariantValidationWidget.h \
    GeneOmimInfoWidget.h \
    LoginDialog.h \
    GeneInfoDBs.h \
    VariantConversionWidget.h \
    PasswordDialog.h \
    CircosPlotWidget.h \
    EmailDialog.h \
    CytobandToRegionsDialog.h\
    SvSearchWidget.h \
    PRSWidget.h \
    SomaticDataTransferWidget.h \
    EvaluationSheetEditDialog.h \
    PublishedVariantsWidget.h \
    PreferredTranscriptsWidget.h \
    TumorOnlyReportDialog.h \
    CfDNAPanelDesignDialog.h \
    DiseaseCourseWidget.h \
    CfDNAPanelWidget.h \
    SomaticVariantInterpreterWidget.h \
    AlleleBalanceCalculator.h \
    GapClosingDialog.h \
    DatabaseService.h \
    DatabaseServiceLocal.h \
    NGSDReplicationWidget.h \
    CohortAnalysisWidget.h \
    VirusDetectionWidget.h \
    cfDNARemovedRegions.h \
    ClinvarUploadDialog.h \
    LiftOverWidget.h \
    BlatWidget.h \
    AnalysisInformationWidget.h \
    FusionWidget.h \
    CohortExpressionDataWidget.h \
    GapClosingEditDialog.h \
    CausalVariantEditDialog.h \
    VariantOpenDialog.h \
    PhenotypeSettingsDialog.h \
    GenLabImportDialog.h \
    ExpressionExonWidget.h \
    GeneSelectionDialog.h \
    ExpressionOverviewWidget.h \
    SplicingWidget.h \
    ExpressionGeneWidget.h \
    ReportVariantSelectionDialog.h \
    BurdenTestWidget.h \
    OncoTreeSelector.h

FORMS    += MainWindow.ui \
    Background/BackgroundJobDialog.ui \
    CfDNAPanelBatchImport.ui \
    ExternalToolDialog.ui \
    GHGAUploadDialog.ui \
    IgvLogWidget.ui \
    ImportDialog.ui \
    MaintenanceDialog.ui \
    PathogenicWtDialog.ui \
    ReSearchWidget.ui \
    RepeatExpansionWidget.ui \
    ReportDialog.ui \
    SettingsDialog.ui \
    TrioDialog.ui \
    ClassificationDialog.ui \
    UserPermissionsEditor.ui \
    ValidationDialog.ui \
    ApprovedGenesDialog.ui \
    GeneWidget.ui \
    PhenoToGenesDialog.ui \
    PhenotypeSelector.ui \
    GenesToRegionsDialog.ui \
    VariantDetailsDockWidget.ui \
    SubpanelDesignDialog.ui \
    SubpanelArchiveDialog.ui \
    IgvDialog.ui \
    GapDialog.ui \
    CnvWidget.ui \
    GeneSelectorDialog.ui \
    MultiSampleDialog.ui \
    SmallVariantSearchWidget.ui \
    PhenotypeSelectionWidget.ui \
    RohWidget.ui \
    DiagnosticStatusWidget.ui \
    SvWidget.ui \
    SingleSampleAnalysisDialog.ui \
    SomaticDialog.ui \
    FilterEditDialog.ui \
    SampleDiseaseInfoWidget.ui \
    ProcessedSampleWidget.ui \
    DBQCWidget.ui \
    DiseaseInfoWidget.ui \
    SequencingRunWidget.ui \
    AnalysisStatusWidget.ui \
    SampleSearchWidget.ui \
    SampleRelationDialog.ui \
    ProcessedSampleSelector.ui \
    FilterWidgetCNV.ui \
    FilterCascadeWidget.ui \
    FilterWidget.ui \
    ReportVariantDialog.ui \
    ProcessedSampleDataDeletionDialog.ui \
    VariantWidget.ui \
    ProcessingSystemWidget.ui \
    ProjectWidget.ui \
    FilterWidgetSV.ui \
    DBTableAdministration.ui \
    SequencingRunOverview.ui \
    SomaticReportVariantDialog.ui \
    SomaticReportDialog.ui \
    MidCheckWidget.ui \
    CnvSearchWidget.ui \
    VariantValidationWidget.ui \
    GeneOmimInfoWidget.ui \
    LoginDialog.ui \
    VariantConversionWidget.ui \
    PasswordDialog.ui \
    CircosPlotWidget.ui \
    EmailDialog.ui \
    CytobandToRegionsDialog.ui \
    SvSearchWidget.ui \
    PRSWidget.ui \
    SomaticDataTransferWidget.ui \
    EvaluationSheetEditDialog.ui \
    PublishedVariantsWidget.ui \
    PreferredTranscriptsWidget.ui \
    TumorOnlyReportDialog.ui \
    CfDNAPanelDesignDialog.ui \
    DiseaseCourseWidget.ui \
    CfDNAPanelWidget.ui \
    SomaticVariantInterpreterWidget.ui \
    AlleleBalanceCalculator.ui \
    GapClosingDialog.ui \
    NGSDReplicationWidget.ui \
    CohortAnalysisWidget.ui \
    cfDNARemovedRegions.ui \
    ClinvarUploadDialog.ui \
    BlatWidget.ui \
    AnalysisInformationWidget.ui \
    LiftOverWidget.ui \
    FusionWidget.ui \
    CohortExpressionDataWidget.ui \
    GapClosingEditDialog.ui \
    CausalVariantEditDialog.ui \
    VariantOpenDialog.ui \
    PhenotypeSettingsDialog.ui \
    GenLabImportDialog.ui \
    ExpressionExonWidget.ui \
    GeneSelectionDialog.ui \
    ExpressionOverviewWidget.ui \
    SplicingWidget.ui \
    ExpressionGeneWidget.ui \
    ReportVariantSelectionDialog.ui \
    BurdenTestWidget.ui \
    OncoTreeSelector.ui

include("../app_gui.pri")

#include NGSD library
INCLUDEPATH += $$PWD/../cppNGSD
LIBS += -L$$PWD/../bin -lcppNGSD

#include VISUAL library
INCLUDEPATH += $$PWD/../cppVISUAL
LIBS += -L$$PWD/../bin -lcppVISUAL

RESOURCES += \
    GSvar.qrc

