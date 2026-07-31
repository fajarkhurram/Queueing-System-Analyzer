#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>

#include "ServerStatusWidget.h"
#include "QueueVisualizationWidget.h"
#include "QueueResult.h"
#include "TimeUnitConverter.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onModelChanged();
    void onCalculateClicked();
    void onClearClicked();
    void onTimeUnitChanged(int index);
    void onParameterModeChanged(int index);

private:
    // Whether the arrival/service spin boxes hold a RATE (events per
    // unit time, e.g. customers/hour) or a MEAN TIME (time per event,
    // e.g. hours/customer -- the reciprocal of a rate).
    enum class ParameterMode
    {
        Rate,
        Mean
    };

    // ----- UI builders -----
    void buildUi();
    QWidget* buildModelPanel();
    QWidget* buildInputPanel();
    QWidget* buildServerStatusPanel();
    QWidget* buildResultsPanel();
    QWidget* buildVisualizationPanel();

    void applyStyleSheet();
    void updateFieldAvailability();
    void showStatusMessage(const QString &message, bool isError);
    void displayResult(const QueueResult &result, bool systemValid, int serversUsed);

    // Returns lambda/mu as RATES regardless of the current input mode,
    // converting from mean time (1/x) if necessary.
    bool resolveRates(double &lambdaOut, double &muOut, QString &errorOut) const;

    // ----- Model selection -----
    QButtonGroup *modelGroup;
    QRadioButton *radioMM1;
    QRadioButton *radioMMS;
    QRadioButton *radioMG1;
    QRadioButton *radioMGS;
    QRadioButton *radioGG1;
    QRadioButton *radioGGS;

    // ----- Inputs -----
    QComboBox      *inputTimeUnit;   // "Hours" or "Minutes" -- unit that
                                      // lambda/mu/variances/results are in
    QComboBox      *inputParameterMode; // "Rate" or "Mean Time" -- how to
                                         // interpret the two spin boxes below
    QDoubleSpinBox *inputArrivalRate;   // rate (events/unit time) OR mean
                                         // inter-arrival time, depending on mode
    QDoubleSpinBox *inputServiceRate;   // rate (events/unit time) OR mean
                                         // service time, depending on mode
    QSpinBox       *inputServers;
    QDoubleSpinBox *inputArrivalVariance;
    QDoubleSpinBox *inputServiceVariance;

    QLabel *labelTimeUnit;
    QLabel *labelParameterMode;
    QLabel *labelArrivalRate;   // caption swaps between "Arrival Rate" /
                                 // "Mean Inter-arrival Time"
    QLabel *labelServiceRate;   // caption swaps between "Service Rate" /
                                 // "Mean Service Time"
    QLabel *labelServers;
    QLabel *labelArrivalVariance;
    QLabel *labelServiceVariance;

    // The unit currently reflected by the spin boxes above. Tracked
    // separately from the combo box's current index so onTimeUnitChanged
    // knows what unit the OLD values were in before converting them.
    TimeUnitConverter::TimeUnit currentTimeUnit = TimeUnitConverter::TimeUnit::Hours;

    // Whether inputArrivalRate/inputServiceRate currently hold rates or
    // mean times. Tracked so onParameterModeChanged and onTimeUnitChanged
    // know how to convert the values already in the spin boxes.
    ParameterMode currentParameterMode = ParameterMode::Rate;

    QPushButton *buttonCalculate;
    QPushButton *buttonClear;
    QPushButton *buttonExit;

    // ----- Output -----
    QProgressBar *utilizationBar;
    QLabel *utilizationValueLabel;

    QLabel *cardLq;
    QLabel *cardLs;
    QLabel *cardWq;
    QLabel *cardWs;

    // Secondary "equivalent in the other time unit" captions shown under
    // Wq/Ws (mean-wise conversion demo, e.g. "= 30.0000 min").
    QLabel *cardWqAlt;
    QLabel *cardWsAlt;

    ServerStatusWidget *serverStatusWidget;
    QueueVisualizationWidget *queueVizWidget;

    QLabel *statusMessageLabel;
};
#endif // MAINWINDOW_H
