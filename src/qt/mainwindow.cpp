#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "aboutdialog.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_serial(new QSerialPort(this)),
      m_pixmap(new QPixmap(500, 400)),
      m_paint(new QPainter(m_pixmap)),
      m_ui_settings(new SettingsDialog()),
      m_ui_about(new AboutDialog()),
      m_statusLabel(new QLabel())
{
    ui->setupUi(this);
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exitApp()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(settings()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->btOpen, SIGNAL(released()), this, SLOT(openSerialPort()));
    connect(ui->btClose, SIGNAL(released()), this, SLOT(closeSerialPort()));
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(ui->sbTrigger, SIGNAL(valueChanged(double)), this, SLOT(updateTrigger()));
    connect(ui->rbNone, SIGNAL(clicked()), this, SLOT(updateTriggerType()));
    connect(ui->rbRising, SIGNAL(clicked()), this, SLOT(updateTriggerType()));
    connect(ui->rbFalling, SIGNAL(clicked()), this, SLOT(updateTriggerType()));
    connect(ui->rbV20, SIGNAL(clicked()), this, SLOT(updateVdiv()));
    connect(ui->rbV10, SIGNAL(clicked()), this, SLOT(updateVdiv()));
    connect(ui->rbV5, SIGNAL(clicked()), this, SLOT(updateVdiv()));
    connect(ui->rbV1, SIGNAL(clicked()), this, SLOT(updateVdiv()));
    connect(ui->rbH100, SIGNAL(clicked()), this, SLOT(updateHdiv()));
    connect(ui->rbH50, SIGNAL(clicked()), this, SLOT(updateHdiv()));
    connect(ui->rbH10, SIGNAL(clicked()), this, SLOT(updateHdiv()));
    connect(ui->rbH5, SIGNAL(clicked()), this, SLOT(updateHdiv()));
    //connect(ui->rbH1, SIGNAL(clicked()), this, SLOT(updateHdiv()));
    connect(ui->rbH1, &QRadioButton::clicked, this, &MainWindow::updateHdiv);
    connect(ui->btExport, &QPushButton::clicked, this, &MainWindow::exportData);
    ui->rbV10->setChecked(true);
    ui->rbH10->setChecked(true);
    ui->sbTrigger->setValue(trigger);
    ui->btClose->setEnabled(false);
    ui->lbPic->setMinimumSize(400, 250);
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
    statusBar()->addWidget(m_statusLabel);
    m_statusLabel->setText("Disconnected");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readData()
{
    static bool isTriggered = true;
    static float oldxPos = 0;
    static uint16_t oldyPos = m_pixmap->height();
    static uint8_t prevValue = 0;
    static uint8_t deltaRising = 0;
    static uint8_t deltaFalling = 0;
    static uint16_t tsRising = 0;
    static uint16_t tsFalling = 0;
    static uint16_t cont = 0;
    uint16_t interval = 13;
    QByteArray data = m_serial->readAll();
    float xDelta = m_pixmap->width() * interval / 1000.0 / timeLenght;  // float needed in case delta is less than one

    for (uint16_t i = 0; i < data.size(); i++)
    {
        // Find V limits
        if ((uint8_t)data[i] > maxValue)
        {
            maxValue = data[i];
        }

        if ((uint8_t)data[i] < minValue)
        {
            minValue = data[i];
        }

        // Find triggers
        if ((uint8_t)data[i] - prevValue > 0)   // rising
        {
            deltaRising += (uint8_t)data[i] - prevValue;
            deltaFalling = 0;
        }

        if ((uint8_t)data[i] - prevValue < 0)   // falling
        {
            deltaFalling += prevValue - (uint8_t)data[i];
            deltaRising = 0;
        }

        prevValue = data[i];

        if (((float)deltaRising) / 0xFF * 5 > trigger)
        {
            if ((triggerType == rising  || triggerType == none) && cont - tsRising > 2)
            {
                isTriggered = true;
                rawFreq = cont - tsRising;
                rawDuty = cont - tsFalling;
            }

            deltaRising = 0;
            deltaFalling = 0;
            tsRising = cont;
        }

        if (((float)deltaFalling) / 0xFF * 5 > trigger)
        {
            if (triggerType == falling  && cont - tsFalling > 2)
            {
                isTriggered = true;
                rawFreq = cont - tsFalling;
                rawDuty = cont - tsRising;
            }

            deltaRising = 0;
            deltaFalling = 0;
            tsFalling = cont;
        }

        cont++;

        if (isTriggered == true || triggerType == none)  // draw new value
        {
            float newxPos = oldxPos + xDelta;
            uint16_t newyPos = m_pixmap->height() - (uint8_t)data[i] * m_pixmap->height() / 0xFF;
            newyPos = newyPos * 5 / volt + m_pixmap->height() * (6 - 5 / vDiv) / 6 / 2;

            if (newyPos < 0)
            {
                newyPos = 0;
            }

            if (newxPos > m_pixmap->width())
            {
                if (triggerType != none)
                {
                    isTriggered = false;
                    deltaRising = 0;
                    deltaFalling = 0;
                }

                ui->lbPic->setPixmap(*m_pixmap);

                if (pendingExport)
                {
                    if (!m_pixmap->toImage().save(fileName))
                    {
                        statusBar()->showMessage("Export error", 3000);
                    }
                    else
                    {
                        statusBar()->showMessage("Saved", 3000);
                    }

                    pendingExport = false;
                }

                drawBackground();
                newxPos -= m_pixmap->width();
                maxValue = 0;
                minValue = 0;
            }
            else
            {
                float timePos = (float)ui->slTime->value() / (ui->slTime->maximum() - ui->slTime->minimum()) * 0.25;
                float voltPos = (float)ui->slVolt->value() / (ui->slVolt->maximum() - ui->slVolt->minimum()) * 0.50;
                m_paint->drawLine(newxPos + m_pixmap->width() * timePos, newyPos + m_pixmap->height() * voltPos, oldxPos + m_pixmap->width() * timePos, oldyPos + m_pixmap->height() * voltPos);
            }

            oldxPos = newxPos;
            oldyPos = newyPos;
        }
    }
}

void MainWindow::drawBackground()
{
    uint16_t height = m_pixmap->height();
    uint16_t width = m_pixmap->width();
    m_paint->fillRect(0, 0, width, height, QColor(255, 255, 255, 255));
    QFont font = m_paint->font();
    font.setPixelSize(14);
    font.setBold(true);
    m_paint->setFont(font);
    m_paint->setPen(QColor(0, 0, 100, 255));
    m_paint->drawText(5, height - 5, QString::number(vDiv * 1000) + "mV/d    " + QString::number((float)timeLenght / 10 * 1000) + "μs/d    Vmax=" + QString::number((float)maxValue / 0xFF * 5, 'f',
                      3) + "    Vmin=" + QString::number((float)minValue / 0xFF * 5, 'f', 3));
    QString freq, duty;

    if (rawFreq == 0)
    {
        freq = "0";
        duty = "0";
    }
    else
    {
        freq = QString::number((float)1000000 / (rawFreq * interval), 'f', 0);
        duty = QString::number(100 * (float)rawDuty / rawFreq, 'f', 0);
    }

    m_paint->drawText(5, 20, freq + "Hz   " + duty + "%");
    // 0 volts line
    m_paint->setPen(QColor(0, 100, 0, 50));
    float voltPos = (float)ui->slVolt->value() / (ui->slVolt->maximum() - ui->slVolt->minimum()) * 0.50;
    uint16_t yPos = m_pixmap->height();
    yPos = yPos * 5 / volt + m_pixmap->height() * (6 - 5 / vDiv) / 6 / 2;
    m_paint->drawLine(0, yPos + m_pixmap->height() * voltPos, m_pixmap->width(), yPos + m_pixmap->height() * voltPos);
    m_paint->setPen(QColor(0, 0, 0, 123));

    for (uint8_t i = 0; i < 6; i++)     // horizontal lines
    {
        m_paint->drawLine(0, height * i / 6, width, height * i / 6);
    }

    for (uint8_t i = 0; i < 10; i++)     // vertical lines
    {
        m_paint->drawLine(width * i / 10, 0, width * i / 10, height);
    }

    m_paint->setPen(QColor(0, 0, 0, 255));
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    m_paint->end();
    *m_pixmap = m_pixmap->scaled(ui->lbPic->width(), ui->lbPic->height(), Qt::IgnoreAspectRatio);
    m_paint->begin(m_pixmap);
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::openSerialPort()
{
    m_serial->setPortName(m_ui_settings->m_currentSettings.name);
    m_serial->setBaudRate(m_ui_settings->m_currentSettings.baudRate);
    m_serial->setDataBits(m_ui_settings->m_currentSettings.dataBits);
    m_serial->setParity(m_ui_settings->m_currentSettings.parity);
    m_serial->setStopBits(m_ui_settings->m_currentSettings.stopBits);
    m_serial->setFlowControl(m_ui_settings->m_currentSettings.flowControl);
    interval = m_ui_settings->m_currentSettings.interval;

    if (m_serial->open(QIODevice::ReadOnly))
    {
        m_statusLabel->setText("Connected   (" + m_serial->portName() + "   " + QString::number(m_serial->baudRate()) + "bps   " + QString::number(interval) + "μs)");
        ui->btOpen->setEnabled(false);
        ui->btClose->setEnabled(true);
    }
    else
    {
        statusBar()->showMessage("Connection error", 3000);
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
    }

    statusBar()->showMessage("Disconnected");
    ui->btOpen->setEnabled(true);
    ui->btClose->setEnabled(false);
}

void MainWindow::updateTrigger()
{
    QString text = ui->sbTrigger->text();
    trigger = text.toFloat();
}

void MainWindow::updateTriggerType()
{
    if (ui->rbNone->isChecked())
    {
        triggerType = none;
    }
    else if (ui->rbRising->isChecked())
    {
        triggerType = rising;
    }
    else if (ui->rbFalling->isChecked())
    {
        triggerType = falling;
    }
}

void MainWindow::updateVdiv()
{
    if (ui->rbV20->isChecked())
    {
        vDiv = 2;
    }
    else if (ui->rbV10->isChecked())
    {
        vDiv = 1;
    }
    else if (ui->rbV5->isChecked())
    {
        vDiv = 0.5;
    }
    else if (ui->rbV1->isChecked())
    {
        vDiv = 0.1;
    }

    volt = vDiv * 6;
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::updateHdiv()
{
    if (ui->rbH100->isChecked())
    {
        timeLenght = 100;
    }
    else if (ui->rbH50->isChecked())
    {
        timeLenght = 50;
    }
    else if (ui->rbH10->isChecked())
    {
        timeLenght = 10;
    }
    else if (ui->rbH5->isChecked())
    {
        timeLenght = 5;
    }
    else if (ui->rbH1->isChecked())
    {
        timeLenght = 1;
    }

    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::settings()
{
    m_ui_settings->show();
}

void MainWindow::about()
{
    m_ui_about->show();
}

void MainWindow::exportData()
{
    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Save signal image"), "",
                                            tr("Bitmap image (*.bmp);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        if (m_serial->isOpen())
        {
            pendingExport = true;
        }
        else
        {
            if (!m_pixmap->toImage().save(fileName))
            {
                statusBar()->showMessage("Export error", 3000);
            }
            else
            {
                statusBar()->showMessage("Saved", 3000);
            }
        }
    }
}

void MainWindow::exitApp()
{
    QApplication::quit();
}




