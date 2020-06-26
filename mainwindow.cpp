#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <cmath>

#include <QLine>
#include <QBitmap>
#include <QPainter>
#include <QSpinBox>
#include <QSaveFile>
#include <QFileDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPaintEngine>
#include <QKeySequence>
#include <QLayout>
#include <QResizeEvent>

template<class To, class In> inline constexpr
To scast(In val) { return static_cast<To>(val); }

template<class To, class In> inline constexpr
To ceil_cast(In val) { return static_cast<To>(std::ceil(val)); }

template<class To, class In> inline constexpr
To floor_cast(In val) { return static_cast<To>(std::floor(val)); }


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , size(geometry())
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _minimap = new Minimap(ui->centralwidget);
    _minimap->show();
    ui->gb_ChangesModify->raise();
    ui->listWidget->raise();
    
    connect(ui->menu_texPNG,  &QAction::triggered, this, &MainWindow::copyTexPNG);
    connect(ui->menu_texWEBP, &QAction::triggered, this, &MainWindow::copyTexWEBP);
    
    connect(ui->menu_open,    &QAction::triggered,             this, &MainWindow::menuOpenClk);
    connect(ui->menu_save,    &QAction::triggered,             this, &MainWindow::menuSaveClk);
    connect(ui->menu_save_as, &QAction::triggered,             this, &MainWindow::menuSaveAsClk);
    connect(ui->listWidget,   &QListWidget::currentRowChanged, this, &MainWindow::changeLayer);

    connect(ui->rb_setSize,     &QRadioButton::released, this, &MainWindow::rbClk);
    connect(ui->rb_setCenter,   &QRadioButton::released, this, &MainWindow::rbClk);
    connect(ui->rb_setPosition, &QRadioButton::released, this, &MainWindow::rbClk);
    
    connect(ui->numHorizont, SIGNAL(valueChanged(int)), this, SLOT(changeValueHorizont(int)));
    connect(ui->numVertical, SIGNAL(valueChanged(int)), this, SLOT(changeValueVertical(int)));
    
    connect(_minimap, &Minimap::FrameMoved, this, &MainWindow::tileMoved);
    
    //connect(ui->numHorizont, &QSpinBox::valueChanged, this, &MainWindow::changeValueHorizont);
    
    Init();
}

MainWindow::~MainWindow() {
    delete _minimap;
    delete ui;
}

/***************************************************************/
/****   Переобпределённые события окна   ***********************/
/***************************************************************/

void MainWindow::resizeEvent(QResizeEvent *ev) {
    QMainWindow::resizeEvent(ev);
    ResetFrom();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    int 
        a = 0, 
        b = 0;
    auto CH = event->text().toLower();
    if(CH == "w" || CH == "ц") { b = -1; } else
    if(CH == "s" || CH == "ы") { b =  1; } else
    if(CH == "a" || CH == "ф") { a = -1; } else
    if(CH == "d" || CH == "в") { a =  1; }
    
    if(ui->rb_setPosition->isChecked()) _dm.AddPosition (a, b); else 
    if(ui->rb_setSize->isChecked()    ) _dm.AddSize     (a, b); else 
    if(ui->rb_setCenter->isChecked()  ) _dm.MoveTile    (a, b);
    
    UpdateLayerList();
    ReDraw();
    
    QMainWindow::keyPressEvent(event);
}

void MainWindow::MooveViewTile(QMouseEvent *ev) {
    /**
      цХ = поз_курсор.х - поз_фон.х - тайл_ширина / 2
      цY = поз_курсор.у - поз_фон.у - тайл_высота / 2 - заголовок_окна_высота
    */
    if( !ui->background->geometry().contains(ev->pos()) ) return;
    
    auto delta = ev->pos() - mpsm;
    
    ui->showTile->move(
        //ev->windowPos().x() - size.x() - size.width()  / 2,              
        //ev->windowPos().y() - size.y() - size.height() / 2 - sizeHeadline
        tpsm + delta
    );
}

void MainWindow::MooveBorderTile(QMouseEvent *ev) {
    //if(!ui->rb_setPosition->isChecked()) return; // если активно перемещение
    //
    //auto& size = ui->miniMap->geometry();
    //auto& frame_size = ui->lblFrame->geometry(); // рамка тайла
    //
    //if(!frame_size.contains(ev->pos())) return; // если мышь не над рамкой
    //
    //auto aw = _atlas.size().width();
    //auto ah = _atlas.size().height();
    //
    //if(!aw || !ah) return; // если один из размеров 0
    //
    //float kw = scast<float>(size.width())  / scast<float>(aw); // kw
    //float kh = scast<float>(size.height()) / scast<float>(ah); // kh
    
    /**
      цХ = поз_курсор.х - поз_миникарта.х - тайл_ширина / 2
      цY = поз_курсор.у - поз_миникарта.у - тайл_высота / 2 - заголовок_окна_высота
    */
    
    //ui->showTile->move(
    //    ev->x() - size.x() - frame_size.width()  / 2, 
    //    ev->y() - size.y() - frame_size.height() / 2 - sizeHeadline
    //);
    //
    //aw = ceil_cast<int>(frame_size.x() / kw);
    //ah = ceil_cast<int>(frame_size.y() / kh);
    //
    //_dm.Position(aw, ah);
}

