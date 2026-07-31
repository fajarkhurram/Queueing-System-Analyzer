#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QFont>
#include <QMessageBox>
#include <QScrollArea>
#include <cmath>

#include "MM1.h"
#include "MMS.h"
#include "MG1.h"
#include "MGS.h"
#include "GG1.h"
#include "GGS.h"
#include "InputValidator.h"
#include "TimeUnitConverter.h"

namespace {

// Small helper that builds one colorful "metric card" used in the
// results panel (Lq / Ls / Wq / Ws). If altLabelOut is non-null, an
// extra small caption is added underneath the value -- used to show the
// same time value converted into the other time unit (mean-wise
// conversion), e.g. "= 30.0000 min".
QFrame* makeMetricCard(const QString &caption, const QString &accentColor,
                        QLabel **valueLabelOut, QLabel **altLabelOut = nullptr)
{
    auto *card = new QFrame();
    card->setObjectName("metricCard");
    card->setProperty("accent", accentColor);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(2);

    auto *captionLabel = new QLabel(caption);
    captionLabel->setObjectName("metricCaption");

    auto *valueLabel = new QLabel("--");
    valueLabel->setObjectName("metricValue");
    valueLabel->setStyleSheet(QString("color:%1;").arg(accentColor));

    layout->addWidget(captionLabel);
    layout->addWidget(valueLabel);

    if (altLabelOut)
    {
        auto *altLabel = new QLabel("");
        altLabel->setObjectName("metricAlt");
        layout->addWidget(altLabel);
        *altLabelOut = altLabel;
    }

    *valueLabelOut = valueLabel;
    return card;
}

QString formatNumber(double value)
{
    if (!std::isfinite(value))
    {
        return "--";
    }
    return QString::number(value, 'f', 4);
}

} // namespace


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Queueing System Analyzer");
    resize(1200, 820);
    buildUi();
    applyStyleSheet();
    updateFieldAvailability();
}

MainWindow::~MainWindow() = default;


// ============================================================
//  UI CONSTRUCTION
// ============================================================

void MainWindow::buildUi()
{
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *central = new QWidget();
    scrollArea->setWidget(central);
    setCentralWidget(scrollArea);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    // ----- Title banner -----
    auto *titleLabel = new QLabel("QUEUEING SYSTEM ANALYZER");
    titleLabel->setObjectName("titleBanner");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // ----- Top row: left column (models + servers) | right column (inputs + results) -----
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(14);

    auto *leftColumn = new QVBoxLayout();
    leftColumn->setSpacing(14);
    leftColumn->addWidget(buildModelPanel());
    leftColumn->addWidget(buildServerStatusPanel());
    leftColumn->addStretch();

    auto *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(14);
    rightColumn->addWidget(buildInputPanel());
    rightColumn->addWidget(buildResultsPanel());

    auto *leftContainer = new QWidget();
    leftContainer->setLayout(leftColumn);
    leftContainer->setMinimumWidth(300);
    leftContainer->setMaximumWidth(340);

    auto *rightContainer = new QWidget();
    rightContainer->setLayout(rightColumn);

    topRow->addWidget(leftContainer);
    topRow->addWidget(rightContainer, 1);

    mainLayout->addLayout(topRow);

    // ----- Queue visualization -----
    mainLayout->addWidget(buildVisualizationPanel());

    // ----- Status / message bar -----
    statusMessageLabel = new QLabel("Choose a model, fill in the parameters and press Calculate.");
    statusMessageLabel->setObjectName("statusMessage");
    statusMessageLabel->setAlignment(Qt::AlignCenter);
    statusMessageLabel->setWordWrap(true);
    mainLayout->addWidget(statusMessageLabel);
}

