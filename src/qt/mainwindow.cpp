/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#ifdef Q_OS_ANDROID
MainWindow *MainWindow::m_instance = nullptr;

static void sendBuffer(JNIEnv *env, jobject /*thiz*/, jbyteArray array)
{
    //jboolean isCopy = JNI_TRUE;
    QByteArray buffer((char*)env->GetByteArrayElements(array,nullptr), env->GetArrayLength(array));
    emit MainWindow::instance()->receiveBuffer(buffer);
}

static void sendStatus(JNIEnv *env, jobject /*thiz*/, int status, jstring str)
{
    QString msg(env->GetStringUTFChars(str,nullptr));
    emit MainWindow::instance()->receiveStatus(status, msg);
}
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pixmap(new QPixmap(520, 230))
    , m_paint(new QPainter(m_pixmap))
    , m_ui_settings(new SettingsDialog(nullptr))
    , m_statusLabel(new QLabel())
#ifndef Q_OS_ANDROID
    , m_serial(new QSerialPort(this))
#endif

{
    ui->setupUi(this);
#ifdef Q_OS_ANDROID
    m_instance = this;
    connect(this, &MainWindow::receiveBuffer, this, &MainWindow::readData);
    connect(this, &MainWindow::receiveStatus, this, &MainWindow::readStatus);
    jclass classId = env.findClass("org/qtproject/qt/UsbSerialInterface");
    usbSerial = QJniObject(classId);
    JNINativeMethod methods[] {{"sendBuffer", "([B)V", reinterpret_cast<void *>(sendBuffer)}
        , {"sendStatus", "(ILjava/lang/String;)V", reinterpret_cast<void *>(sendStatus)}
    };
    env.registerNativeMethods(classId, methods, 2);
    m_ui_settings->fillPortsInfo(usbSerial);
#endif
    ui->lbPic->setMinimumSize(50, 50);
    ui->lbPic->setPixmap(*m_pixmap);
    statusBar()->addWidget(m_statusLabel);
    m_statusLabel->setText("Disconnected");
    ui->cbVdiv->addItem("2V/d");
    ui->cbVdiv->addItem("1V/d");
    ui->cbVdiv->addItem("500mV/d");
    ui->cbVdiv->addItem("100mV/d");
    ui->cbVdiv->setItemData(0, 2000);
    ui->cbVdiv->setItemData(1, 1000);
    ui->cbVdiv->setItemData(2, 500);
    ui->cbVdiv->setItemData(3, 100);
    ui->cbVdiv->setCurrentIndex(1);
    ui->cbHdiv->addItem("10ms/d");
    ui->cbHdiv->addItem("5ms/d");
    ui->cbHdiv->addItem("1ms/d");
    ui->cbHdiv->addItem("500us/d");
    ui->cbHdiv->addItem("100us/d");
    ui->cbHdiv->setItemData(0, 10000);
    ui->cbHdiv->setItemData(1, 5000);
    ui->cbHdiv->setItemData(2, 1000);
    ui->cbHdiv->setItemData(3, 500);
    ui->cbHdiv->setItemData(4, 100);
    ui->cbHdiv->setCurrentIndex(2);
    ui->cbTriggerType->addItem("None");
    ui->cbTriggerType->addItem("Rising");
    ui->cbTriggerType->addItem("Falling");
    ui->cbTriggerType->setCurrentIndex(1);
    QMainWindow::setWindowIcon(QIcon(":/oscilloscope.ico"));
    QMenu *menu = menuBar()->addMenu(tr("&Menu"));
    const QIcon connectIcon = QIcon(":/connect.png");
    connectAct = new QAction(connectIcon, tr("&Connect"), this);
    menu->addAction(connectAct);
    const QIcon disconnectIcon = QIcon(":/disconnect.png");
    disconnectAct = new QAction(disconnectIcon, tr("&Disconnect"), this);
    menu->addAction(disconnectAct);
    menu->addSeparator();
    const QIcon exportIcon = QIcon(":/save.png");
    QAction *exportAct = new QAction(exportIcon, tr("&Export..."), this);
    menu->addAction(exportAct);
    const QIcon settingsIcon = QIcon(":/settings.png");
    QAction *settingsAct = new QAction(settingsIcon, tr("&Settings..."), this);
    menu->addAction(settingsAct);
    menu->addSeparator();
    const QIcon aboutIcon = QIcon(":/info.png");
    QAction *aboutAct = new QAction(aboutIcon, tr("&About"), this);
    menu->addAction(aboutAct);
    const QIcon exitIcon = QIcon(":/application-exit.png");
    QAction *exitAct = new QAction(exitIcon, tr("&Exit"), this);
    menu->addAction(exitAct);
#ifndef Q_OS_ANDROID
    QToolBar *toolBar = addToolBar(tr("Toolbar"));
    toolBar->addAction(connectAct);
    toolBar->addAction(disconnectAct);
    toolBar->addSeparator();
    toolBar->addAction(exportAct);
    toolBar->addAction(settingsAct);
    toolBar->addSeparator();
    toolBar->addAction(aboutAct);
    toolBar->addSeparator();
    toolBar->addAction(exitAct);
    disconnectAct->setEnabled(false);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
#endif
    connect(connectAct, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(disconnectAct, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::settings);
    connect(exportAct, &QAction::triggered, this, &MainWindow::exportData);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    connect(exitAct, &QAction::triggered, this, &MainWindow::exitApp);
    connect(ui->cbHdiv, &QComboBox::currentTextChanged, this, &MainWindow::updateHdiv);
    connect(ui->cbVdiv, &QComboBox::currentTextChanged, this, &MainWindow::updateVdiv);
    connect(ui->cbTriggerType, &QComboBox::currentTextChanged, this, &MainWindow::updateTriggerType);
    connect(ui->sbTriggerValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateTrigger);
    connect(m_ui_settings, &SettingsDialog::accepted, this, &MainWindow::refresh);
    ui->lbPic->adjustSize();
    drawBackground();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processData(const QByteArray &data)
{
    static bool isTriggered = true;
    static float oldxPos = 0;
    static int16_t oldyPos = m_pixmap->height();
    static uint8_t prevValue = 0;
    static uint8_t deltaRising = 0;
    static uint8_t deltaFalling = 0;
    static uint16_t tsRising = 0;
    static uint16_t tsFalling = 0;
    static uint16_t cont = 0;
    m_paint->setPen(QPen(QColor(m_ui_settings->m_currentSettings.signalcolor), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    float xDelta = m_pixmap->width() * interval / 1000.0 / timeLenght;  // float needed in case delta is less than one
    for (uint16_t i = 0; i < data.size(); i++) {
        // Find V limits
        if ((uint8_t)data[i] > maxValue) {
            maxValue = data[i];
        }
        if ((uint8_t)data[i] < minValue) {
            minValue = data[i];
        }
        // Find triggers and signal properties
        if ((uint8_t)data[i] - prevValue > 0) { // rising
            deltaRising += (uint8_t)data[i] - prevValue;
            deltaFalling = 0;
        }
        if ((uint8_t)data[i] - prevValue < 0) { // falling
            deltaFalling += prevValue - (uint8_t)data[i];
            deltaRising = 0;
        }
        prevValue = data[i];
        if (((float)deltaRising) / 0xFF * 5 > trigger) {
            if ((triggerType == rising  || triggerType == none) && cont - tsRising > 2) {
                isTriggered = true;
                rawFreq = cont - tsRising;
                rawDuty = cont - tsFalling;
            }
            deltaRising = 0;
            deltaFalling = 0;
            tsRising = cont;
        }
        if (((float)deltaFalling) / 0xFF * 5 > trigger) {
            if (triggerType == falling  && cont - tsFalling > 2) {
                isTriggered = true;
                rawFreq = cont - tsFalling;
                rawDuty = cont - tsRising;
            }
            deltaRising = 0;
            deltaFalling = 0;
            tsFalling = cont;
        }
        cont++;
        // Draw new value
        if (isTriggered == true || triggerType == none) {
            float newxPos = oldxPos + xDelta;
            int16_t newyPos = m_pixmap->height() - (uint8_t)data[i] * m_pixmap->height() / 0xFF;
            newyPos = newyPos * 5 / volt + m_pixmap->height() * (6 - 5 / vDiv) / 6 / 2;
            // End of pixmap
            if (newxPos > m_pixmap->width()) {
                if (triggerType != none) {
                    isTriggered = false;
                    deltaRising = 0;
                    deltaFalling = 0;
                }
                ui->lbPic->setPixmap(*m_pixmap);
                if (pendingExport) {
                    if (!m_pixmap->toImage().save(fileName)) {
                        statusBar()->showMessage("Export error", 3000);
                    } else {
                        statusBar()->showMessage("Saved", 3000);
                    }
                    pendingExport = false;
                }
                drawBackground();
                m_paint->setPen(QPen(QColor(m_ui_settings->m_currentSettings.signalcolor), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                newxPos = 0;
                maxValue = 0;
                minValue = 0;
            }
            // Draw new value
            else {
                float timePos = (float)ui->slTime->value() / (ui->slTime->maximum() - ui->slTime->minimum()) * 0.25;
                float voltPos = (float)ui->slVolt->value() / (ui->slVolt->maximum() - ui->slVolt->minimum());
                m_paint->drawLine(newxPos + m_pixmap->width() * timePos, newyPos + m_pixmap->height() * voltPos, oldxPos + m_pixmap->width() * timePos, oldyPos + m_pixmap->height() * voltPos);
            }
            oldxPos = newxPos;
            oldyPos = newyPos;
        }
    }
}

#ifndef Q_OS_ANDROID
void MainWindow::readData()
{
    QByteArray data = m_serial->readAll();
    processData(data);

}
#else
void MainWindow::readData(const QByteArray &data)
{
    processData(data);
}

void MainWindow::readStatus(int status, const QString &msg)
{
    this->status = (Status)status;
    switch( status ) {
    case Status::CONNECTED:
        m_statusLabel->setText("Connected (" + msg + ")");
        statusBar()->showMessage("Connected (" + msg + ")");
        //connectAct->setEnabled(false);
        //disconnectAct->setEnabled(true);
        break;
    case Status::DISCONNECTED:
        m_statusLabel->setText("Disconnected");
        statusBar()->showMessage("Disconnected");
        //connectAct->setEnabled(false);
        //disconnectAct->setEnabled(false);
        break;
    case Status::CONNECTION_LOST:
        //statusBar()->showMessage("Connection lost", 3000);
        statusBar()->showMessage("Disconnected");
        m_statusLabel->setText("Disconnected");
        //connectAct->setEnabled(true);
        //disconnectAct->setEnabled(false);
        break;
    case Status::ERROR_CONNECTING:
        statusBar()->showMessage("Connection error", 3000);
        break;
    case Status::ERROR_OPENING:
        statusBar()->showMessage("Connection error", 3000);
        break;
    case Status::PERMISSION_NOT_GRANTED:
        statusBar()->showMessage("Permission not granted", 3000);
        break;
    }
}
#endif

void MainWindow::drawBackground()
{
    uint16_t height = m_pixmap->height();
    uint16_t width = m_pixmap->width();
    m_paint->fillRect(0, 0, width, height, QColor(m_ui_settings->m_currentSettings.backgroundcolor));
    // 0 volts line
    m_paint->setPen(QPen(Qt::red, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    float voltPos = (float)ui->slVolt->value() / (ui->slVolt->maximum() - ui->slVolt->minimum());
    uint16_t yPos = m_pixmap->height() * (5 / volt + (6 - 5 / vDiv) / 6 / 2);
    m_paint->drawLine(0, yPos + m_pixmap->height() * voltPos, m_pixmap->width(), yPos + m_pixmap->height() * voltPos);
    m_paint->setPen(QColor(m_ui_settings->m_currentSettings.gridcolor));
    for (uint8_t i = 0; i < 6; i++) {   // horizontal lines
        m_paint->drawLine(0, height * i / 6, width, height * i / 6);
    }
    for (uint8_t i = 0; i < 10; i++) {   // vertical lines
        m_paint->drawLine(width * i / 10, 0, width * i / 10, height);
    }
    QFont font = m_paint->font();
    font.setPixelSize(14);
    m_paint->setFont(font);
    m_paint->setPen(QColor(m_ui_settings->m_currentSettings.textcolor));
    m_paint->drawText(5, height - 5, QString::number(vDiv * 1000) + "mV/d    " + QString::number((float)timeLenght / 10 * 1000) + "μs/d    Vmax=" + QString::number((float)maxValue / 0xFF * 5, 'f',
                      3) + "    Vmin=" + QString::number((float)minValue / 0xFF * 5, 'f', 3));
    QString freq, duty;
    if (rawFreq == 0) {
        freq = "0";
        duty = "0";
    } else {
        freq = QString::number((float)1000000 / (rawFreq * interval), 'f', 0);
        duty = QString::number(100 * (float)rawDuty / rawFreq, 'f', 0);
    }
    m_paint->drawText(5, 20, freq + "Hz   " + duty + "%");
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

#ifndef Q_OS_ANDROID
void MainWindow::openSerialPort()
{
    m_serial->setPortName(m_ui_settings->m_currentSettings.name);
    m_serial->setBaudRate(m_ui_settings->m_currentSettings.baudRate);
    m_serial->setDataBits(m_ui_settings->m_currentSettings.dataBits);
    m_serial->setParity(m_ui_settings->m_currentSettings.parity);
    m_serial->setStopBits(QSerialPort::StopBits::OneStop);
    m_serial->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    interval = m_ui_settings->m_currentSettings.interval;
    if (m_serial->open(QIODevice::ReadOnly)) {
        m_statusLabel->setText("Connected   (" + m_serial->portName() + "   " + QString::number(m_serial->baudRate()) + "bps   " + QString::number(interval) + "μs)");
        connectAct->setEnabled(false);
        disconnectAct->setEnabled(true);
    } else {
        statusBar()->showMessage("Connection error", 3000);
    }
}
#else
void MainWindow::openSerialPort()
{
    statusBar()->showMessage("Connecting...");
    uint32_t baudrate = m_ui_settings->m_currentSettings.baudRate;
    uint8_t dataBits = m_ui_settings->m_currentSettings.dataBits;
    uint8_t parity;
    if (m_ui_settings->m_currentSettings.parity == QSerialPort::OddParity)
        parity = 1;
    else
        parity = m_ui_settings->m_currentSettings.parity;
    interval = m_ui_settings->m_currentSettings.interval;
    jstring name = env->NewStringUTF(m_ui_settings->m_currentSettings.name.toStdString().c_str());
    QJniObject result = usbSerial.callObjectMethod
                        ("openConnection"
                         , "(Landroid/content/Context;Ljava/lang/String;IIII)Ljava/lang/String;"
                         , QNativeInterface::QAndroidApplication::context(), name, baudrate, dataBits, 1, parity);
}
#endif

void MainWindow::closeSerialPort()
{
#ifndef Q_OS_ANDROID
    if (m_serial->isOpen()) {
        m_serial->close();
    }
#else
    usbSerial.callMethod<void>("closeConnection");
#endif
    m_statusLabel->setText("Disconnected");
    connectAct->setEnabled(true);
    disconnectAct->setEnabled(false);
}

void MainWindow::updateTrigger()
{
    QString text = ui->sbTriggerValue->text();
    trigger = text.toFloat();
}

void MainWindow::updateTriggerType()
{
    triggerType = (TriggerType)ui->cbTriggerType->currentIndex();
}

void MainWindow::updateVdiv()
{
    vDiv = ui->cbVdiv->currentData().toFloat() / 1000;
    volt = vDiv * 6;
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::updateHdiv()
{
    hDiv = ui->cbHdiv->currentData().toFloat() / 1000;
    timeLenght = hDiv * 10;
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::settings()
{
#ifdef Q_OS_ANDROID
    m_ui_settings->showMaximized();
#else
    m_ui_settings->move(x() + (width() - m_ui_settings->width()) / 2,
                        y() + (height() - m_ui_settings->height()) / 2);
    m_ui_settings->show();
#endif

}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About"),
                       tr("<center><b>Oscilloscope Serial </b>v0.1<br>"
                          "<br>"
                          "DanielGeA, 2021"));
}

void MainWindow::exportData()
{
    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Save signal image"), "",
                                            tr("Bitmap image (*.bmp);;All Files (*)"));
#ifndef Q_OS_ANDROID
    if (fileName.isEmpty()) {
        return;
    } else {
        if (m_serial->isOpen()) {
            pendingExport = true;
        } else {
            if (!m_pixmap->toImage().save(fileName)) {
                statusBar()->showMessage("Export error", 3000);
            } else {
                statusBar()->showMessage("Saved", 3000);
            }
        }
    }
#else
    if (fileName.isEmpty()) {
        return;
    } else {
        if (this->status == Status::CONNECTED) {
            pendingExport = true;
        } else {
            if (!m_pixmap->toImage().save(fileName)) {
                statusBar()->showMessage("Export error", 3000);
            } else {
                statusBar()->showMessage("Saved", 3000);
            }
        }
    }
#endif
}

void MainWindow::refresh()
{
    drawBackground();
    ui->lbPic->setPixmap(*m_pixmap);
}

void MainWindow::exitApp()
{
    QApplication::quit();
}




