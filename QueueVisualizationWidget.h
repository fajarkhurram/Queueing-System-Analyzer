#ifndef QUEUEVISUALIZATIONWIDGET_H
#define QUEUEVISUALIZATIONWIDGET_H

#include <QWidget>

// Custom widget that draws a simple animation-free picture of the
// queueing system: customers waiting in line -> servers processing them.
class QueueVisualizationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QueueVisualizationWidget(QWidget *parent = nullptr);

    // waitingCount   -> how many customers are drawn waiting in the queue (Lq, rounded)
    // servers        -> number of server stations to draw
    // busyServers    -> how many of those servers are currently drawn as "serving"
    // valid          -> false draws an empty / error state
    void setQueueState(int waitingCount, int servers, int busyServers, bool valid);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_waitingCount;
    int m_servers;
    int m_busyServers;
    bool m_valid;
};

#endif // QUEUEVISUALIZATIONWIDGET_H