QWidget* MainWindow::buildModelPanel()
{
    auto *box = new QGroupBox("Queue Models");
    box->setObjectName("panelBox");

    auto *layout = new QVBoxLayout(box);
    layout->setSpacing(10);

    radioMM1 = new QRadioButton("M / M / 1");
    radioMMS = new QRadioButton("M / M / S");
    radioMG1 = new QRadioButton("M / G / 1");
    radioMGS = new QRadioButton("M / G / S");
    radioGG1 = new QRadioButton("G / G / 1");
    radioGGS = new QRadioButton("G / G / S");

    modelGroup = new QButtonGroup(this);
    modelGroup->addButton(radioMM1, 0);
    modelGroup->addButton(radioMMS, 1);
    modelGroup->addButton(radioMG1, 2);
    modelGroup->addButton(radioMGS, 3);
    modelGroup->addButton(radioGG1, 4);
    modelGroup->addButton(radioGGS, 5);

    radioMM1->setChecked(true);

    layout->addWidget(radioMM1);
    layout->addWidget(radioMMS);
    layout->addWidget(radioMG1);
    layout->addWidget(radioMGS);
    layout->addWidget(radioGG1);
    layout->addWidget(radioGGS);

    connect(modelGroup, &QButtonGroup::idClicked, this, &MainWindow::onModelChanged);

    return box;
}

