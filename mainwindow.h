#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QPainter>
#include <QMainWindow>
#include "smal_lresource.h"
#include "Parce_WoT_Blitz_txt_mode.h"
#include "minimap.h"

const std::string 
              RESOURCE_CONFIG = "./Resource/data.cfg", 
              RESOURCE_DATA   = "./Resource/data.bin",
              PNG_TEX         = "PNG" ,
              WEBP_TEX        = "WEBP";

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    
    void resizeEvent(QResizeEvent*)    override;
    void keyPressEvent(QKeyEvent*)     override;
    void mouseMoveEvent(QMouseEvent*)  override;
    void mousePressEvent(QMouseEvent*) override;
    
private slots:
    void rbClk();
    void copyTexPNG();
    void copyTexWEBP();
    void menuOpenClk();
    void menuSaveClk();
    void menuSaveAsClk();
    void changeLayer(int currentRow);
    
    void tileMoved(QPoint new_pos);
    
    void changeValueHorizont(int val);
    void changeValueVertical(int val);
    
private:
    void CopyTex(bool png = true);

    int sizeHeadline;
    
    void Init();            // Инициализация конструктора
    void ReDraw();          // Обновление тайла, его перерисовка
    void UpdateCenter();    // Обновление данных в _dm
    void UpdateMinimap();   // Обновление состояния миникарты
    void UpdateLayerList(); // Обновление списка слоев
                              
    void MooveViewTile(QMouseEvent *event);   // перемещение тайла
    void MooveBorderTile(QMouseEvent *event); // перемещение рамки тайла на миникарте
    
    void MinimapMaximize(); // максимизация миникарты
    void MinimapMinimize(); // минимизация миникарты
    
    void UpdateNumerics(); // обновление данных в numeric элементах
    
    
    
    void ResetFrom();
    
    SmallResource _resource; // ресурсы
    
    const QRect& size;
    
    QPoint mpsm, // mouse pos start move
           tpsm; // tile pos start move
    
    QPixmap _atlas;  // атлас
    DataManager _dm; // данные
    
    Minimap* _minimap;
    
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
