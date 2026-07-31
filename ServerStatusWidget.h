#ifndef SERVERSTATUSWIDGET_H
#define SERVERSTATUSWIDGET_H

#include <QWidget>
#include <QVector>

// Custom widget that draws a small "server rack" -
// one colored circle per server, green/amber/red depending
// on how busy the system currently is.
class ServerStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerStatusWidget(QWidget *parent = nullptr);

    // Sets number of servers to draw and current utilization (0..1+)
    void setServers(int serverCount, double utilization, bool systemValid);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_serverCount;
    double m_utilization;
    bool m_valid;
};

#endif // SERVERSTATUSWIDGET_H