QWidget* MainWindow::buildInputPanel()
{
    auto *box = new QGroupBox("Input Parameters");
    box->setObjectName("panelBox");

    auto *outerLayout = new QVBoxLayout(box);

    auto *form = new QFormLayout();
    form->setSpacing(10);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // ----- Time unit selector -----
    // Changing this converts every rate/mean/variance field already
    // entered into the new unit, rather than just relabeling them.
    inputTimeUnit = new QComboBox();
    inputTimeUnit->addItem("Hours", static_cast<int>(TimeUnitConverter::TimeUnit::Hours));
    inputTimeUnit->addItem("Minutes", static_cast<int>(TimeUnitConverter::TimeUnit::Minutes));
    inputTimeUnit->setCurrentIndex(0);

    labelTimeUnit = new QLabel("Time Unit");
    form->addRow(labelTimeUnit, inputTimeUnit);

    // ----- Input mode selector -----
    // Lets the user type lambda/mu directly as RATES (e.g. "5 /hr"), or
    // instead type the MEAN TIME between events (e.g. "12 min between
    // arrivals") and have the calculator invert it into a rate (rate = 1/mean)
    // before running the model. Switching this converts the values already
    // in the two boxes below, it does not just relabel them.
    inputParameterMode = new QComboBox();
    inputParameterMode->addItem("Rate (\xCE\xBB, \xCE\xBC)", static_cast<int>(ParameterMode::Rate));
    inputParameterMode->addItem("Mean Time (1/\xCE\xBB, 1/\xCE\xBC)", static_cast<int>(ParameterMode::Mean));
    inputParameterMode->setCurrentIndex(0);

    labelParameterMode = new QLabel("Input As");
    form->addRow(labelParameterMode, inputParameterMode);

    inputArrivalRate = new QDoubleSpinBox();
    inputArrivalRate->setRange(0.0, 1000000.0);
    inputArrivalRate->setDecimals(3);
    inputArrivalRate->setValue(5.0);
    inputArrivalRate->setSuffix(TimeUnitConverter::rateSuffix(TimeUnitConverter::TimeUnit::Hours));

    inputServiceRate = new QDoubleSpinBox();
    inputServiceRate->setRange(0.0, 1000000.0);
    inputServiceRate->setDecimals(3);
    inputServiceRate->setValue(10.0);
    inputServiceRate->setSuffix(TimeUnitConverter::rateSuffix(TimeUnitConverter::TimeUnit::Hours));

    inputServers = new QSpinBox();
    inputServers->setRange(1, 1000);
    inputServers->setValue(2);

    inputArrivalVariance = new QDoubleSpinBox();
    inputArrivalVariance->setRange(0.0, 1000000.0);
    inputArrivalVariance->setDecimals(4);
    inputArrivalVariance->setValue(0.01);

    inputServiceVariance = new QDoubleSpinBox();
    inputServiceVariance->setRange(0.0, 1000000.0);
    inputServiceVariance->setDecimals(4);
    inputServiceVariance->setValue(0.01);

    labelArrivalRate = new QLabel("\xCE\xBB  Arrival Rate");
    form->addRow(labelArrivalRate, inputArrivalRate);

    labelServiceRate = new QLabel("\xCE\xBC  Service Rate");
    form->addRow(labelServiceRate, inputServiceRate);

    labelServers = new QLabel("Servers");
    form->addRow(labelServers, inputServers);

    labelArrivalVariance = new QLabel("Arrival Variance");
    form->addRow(labelArrivalVariance, inputArrivalVariance);

    labelServiceVariance = new QLabel("Service Variance");
    form->addRow(labelServiceVariance, inputServiceVariance);

    outerLayout->addLayout(form);

    connect(inputTimeUnit, &QComboBox::currentIndexChanged, this, &MainWindow::onTimeUnitChanged);
    connect(inputParameterMode, &QComboBox::currentIndexChanged, this, &MainWindow::onParameterModeChanged);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(10);

    buttonCalculate = new QPushButton("CALCULATE");
    buttonCalculate->setObjectName("calculateButton");
    buttonClear = new QPushButton("CLEAR");
    buttonClear->setObjectName("clearButton");
    buttonExit = new QPushButton("EXIT");
    buttonExit->setObjectName("exitButton");

    buttonRow->addWidget(buttonCalculate);
    buttonRow->addWidget(buttonClear);
    buttonRow->addWidget(buttonExit);

    outerLayout->addSpacing(8);
    outerLayout->addLayout(buttonRow);

    connect(buttonCalculate, &QPushButton::clicked, this, &MainWindow::onCalculateClicked);
    connect(buttonClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(buttonExit, &QPushButton::clicked, this, &MainWindow::close);

    return box;
}

QWidget* MainWindow::buildServerStatusPanel()
{
    auto *box = new QGroupBox("Server Status");
    box->setObjectName("panelBox");

    auto *layout = new QVBoxLayout(box);
    serverStatusWidget = new ServerStatusWidget();
    layout->addWidget(serverStatusWidget);

    return box;
}

QWidget* MainWindow::buildResultsPanel()
{
    auto *box = new QGroupBox("Performance Metrics");
    box->setObjectName("panelBox");

    auto *layout = new QVBoxLayout(box);
    layout->setSpacing(12);

    // Utilization bar
    auto *utilRow = new QHBoxLayout();
    auto *utilCaption = new QLabel("Utilization (\xCF\x81)");
    utilCaption->setObjectName("metricCaption");

    utilizationBar = new QProgressBar();
    utilizationBar->setRange(0, 100);
    utilizationBar->setValue(0);
    utilizationBar->setTextVisible(false);
    utilizationBar->setFixedHeight(22);

    utilizationValueLabel = new QLabel("0%");
    utilizationValueLabel->setObjectName("utilizationValue");
    utilizationValueLabel->setFixedWidth(56);
    utilizationValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(utilCaption);
    utilRow->addWidget(utilizationBar, 1);
    utilRow->addWidget(utilizationValueLabel);
    layout->addLayout(utilRow);

    // Metric cards grid: Lq, Ls, Wq, Ws
    auto *grid = new QGridLayout();
    grid->setSpacing(12);

    grid->addWidget(makeMetricCard("Lq  (avg. in queue)", "#6C5CE7", &cardLq), 0, 0);
    grid->addWidget(makeMetricCard("Ls  (avg. in system)", "#00B894", &cardLs), 0, 1);
    grid->addWidget(makeMetricCard("Wq  (avg. wait time)", "#E17055", &cardWq, &cardWqAlt), 1, 0);
    grid->addWidget(makeMetricCard("Ws  (avg. time in system)", "#0984E3", &cardWs, &cardWsAlt), 1, 1);

    layout->addLayout(grid);

    return box;
}

QWidget* MainWindow::buildVisualizationPanel()
{
    auto *box = new QGroupBox("Queue Visualization");
    box->setObjectName("panelBox");

    auto *layout = new QVBoxLayout(box);
    queueVizWidget = new QueueVisualizationWidget();
    layout->addWidget(queueVizWidget);

    return box;
}


// ============================================================
//  STYLE
// ============================================================

void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Segoe UI", "Cantarell", sans-serif;
            font-size: 13px;
            color: #2D3436;
        }

        QScrollArea, QWidget#centralwidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #EAF0FF, stop:1 #F7F3FF);
        }

        QLabel#titleBanner {
            font-size: 26px;
            font-weight: 800;
            color: white;
            padding: 16px;
            border-radius: 14px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6C5CE7, stop:0.5 #A29BFE, stop:1 #00B894);
        }

        QGroupBox#panelBox {
            background: white;
            border: 2px solid #E0E6FF;
            border-radius: 14px;
            margin-top: 14px;
            padding: 12px;
            font-weight: 700;
            color: #34314C;
        }

        QGroupBox#panelBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 2px 10px;
            background: #6C5CE7;
            color: white;
            border-radius: 8px;
        }

        QRadioButton {
            padding: 6px;
            font-weight: 600;
        }

        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }

        QDoubleSpinBox, QSpinBox {
            background: #F5F6FF;
            border: 2px solid #DCE0FF;
            border-radius: 8px;
            padding: 4px 8px;
            min-height: 24px;
        }

        QDoubleSpinBox:focus, QSpinBox:focus {
            border: 2px solid #6C5CE7;
        }

        QPushButton {
            border: none;
            border-radius: 10px;
            padding: 10px 14px;
            font-weight: 800;
            color: white;
        }

        QPushButton#calculateButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00B894, stop:1 #55EFC4);
        }
        QPushButton#calculateButton:hover { background: #00997A; }

        QPushButton#clearButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FDCB6E, stop:1 #FFEAA7);
            color: #6D4C00;
        }
        QPushButton#clearButton:hover { background: #E6B95C; }

        QPushButton#exitButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #E74C3C, stop:1 #FF7675);
        }
        QPushButton#exitButton:hover { background: #C0392B; }

        QFrame#metricCard {
            background: #F8F9FF;
            border: 2px solid #ECEBFF;
            border-radius: 12px;
        }

        QLabel#metricCaption {
            color: #636E72;
            font-weight: 600;
            font-size: 11px;
        }

        QLabel#metricValue {
            font-size: 22px;
            font-weight: 800;
        }

        QLabel#metricAlt {
            color: #9A96C4;
            font-size: 10px;
            font-weight: 600;
        }

        QLabel#utilizationValue {
            font-weight: 700;
            color: #34314C;
        }

        QProgressBar {
            background: #ECECFF;
            border-radius: 11px;
        }

        QProgressBar::chunk {
            border-radius: 11px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00B894, stop:0.6 #FDCB6E, stop:1 #E74C3C);
        }

        QLabel#statusMessage {
            padding: 10px;
            border-radius: 10px;
            background: #EFF3FF;
            font-weight: 600;
            color: #2D3436;
        }
    )");
}


