#include "ServerStatusWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <cmath>

ServerStatusWidget::ServerStatusWidget(QWidget *parent)
    : QWidget(parent)
    , m_serverCount(1)
    , m_utilization(0.0)
    , m_valid(false)
{
    setMinimumHeight(90);
}

void ServerStatusWidget::setServers(int serverCount, double utilization, bool systemValid)
{
    m_serverCount = serverCount > 0 ? serverCount : 1;
    m_utilization = utilization;
    m_valid = systemValid;
    update();
}

QSize ServerStatusWidget::sizeHint() const
{
    return QSize(300, 90);
}

void ServerStatusWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int circleSize = 26;
    const int spacingX = 10;
    const int spacingY = 34;
    const int marginLeft = 12;
    const int marginTop = 12;

    int perRow = qMax(1, (width() - marginLeft) / (circleSize + spacingX));
    // Cap how many circles we actually draw so huge server counts stay readable
    int drawCount = qMin(m_serverCount, 40);

    // How many servers should be shown "busy" (colored) based on utilization
    int busyCount = static_cast<int>(std::round(m_utilization * m_serverCount));
    busyCount = qBound(0, busyCount, drawCount);

    QFont labelFont = painter.font();
    labelFont.setPointSize(9);
    painter.setFont(labelFont);

    for (int i = 0; i < drawCount; ++i)
    {
        int row = i / perRow;
        int col = i % perRow;

        int x = marginLeft + col * (circleSize + spacingX);
        int y = marginTop + row * spacingY;

        QColor fillColor;
        QString icon;

        if (!m_valid)
        {
            fillColor = QColor("#E74C3C"); // red - invalid / unstable system
            icon = QStringLiteral("\xF0\x9F\x94\xB4");
        }
        else if (i < busyCount)
        {
            fillColor = QColor("#2ECC71"); // green - busy / working server
            icon = QStringLiteral("\xF0\x9F\x9F\xA2");
        }
        else
        {
            fillColor = QColor("#F1C40F"); // amber - idle server
            icon = QStringLiteral("\xF0\x9F\x9F\xA1");
        }

        painter.setBrush(fillColor);
        painter.setPen(QPen(fillColor.darker(130), 2));
        painter.drawEllipse(x, y, circleSize, circleSize);

        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setBold(true);
        f.setPointSize(10);
        painter.setFont(f);
        painter.drawText(QRect(x, y, circleSize, circleSize), Qt::AlignCenter, QString::number(i + 1));
    }

    if (m_serverCount > drawCount)
    {
        int row = drawCount / perRow;
        int y = marginTop + row * spacingY + circleSize + 4;
        painter.setPen(QColor("#7F8C8D"));
        QFont f = painter.font();
        f.setBold(false);
        f.setPointSize(9);
        painter.setFont(f);
        painter.drawText(QRect(marginLeft, y, width() - marginLeft, 20),
                          Qt::AlignLeft,
                          QString("+ %1 more servers").arg(m_serverCount - drawCount));
    }
}
