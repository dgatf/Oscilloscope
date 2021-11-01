#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SettingsDialog;
class AboutDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent* event);

    uint16_t interval; // us

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serial = nullptr;
    QPixmap *m_pixmap;
    QPainter *m_paint;
    SettingsDialog *m_ui_settings = nullptr;
    AboutDialog *m_ui_about = nullptr;
    QLabel *m_statusLabel = nullptr;

    float vDiv = 1;
    float volt = 1 * 6;
    uint16_t timeLenght = 1 * 10;
    float trigger = 1;
    enum Edge { min, max };
    enum TriggerTypes { none, rising, falling };
    uint8_t triggerType = none;

    uint8_t maxValue = 0;
    uint8_t minValue = 0;
    uint16_t rawFreq;
    uint16_t rawDuty;
    bool pendingExport = false;
    QString fileName;

    void drawBackground();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void updateTrigger();
    void updateTriggerType();
    void exitApp();
    void updateVdiv();
    void updateHdiv();
    void settings();
    void about();
    void exportData();
};

#endif // MAINWINDOW_H