void MainWindow::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::MouseButton::LeftButton) {
        mpsm = ev->pos();
        tpsm = ui->showTile->pos();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *ev) {
    MooveViewTile(ev);
    //MooveBorderTile(ev);

    UpdateCenter();    // обновления _dm относительно положения тайла
    UpdateLayerList(); // обновление списка слоев, относительно новых в _dm
    
//    QMainWindow::mouseMoveEvent(event);
}

/***************************************************************/
/****   Вспомогательные методы   *******************************/
/***************************************************************/

void MainWindow::Init() {
    QPixmap a("./Resource/Cross.png");
    
    ui->background->move(10, 10);
    ui->background->setPixmap(a);                 // устновка фона для центрирования
    ui->showTile->setParent(ui->background);      // для расчета положения тайла
    
    sizeHeadline = this->sizeHint().height() / 2; // высота заголовка
    
    _resource.Load(RESOURCE_CONFIG, RESOURCE_DATA);   // загрузка байтов ресурсов
    
    ui->rb_setCenter->setChecked(true);
    
    ResetFrom();
}

///
QSize TileSize(const DataManager& dm) { return { dm.Size().first, dm.Size().second }; }
QPoint TilePos(const DataManager& dm) { return { dm.Position().first, dm.Position().second }; }

/// Отрисовка тайла по координатам в _dm 
void MainWindow::ReDraw() {
    _atlas.detach();
    _atlas.load(_dm.CurrentAtlas().filePath());
     
    auto& [x, y]  = _dm.Position();         
    auto& [w, h]  = _dm.Size();             
    auto [ox, oy] = _dm.Center();           
    auto size     = ui->background->size(); 
    
    ox = ( size.width()  - w ) / 2 + ox;
    oy = ( size.height() - h ) / 2 + oy;
     
    ui->showTile->setGeometry(ox, oy, w, h);
    ui->showTile->setPixmap(_atlas.copy(x, y, w, h));
 
    UpdateMinimap();
}

void MainWindow::UpdateCenter() {
    /** установка значений соответствующих новому центру */
    int 
        x  = ui->background->size().width(), 
        y  = ui->background->size().height(),
        px = ui->showTile->pos().x(),
        py = ui->showTile->pos().y(),
        w  = ui->showTile->size().width(),
        h  = ui->showTile->size().height();
    
    x = -(x / 2 - (px + w / 2));
    y = -(y / 2 - (py + h / 2));
    
    _dm.Center(x, y);
}

void MainWindow::UpdateMinimap() {
    _minimap->setPixmap(_atlas);
    _minimap->SetFramePos( TilePos(_dm) );
    _minimap->SetFrameSize( TileSize(_dm) );
   //
   //// получаем исходный размер атласа
   // int aw = _atlas.size().width(); 
   // int ah = _atlas.size().height();
   // 
   // // проверка размера
   // if(!aw) aw = 1;
   // if(!ah) ah = 1; 
   // 
   // // получаем текущий размер миникарты
   // auto& size = ui->miniMap->geometry();
   // 
   // // изменяем размер пропорционально
   // ui->miniMap->resize (
   //     size.width(),
   //     (ah * size.width()) / aw
   // );
   // 
   // auto [tx, ty] = _dm.Position();
   // auto [tw, th] = _dm.Size();
   // 
   // /*
   //  * kw = mW / aw
   //  * kh = mH / ah
   //  * 
   //  * x1 = tx * kw; w1 = tw * kw;
   //  * y1 = ty * kh; h1 = th * kh;
   //  */
   // 
   // float kw = (float)size.width()  / (float)aw; // kw
   // float kh = (float)size.height() / (float)ah; // kh
   // tx = std::floor(tx * kw); 
   // tw = std::ceil(tw * kw);
   // ty = std::floor(ty * kh); 
   // th = std::ceil(th * kh);
   // 
   // ui->miniMap->setPixmap(_atlas);
   // ui->lblFrame->setGeometry(tx, ty, tw, th);
}

void MainWindow::UpdateLayerList() {
    /** обновление списка слоев */
    ui->listWidget->clear();
    for(auto& i : _dm.AllLayers()) {
        ui->listWidget->addItem(i.data());
    }
}

void MainWindow::MinimapMaximize() {
    auto wSize = ui->centralwidget->size();
    _minimap->resize(wSize * 0.9);
    ResetFrom();
}

void MainWindow::MinimapMinimize() {
    auto wSize = ui->centralwidget->size();
    _minimap->resize(wSize * 0.2);
    ResetFrom();
}

