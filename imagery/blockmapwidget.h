#ifndef BLOCKMAPWIDGET_H
#define BLOCKMAPWIDGET_H

#include <QtGui/QtGui>
#include "dimage.h"

#include <QWidget>

class BlockMapWidget : public QWidget
{
  Q_OBJECT
  
public:
  BlockMapWidget(QWidget *parent=0);
  ~BlockMapWidget();
  
  void setBlockMap(const BlockMap &blockMap);
  QColor blockColor(int track,int sector);

  void setClearColor(const QColor &color);
  QColor clearColor() const;
  void setDefaultBlockSize(const QSize &size);
  QSize defaultBlockSize() const;
  
  QSize sizeHint() const;
  QSize minimumSizeHint() const;

public slots:
  void clear();
  void setBlockColor(int track,int sector,const QColor &color);

protected:
  BlockMap  m_blockMap;
  OffsetMap m_offsetMap;
  QVector<QColor> m_colorMap;
  int m_maxSectors;
  int m_numTracks;
  int m_numBlocks;
  QColor m_clearColor;
  QSize m_defaultBlockSize;
  QSize m_blockSize;
  
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  QRect blockRect(int track,int sector);
};

#endif