// ============================================================
//  BEHAVIOR
// ============================================================

void MainWindow::updateFieldAvailability()
{
    bool needsServers = radioMMS->isChecked() || radioMGS->isChecked() || radioGGS->isChecked();
    bool needsArrivalVariance = radioGG1->isChecked() || radioGGS->isChecked();
    bool needsServiceVariance = radioMG1->isChecked() || radioMGS->isChecked()
                                 || radioGG1->isChecked() || radioGGS->isChecked();

    labelServers->setVisible(needsServers);
    inputServers->setVisible(needsServers);

    labelArrivalVariance->setVisible(needsArrivalVariance);
    inputArrivalVariance->setVisible(needsArrivalVariance);

    labelServiceVariance->setVisible(needsServiceVariance);
    inputServiceVariance->setVisible(needsServiceVariance);
}

void MainWindow::onModelChanged()
{
    updateFieldAvailability();
    showStatusMessage("Model changed — press Calculate to refresh the results.", false);
}

bool MainWindow::resolveRates(double &lambdaOut, double &muOut, QString &errorOut) const
{
    double arrivalField = inputArrivalRate->value();
    double serviceField = inputServiceRate->value();

    if (currentParameterMode == ParameterMode::Rate)
    {
        lambdaOut = arrivalField;
        muOut = serviceField;
        return true;
    }

    // Mean mode: the two boxes hold MEAN TIMES (e.g. "12 minutes between
    // arrivals"), not rates. The queue models below always need rates
    // (lambda, mu), so invert here: rate = 1 / mean.
    if (arrivalField <= 0.0)
    {
        errorOut = "Mean Inter-arrival Time must be greater than zero.";
        return false;
    }

    if (serviceField <= 0.0)
    {
        errorOut = "Mean Service Time must be greater than zero.";
        return false;
    }

    lambdaOut = TimeUnitConverter::toRate(arrivalField);
    muOut = TimeUnitConverter::toRate(serviceField);
    return true;
}

