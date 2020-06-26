#include "minimap.h"

#include <QResizeEvent>
#include <QMouseEvent>
#include <cmath>

Minimap::Minimap(QWidget *parent) : QLabel(parent)
{
    Init();
}

void Minimap::Init() {
    this->resize(1,1);
    this->_frame.setParent(this);
    this->_frame.setFrameStyle(QFrame::Box);
    this->_frame.resize(1, 1);
}

void Minimap::setPixmap(const QPixmap &pixmap) {
    this->_background = pixmap;
    QLabel::setPixmap(pixmap);
    
    this->resize(_background.size());
}

template<class In>
constexpr float scastf(const In& in) { return static_cast<float>(in); }
template<class In>
constexpr float scasti(const In& in) { return static_cast<int>(in); }


void Minimap::resizeEvent(QResizeEvent *ev) {

    // получили исходный размер атласа
    int aw = _background.size().width();
    int ah = _background.size().height();
    
    if(!aw) aw = 1;
    if(!ah) ah = 1; 
    
    // коэфициенты масштабирования
    float kw = scastf(ev->oldSize().width())  / scastf(ev->size().width());   
    float kh = scastf(ev->oldSize().height()) / scastf(ev->size().height());    
    
    QLabel::resize (
        ev->size().width(),
        ah * ev->size().width() / aw
    );
    
    auto tPos = _frame.pos(); // tx ty
    auto tSiz = _frame.size(); // tw th
    
    /*
     * kw = mW / aw
     * kh = mH / ah
     * 
     * x1 = tx * kw; w1 = tw * kw;
     * y1 = ty * kh; h1 = th * kh;
     */
     
   // float kw = (float)size.width()  / (float)aw; // kw
   // float kh = (float)size.height() / (float)ah; // kh
    
    tPos.setX     (std::floor(tPos.x()      * kw)); 
    tPos.setY     (std::floor(tPos.y()      * kh)); 
    tSiz.setWidth (std::ceil (tSiz.width()  * kw));
    tSiz.setHeight(std::ceil (tSiz.height() * kh));
    
    this->setPixmap(_background);
    _frame.move(tPos);
    _frame.resize(tSiz);
}

void Minimap::mouseMoveEvent(QMouseEvent *ev) {
    auto delta = ev->pos() - _start_move;
    
    auto new_pos = _pos_frame + delta;
    _frame.move(new_pos);
    
    FrameMoved(_frame.pos());
}

void Minimap::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::MouseButton::LeftButton)  {
        _start_move = ev->pos(); 
        _pos_frame = _frame.pos();
    }
}

void Minimap::MoveFrame(QPoint offset) {
    auto new_pos = _frame.pos() + offset;
    _frame.move(new_pos);
}

void Minimap::MoveFrame(int ox, int oy) {
    MoveFrame( {ox, oy} );
}

void Minimap::SetFramePos(QPoint pos) {
    if(_background.isNull()) return;
    
    float kw = this->size().width()  / _background.size().width();
    float kh = this->size().height() / _background.size().height();
    
    pos.setX (std::floor(pos.x() * kw)); 
    pos.setY (std::floor(pos.y() * kh)); 
    
    auto size = _frame.size();
    _frame.setGeometry( {pos, size} );
}
void Minimap::SetFrameSize(QSize size) {
    if(_background.isNull()) return;
    
    float kw = this->size().width()  / _background.size().width();
    float kh = this->size().height() / _background.size().height();
    
    size.setWidth (std::ceil (size.width()  * kw));
    size.setHeight(std::ceil (size.height() * kh));
    
    auto pos = _frame.pos();
    _frame.setGeometry( {pos, size} );
}

void Minimap::SetFrameGeom(QRect geometry) {
    if(_background.isNull()) return;
    
    float kw = this->size().width() / _background.size().width();
    float kh = this->size().height() / _background.size().height();
    
    geometry.setX     (std::floor(geometry.x()      * kw)); 
    geometry.setY     (std::floor(geometry.y()      * kh)); 
    geometry.setWidth (std::ceil (geometry.width()  * kw));
    geometry.setHeight(std::ceil (geometry.height() * kh));

    _frame.setGeometry(geometry);
}