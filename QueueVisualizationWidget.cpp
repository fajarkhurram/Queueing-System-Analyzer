#include "QueueVisualizationWidget.h"
#include <QPainter>
#include <QLinearGradient>

QueueVisualizationWidget::QueueVisualizationWidget(QWidget *parent)
    : QWidget(parent)
    , m_waitingCount(0)
    , m_servers(1)
    , m_busyServers(0)
    , m_valid(false)
{
    setMinimumHeight(150);
}

void QueueVisualizationWidget::setQueueState(int waitingCount, int servers, int busyServers, bool valid)
{
    m_waitingCount = qMax(0, waitingCount);
    m_servers = qMax(1, servers);
    m_busyServers = qBound(0, busyServers, m_servers);
    m_valid = valid;
    update();
}

QSize QueueVisualizationWidget::sizeHint() const
{
    return QSize(600, 160);
}

void QueueVisualizationWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor("#F4F6FB"));
    bg.setColorAt(1, QColor("#E9ECF7"));
    painter.fillRect(rect(), bg);

    if (!m_valid)
    {
        painter.setPen(QColor("#E74C3C"));
        QFont f = painter.font();
        f.setPointSize(11);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter,
                          QStringLiteral("Enter valid, stable parameters to see the queue"));
        return;
    }

    const int laneY = height() / 2;
    const int personRadius = 14;
    const int maxDrawnWaiting = qMin(m_waitingCount, 25);
    const int serverBoxW = 46;
    const int serverBoxH = 60;
    const int rightMargin = 20;

    int serverAreaX = width() - rightMargin - m_servers * (serverBoxW + 12);
    serverAreaX = qMax(serverAreaX, width() / 2);

    // --- Draw the queue line (waiting customers) ---
    int spacing = 30;
    int startX = 24;

    for (int i = 0; i < maxDrawnWaiting; ++i)
    {
        int x = startX + i * spacing;
        if (x > serverAreaX - 30) break;

        QColor bodyColor = QColor("#6C5CE7");
        painter.setBrush(bodyColor);
        painter.setPen(Qt::NoPen);

        // head
        painter.drawEllipse(QPoint(x, laneY - 10), personRadius / 2 + 2, personRadius / 2 + 2);
        // body
        painter.drawRoundedRect(x - 9, laneY, 18, 22, 6, 6);
    }

    if (m_waitingCount > maxDrawnWaiting)
    {
        painter.setPen(QColor("#636E72"));
        QFont f = painter.font();
        f.setPointSize(9);
        painter.setFont(f);
        painter.drawText(QRect(startX, laneY + 26, 200, 20),
                          QString("+%1 more waiting").arg(m_waitingCount - maxDrawnWaiting));
    }

    // Queue label
    painter.setPen(QColor("#2D3436"));
    QFont labelFont = painter.font();
    labelFont.setPointSize(9);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.drawText(QRect(startX, laneY - 45, 220, 20),
                      QString("Waiting: %1").arg(m_waitingCount));

    // --- Draw arrow towards servers ---
    QPen arrowPen(QColor("#B2BEC3"), 2, Qt::DashLine);
    painter.setPen(arrowPen);
    painter.drawLine(startX, laneY + 40, serverAreaX - 10, laneY + 40);

    // --- Draw servers ---
    for (int s = 0; s < m_servers; ++s)
    {
        int x = serverAreaX + s * (serverBoxW + 12);
        int y = laneY - serverBoxH / 2;

        bool busy = s < m_busyServers;
        QColor boxColor = busy ? QColor("#00B894") : QColor("#DFE6E9");

        painter.setPen(QPen(boxColor.darker(140), 2));
        painter.setBrush(boxColor);
        painter.drawRoundedRect(x, y, serverBoxW, serverBoxH, 10, 10);

        painter.setPen(Qt::white);
        QFont sf = painter.font();
        sf.setBold(true);
        sf.setPointSize(9);
        painter.setFont(sf);
        painter.drawText(QRect(x, y, serverBoxW, 18), Qt::AlignCenter, QString("S%1").arg(s + 1));

        if (busy)
        {
            // little person icon to represent "being served"
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(x + serverBoxW / 2, y + 34), 6, 6);
            painter.drawRoundedRect(x + serverBoxW / 2 - 8, y + 42, 16, 14, 4, 4);
        }
    }

    painter.setPen(QColor("#2D3436"));
    painter.setFont(labelFont);
    painter.drawText(QRect(serverAreaX, laneY - serverBoxH / 2 - 24, m_servers * (serverBoxW + 12), 20),
                      QString("Servers (%1 busy / %2)").arg(m_busyServers).arg(m_servers));
}
