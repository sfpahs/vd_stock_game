#pragma once
#include <QWidget>
#include <QTimer>

class TickerWidget : public QWidget {
    Q_OBJECT
public:
    explicit TickerWidget(QWidget* parent = nullptr);
    void setText(const QString& text);
protected:
    void paintEvent(QPaintEvent* event) override;
private slots:
    void tick();
private:
    QString m_text;
    int     m_offset;
    QTimer* m_timer;
};