void MainWindow::onTimeUnitChanged(int index)
{
    auto newUnit = static_cast<TimeUnitConverter::TimeUnit>(inputTimeUnit->itemData(index).toInt());

    if (newUnit == currentTimeUnit)
    {
        return;
    }

    // The arrival/service boxes need RATE-wise conversion (divide/multiply
    // by 60) if they currently hold rates, or MEAN-wise conversion
    // (multiply/divide by 60 -- the opposite direction) if they currently
    // hold mean times. Using the wrong one here is the classic mistake
    // this feature exists to avoid.
    if (currentParameterMode == ParameterMode::Rate)
    {
        inputArrivalRate->setValue(
            TimeUnitConverter::convertRate(inputArrivalRate->value(), currentTimeUnit, newUnit));
        inputServiceRate->setValue(
            TimeUnitConverter::convertRate(inputServiceRate->value(), currentTimeUnit, newUnit));
        inputArrivalRate->setSuffix(TimeUnitConverter::rateSuffix(newUnit));
        inputServiceRate->setSuffix(TimeUnitConverter::rateSuffix(newUnit));
    }
    else
    {
        inputArrivalRate->setValue(
            TimeUnitConverter::convertMean(inputArrivalRate->value(), currentTimeUnit, newUnit));
        inputServiceRate->setValue(
            TimeUnitConverter::convertMean(inputServiceRate->value(), currentTimeUnit, newUnit));
        inputArrivalRate->setSuffix(TimeUnitConverter::timeSuffix(newUnit));
        inputServiceRate->setSuffix(TimeUnitConverter::timeSuffix(newUnit));
    }

    // Variance-wise conversion (time^2 units) -- independent of whether
    // the arrival/service boxes are in Rate or Mean mode, since variance
    // always describes the spread of a TIME, not a rate.
    inputArrivalVariance->setValue(
        TimeUnitConverter::convertVariance(inputArrivalVariance->value(), currentTimeUnit, newUnit));
    inputServiceVariance->setValue(
        TimeUnitConverter::convertVariance(inputServiceVariance->value(), currentTimeUnit, newUnit));

    currentTimeUnit = newUnit;

    QString how = (currentParameterMode == ParameterMode::Rate)
                      ? "rates \xC3\xB7/\xC3\x97 60 (rate-wise)"
                      : "mean times \xC3\x97/\xC3\xB7 60 (mean-wise)";

    showStatusMessage(
        QString("Values converted to %1 -- %2, variances \xC3\x97/\xC3\xB7 3600 -- press Calculate to refresh results.")
            .arg(TimeUnitConverter::unitName(newUnit), how),
        false);
}

void MainWindow::onParameterModeChanged(int index)
{
    auto newMode = static_cast<ParameterMode>(inputParameterMode->itemData(index).toInt());

    if (newMode == currentParameterMode)
    {
        return;
    }

    // Rate and Mean Time are reciprocals of one another (mean = 1/rate),
    // so switching modes inverts whatever is currently in the boxes.
    // A value of exactly zero has no defined reciprocal, so it is left
    // at zero rather than producing infinity/NaN.
    double arrivalValue = inputArrivalRate->value();
    double serviceValue = inputServiceRate->value();

    inputArrivalRate->setValue(arrivalValue > 0.0 ? (1.0 / arrivalValue) : 0.0);
    inputServiceRate->setValue(serviceValue > 0.0 ? (1.0 / serviceValue) : 0.0);

    if (newMode == ParameterMode::Rate)
    {
        labelArrivalRate->setText("\xCE\xBB  Arrival Rate");
        labelServiceRate->setText("\xCE\xBC  Service Rate");
        inputArrivalRate->setSuffix(TimeUnitConverter::rateSuffix(currentTimeUnit));
        inputServiceRate->setSuffix(TimeUnitConverter::rateSuffix(currentTimeUnit));
    }
    else
    {
        labelArrivalRate->setText("Mean Inter-arrival Time");
        labelServiceRate->setText("Mean Service Time");
        inputArrivalRate->setSuffix(TimeUnitConverter::timeSuffix(currentTimeUnit));
        inputServiceRate->setSuffix(TimeUnitConverter::timeSuffix(currentTimeUnit));
    }

    currentParameterMode = newMode;

    showStatusMessage(
        newMode == ParameterMode::Rate
            ? "Now entering \xCE\xBB/\xCE\xBC as rates (events per unit time)."
            : "Now entering mean inter-arrival/service TIME instead of a rate "
              "(the calculator inverts it internally: rate = 1 / mean).",
        false);
}

