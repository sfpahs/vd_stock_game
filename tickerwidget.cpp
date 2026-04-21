#include "tickerwidget.h"
#include <QPainter>
#include <QFontMetrics>

TickerWidget::TickerWidget(QWidget* parent)
    : QWidget(parent), m_offset(0)
{
    setMinimumWidth(200);
    setFixedHeight(30);
    setAttribute(Qt::WA_OpaquePaintEvent);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TickerWidget::tick);
    QTimer::singleShot(100, this, [this]() { m_timer->start(40); });
}

void TickerWidget::setText(const QString& text) {
    m_text = text + "          ";
    m_offset = 0;
    update();
}

void TickerWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), palette().window());
    p.setClipRect(rect());
    QFontMetrics fm(font());
    int y = (height() + fm.ascent() - fm.descent()) / 2;
    p.drawText(-m_offset,                                y, m_text);
    p.drawText(-m_offset + fm.horizontalAdvance(m_text), y, m_text);
}

void TickerWidget::tick() {
    if (m_text.isEmpty()) return;
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(m_text);
    if (textWidth <= 0) return;
    m_offset = (m_offset + 1) % textWidth;
    update();
}
