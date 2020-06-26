#ifndef MINIMAP_H
#define MINIMAP_H

#include <QWidget>
#include <QLabel>

class Minimap final : public QLabel
{
    Q_OBJECT
public:
    explicit Minimap(QWidget *parent = nullptr);
    
    void MoveFrame(QPoint offset);
    // перемещение на ox oy
    void MoveFrame(int ox, int oy);
    
    void SetFramePos(QPoint pos);
    void SetFrameSize(QSize size);
    void SetFrameGeom(QRect geometry);
    
    void resizeEvent(QResizeEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    
    void setPixmap(const QPixmap& pixmap);
signals:
    void FrameMoved(QPoint new_pos);
    void FrameSized(QSize new_size);
    
public slots:

private:
    void Init();

    QPoint _start_move, _pos_frame;

    QPixmap _background;
    QLabel  _frame;    
};

#endif // MINIMAP_H