void MainWindow::ResetFrom() {
    const int 
        BOTTOM_FREE = 50,
        RIGHT_FREE = 10;
    
    //ev->size().width()   --> size.width
    //ev->size().height()  --> size.height
    
    // background pos
    ui->background->resize(
        size.width()  - RIGHT_FREE, 
        size.height() - BOTTOM_FREE
    );
    
    /// listWidget pos
    auto lSize = ui->listWidget->size();
    ui->listWidget->move(
        size.width()  - lSize.width()  - RIGHT_FREE, 
        size.height() - lSize.height() - BOTTOM_FREE
    );
    
    /// radiobuttons pos
    auto gbSize = ui->gb_ChangesModify->size();
    ui->gb_ChangesModify->move(
        size.width()  - gbSize.width()  - RIGHT_FREE, 
        size.height() - gbSize.height() - lSize.height() - 10 - BOTTOM_FREE
    );
    
    /// minimap pos
    //auto mmSize = ui->miniMap->geometry();
    //ui->miniMap->move( 
    //    size.width()  - mmSize.width()  - RIGHT_FREE, 
    //    20 
    //);
    auto mmSize = _minimap->geometry();
    _minimap->move( 
        size.width()  - mmSize.width()  - RIGHT_FREE, 
        20 
    );
    
    ReDraw();
}
 
void MainWindow::UpdateNumerics() {
    Point p;
    
    if(ui->rb_setSize->isChecked()) p = _dm.Size(); else 
    if(ui->rb_setCenter->isChecked()) p = _dm.Center(); else 
    if(ui->rb_setPosition->isChecked())p = _dm.Position();
    
    ui->numHorizont->setValue(p.first);
    ui->numVertical->setValue(p.second);
}

/***************************************************************/
/****   Обработка слотов   *************************************/
/***************************************************************/

/// Clicked radio buttons
void MainWindow::rbClk() {

    if(ui->rb_setSize->isChecked()) {         
        MinimapMaximize();
        ui->showTile->setFrameStyle(QFrame::Box);      
    } else 
    if(ui->rb_setCenter->isChecked()) {
        MinimapMinimize();
        ui->showTile->setFrameStyle(QFrame::NoFrame);
    } else 
    if(ui->rb_setPosition->isChecked()) {            
        MinimapMaximize();
        ui->showTile->setFrameStyle(QFrame::Box);
    }   
    
    ReDraw();
    UpdateNumerics();
}

/// Menu open
void MainWindow::menuOpenClk() {
    auto File = QFileDialog::getOpenFileName (
        this,
        "Выбор текстового файла",
        QDir::homePath(),
        "Text (*.txt)"
    );
    
    _dm.Load(File);
    UpdateLayerList();
    
    _atlas.load(_dm.CurrentAtlas().filePath());
    ReDraw();
}

/// Menu save
void MainWindow::menuSaveClk() {
    _dm.SaveTo();
}

/// Menu save as
void MainWindow::menuSaveAsClk() {
    auto savePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить",
        QDir::homePath(),
        "Text file (*.txt)"
    );
    _dm.CurrentFile(savePath);
    menuSaveClk();
}

/// change elemt from listWidget
void MainWindow::changeLayer(int currentRow) {
   _dm.ChangeLayer(static_cast<size_t>(currentRow)); 
   ui->listWidget->clearFocus();
   ReDraw();
}

/// helpers function
void MainWindow::CopyTex(bool png) {
    auto savePath = QFileDialog::getSaveFileName(
        nullptr,
        "Создать",
        QDir::homePath(),
        "Tex files (*.tex)"
    );
    std::ofstream out(savePath.toLocal8Bit(), std::ios_base::binary);
    auto bytes = (png) ? _resource.GetResource(PNG_TEX) : _resource.GetResource(WEBP_TEX);
    out.write(bytes.data(), static_cast<long long>(bytes.size()));
    out.close();
}

/// menu TEX
void MainWindow::copyTexPNG()  { CopyTex(); }
/// menu WEBP
void MainWindow::copyTexWEBP() { CopyTex(false); }


void MainWindow::tileMoved(QPoint new_pos) {
    _dm.Position(new_pos.x(), new_pos.y());
    UpdateLayerList();
    ReDraw();
}

void MainWindow::changeValueHorizont(int x) {
    int y;
    
    if(ui->rb_setSize->isChecked()) {         
        y = _dm.Size().second;     
        _dm.Size(x, y);
    } else 
        if(ui->rb_setCenter->isChecked()) {
            y = _dm.Center().second;
            _dm.Center(x, y);   
    } else 
        if(ui->rb_setPosition->isChecked()) {            
            y = _dm.Position().second; 
            _dm.Position(x, y);
    }
}

void MainWindow::changeValueVertical(int y) {
    int x;
    
    if(ui->rb_setSize->isChecked()) {         
        x = _dm.Size().first;     
        _dm.Size(x, y);
    } else 
        if(ui->rb_setCenter->isChecked()) {
            x = _dm.Center().first;
            _dm.Center(x, y);   
    } else 
        if(ui->rb_setPosition->isChecked()) {            
            x = _dm.Position().first; 
            _dm.Position(x, y);
    }
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