void MainWindow::showStatusMessage(const QString &message, bool isError)
{
    statusMessageLabel->setText(message);
    statusMessageLabel->setStyleSheet(isError
        ? "background:#FDE8E8; color:#C0392B; padding:10px; border-radius:10px; font-weight:700;"
        : "background:#EFF3FF; color:#2D3436; padding:10px; border-radius:10px; font-weight:600;");
}

void MainWindow::onCalculateClicked()
{
    double lambda = 0.0;
    double mu = 0.0;
    QString resolveError;

    // Converts the two input boxes into RATES regardless of whether the
    // user is currently in "Rate" or "Mean Time" input mode (inverting
    // mean -> rate if needed). This was previously missing entirely,
    // which meant "Mean Time" mode fed mean times straight into the
    // models as if they were rates.
    if (!resolveRates(lambda, mu, resolveError))
    {
        int servers = inputServers->isVisible() ? inputServers->value() : 1;
        showStatusMessage(resolveError, true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    int servers = inputServers->isVisible() ? inputServers->value() : 1;
    double arrivalVar = inputArrivalVariance->value();
    double serviceVar = inputServiceVariance->value();

    QString arrivalLabel = (currentParameterMode == ParameterMode::Rate)
                                ? "Arrival Rate (\xCE\xBB)"
                                : "Mean Inter-arrival Time";
    QString serviceLabel = (currentParameterMode == ParameterMode::Rate)
                                ? "Service Rate (\xCE\xBC)"
                                : "Mean Service Time";

    // ---- Basic validation shared by all models ----
    if (!InputValidator::isPositive(lambda))
    {
        showStatusMessage(arrivalLabel + " must be greater than zero.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    if (!InputValidator::isPositive(mu))
    {
        showStatusMessage(serviceLabel + " must be greater than zero.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    if (!InputValidator::isReasonableValue(lambda) || !InputValidator::isReasonableValue(mu))
    {
        showStatusMessage("Please use more reasonable values (resulting rate <= 1,000,000).", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    bool needsServers = radioMMS->isChecked() || radioMGS->isChecked() || radioGGS->isChecked();
    if (needsServers && !InputValidator::isValidServerCount(servers))
    {
        showStatusMessage("Number of servers must be between 1 and 1000.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    bool needsArrivalVariance = radioGG1->isChecked() || radioGGS->isChecked();
    bool needsServiceVariance = radioMG1->isChecked() || radioMGS->isChecked()
                                 || radioGG1->isChecked() || radioGGS->isChecked();

    if (needsArrivalVariance && !InputValidator::isValidVariance(arrivalVar))
    {
        showStatusMessage("Arrival Variance cannot be negative.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    if (needsServiceVariance && !InputValidator::isValidVariance(serviceVar))
    {
        showStatusMessage("Service Variance cannot be negative.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    // ---- Stability check ----
    bool stable = needsServers
                      ? InputValidator::isMultiServerStable(lambda, mu, servers)
                      : InputValidator::isSingleServerStable(lambda, mu);

    if (!stable)
    {
        showStatusMessage("System is unstable (\xCE\xBB \xE2\x89\xA5 service capacity). "
                           "Reduce arrival rate, increase service rate or add servers.", true);
        displayResult(QueueResult(), false, servers);
        return;
    }

    // ---- Run the selected backend model ----
    QueueResult result;

    if (radioMM1->isChecked())
    {
        MM1 model(lambda, mu);
        result = model.calculate();
    }
    else if (radioMMS->isChecked())
    {
        MMS model(lambda, mu, servers);
        result = model.calculate();
    }
    else if (radioMG1->isChecked())
    {
        MG1 model(lambda, mu, serviceVar);
        result = model.calculate();
    }
    else if (radioMGS->isChecked())
    {
        MGS model(lambda, mu, servers, serviceVar);
        result = model.calculate();
    }
    else if (radioGG1->isChecked())
    {
        GG1 model(lambda, mu, arrivalVar, serviceVar);
        result = model.calculate();
    }
    else // GGS
    {
        GGS model(lambda, mu, servers, arrivalVar, serviceVar);
        result = model.calculate();
    }

    showStatusMessage("Calculation complete. System is stable.", false);
    displayResult(result, true, servers);
}

void MainWindow::onClearClicked()
{
    inputArrivalRate->setValue(0.0);
    inputServiceRate->setValue(0.0);
    inputServers->setValue(1);
    inputArrivalVariance->setValue(0.0);
    inputServiceVariance->setValue(0.0);

    showStatusMessage("Choose a model, fill in the parameters and press Calculate.", false);
    displayResult(QueueResult(), false, 1);
}

void MainWindow::displayResult(const QueueResult &result, bool systemValid, int serversUsed)
{
    if (!systemValid)
    {
        cardLq->setText("--");
        cardLs->setText("--");
        cardWq->setText("--");
        cardWs->setText("--");
        cardWqAlt->setText("");
        cardWsAlt->setText("");
        utilizationBar->setValue(0);
        utilizationValueLabel->setText("--");

        serverStatusWidget->setServers(serversUsed, 0.0, false);
        queueVizWidget->setQueueState(0, serversUsed, 0, false);
        return;
    }

    cardLq->setText(formatNumber(result.getAverageCustomersInQueue()));
    cardLs->setText(formatNumber(result.getAverageCustomersInSystem()));

    // Wq/Ws are MEAN times, so the current time unit's suffix is shown on
    // the primary value, and the small caption underneath shows the same
    // value converted to the OTHER unit via a mean-wise conversion
    // (multiply/divide by 60 -- not the rate-wise divide/multiply by 60
    // used for lambda/mu above).
    TimeUnitConverter::TimeUnit otherUnit =
        (currentTimeUnit == TimeUnitConverter::TimeUnit::Hours)
            ? TimeUnitConverter::TimeUnit::Minutes
            : TimeUnitConverter::TimeUnit::Hours;

    double wq = result.getAverageWaitingTime();
    double ws = result.getAverageTimeInSystem();
    double wqAlt = TimeUnitConverter::convertMean(wq, currentTimeUnit, otherUnit);
    double wsAlt = TimeUnitConverter::convertMean(ws, currentTimeUnit, otherUnit);

    cardWq->setText(formatNumber(wq) + TimeUnitConverter::timeSuffix(currentTimeUnit));
    cardWs->setText(formatNumber(ws) + TimeUnitConverter::timeSuffix(currentTimeUnit));
    cardWqAlt->setText(QString("= %1%2").arg(formatNumber(wqAlt), TimeUnitConverter::timeSuffix(otherUnit)));
    cardWsAlt->setText(QString("= %1%2").arg(formatNumber(wsAlt), TimeUnitConverter::timeSuffix(otherUnit)));

    double utilization = result.getUtilization();
    int barValue = qBound(0, static_cast<int>(std::round(utilization * 100.0)), 100);
    utilizationBar->setValue(barValue);
    utilizationValueLabel->setText(QString("%1%").arg(barValue));

    serverStatusWidget->setServers(serversUsed, utilization, true);

    int waitingCustomers = static_cast<int>(std::round(result.getAverageCustomersInQueue()));
    int busyServers = qBound(0, static_cast<int>(std::round(utilization * serversUsed)), serversUsed);
    if (utilization > 0 && busyServers == 0)
    {
        busyServers = 1;
    }

    queueVizWidget->setQueueState(waitingCustomers, serversUsed, busyServers, true);
}
