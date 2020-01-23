#include "blockmapwidget.h"

#include <QPainter>
#include <QWidget>

BlockMapWidget::BlockMapWidget(QWidget *parent)
    : QWidget(parent), m_maxSectors(0), m_numTracks(0), m_numBlocks(0),
      m_clearColor(Qt::black), m_defaultBlockSize(8, 8) {
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
}

BlockMapWidget::~BlockMapWidget() {}

void BlockMapWidget::setClearColor(const QColor &color) {
    m_clearColor = color;
    clear();
}

QColor BlockMapWidget::clearColor() const { return m_clearColor; }

void BlockMapWidget::setDefaultBlockSize(const QSize &blockSize) {
    m_defaultBlockSize = blockSize;
    update();
    updateGeometry();
}

QSize BlockMapWidget::defaultBlockSize() const { return m_defaultBlockSize; }

void BlockMapWidget::setBlockMap(const BlockMap &blockMap) {
    m_blockMap = blockMap;

    m_numTracks = m_blockMap.size();
    m_maxSectors = 0;
    m_offsetMap.resize(m_numTracks);
    int sum = 0;
    for (int t = 0; t < m_numTracks; t++) {
        int sectors = m_blockMap[t];
        if (sectors > m_maxSectors)
            m_maxSectors = sectors;
        m_offsetMap[t] = sum;
        sum += sectors;
    }
    m_numBlocks = sum;
    m_colorMap.resize(m_numBlocks);

    clear();
    update();
    updateGeometry();
}

QColor BlockMapWidget::blockColor(int track, int sector) {
    return m_colorMap[m_offsetMap[track] + sector];
}

void BlockMapWidget::clear() {
    for (int b = 0; b < m_numBlocks; b++) {
        m_colorMap[b] = m_clearColor;
    }
    update();
}

void BlockMapWidget::setBlockColor(int track, int sector, const QColor &color) {
    m_colorMap[m_offsetMap[track] + sector] = color;
    update(blockRect(track, sector));
}

void BlockMapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    int b = 0;
    for (int t = 0; t < m_numTracks; t++) {
        int numSectors = m_blockMap[t];
        for (int s = 0; s < numSectors; s++) {
            QRect rect = blockRect(t, s);
            if (!event->region().intersected(rect).isEmpty()) {
                const QColor &color = m_colorMap[b];
                painter.fillRect(rect, color);
            }
            b++;
        }
    }
}

void BlockMapWidget::resizeEvent(QResizeEvent *event) {
    QSize size = event->size();
    // update block size
    int bw, bh;
    if (m_numTracks == 0)
        bw = size.width();
    else
        bw = (size.width() - 1) / m_numTracks;
    if (m_maxSectors == 0)
        bh = size.height();
    else
        bh = (size.height() - 1) / m_maxSectors;
    if (bw < 2)
        bw = 2;
    if (bh < 2)
        bh = 2;
    m_blockSize = QSize(bw, bh);
}

QRect BlockMapWidget::blockRect(int track, int sector) {
    int bw = m_blockSize.width();
    int bh = m_blockSize.height();
    return QRect(1 + track * bw, 1 + sector * bh, bw - 1, bh - 1);
}

QSize BlockMapWidget::sizeHint() const {
    int w = m_numTracks * m_defaultBlockSize.width() + 1;
    int h = m_maxSectors * m_defaultBlockSize.height() + 1;
    return QSize(w, h);
}

QSize BlockMapWidget::minimumSizeHint() const {
    int w = 2 * m_numTracks + 1;
    int h = 2 * m_maxSectors + 1;
    return QSize(w, h);
}
