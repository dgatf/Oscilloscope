#include "oscilloscope.h"

#ifdef Q_OS_ANDROID
Oscilloscope *Oscilloscope::m_instance = nullptr;

static void sendBuffer(JNIEnv *env, jobject /*thiz*/, jbyteArray array)
{
    //jboolean isCopy = JNI_TRUE;
    QByteArray buffer((char*)env->GetByteArrayElements(array,nullptr), env->GetArrayLength(array));
    emit Oscilloscope::instance()->receiveBuffer(buffer);
}

static void sendStatus(JNIEnv *env, jobject /*thiz*/, int status, jstring str)
{
    QString msg(env->GetStringUTFChars(str,nullptr));
    emit Oscilloscope::instance()->receiveStatus(status, msg);
}
#endif

#ifndef Q_OS_ANDROID
Oscilloscope::Oscilloscope(qreal pixelRatio)
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , pixelRatio(pixelRatio)
    , pixmap(new QPixmap(pmWidth, pmHeight))
    , paint(new QPainter(pixmap))
    , serial(new QSerialPort())
{
}
#else
Oscilloscope::Oscilloscope(qreal pixelRatio, jobject context)
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , pixelRatio(pixelRatio)
    , pixmap(new QPixmap(pmWidth, pmHeight))
    , paint(new QPainter(pixmap))
    , context(context)
{
    m_instance = this;
    connect(this, &Oscilloscope::receiveBuffer, this, &Oscilloscope::readData);
    connect(this, &Oscilloscope::receiveStatus, this, &Oscilloscope::readStatus);
    jclass classId = env.findClass("org/qtproject/qt/UsbSerialInterface");
    usbSerial = QJniObject(classId);
    JNINativeMethod methods[] {{"sendBuffer", "([B)V", reinterpret_cast<void *>(sendBuffer)}
        , {"sendStatus", "(ILjava/lang/String;)V", reinterpret_cast<void *>(sendStatus)}
    };
    env.registerNativeMethods(classId, methods, 2);
}
#endif

Oscilloscope::~Oscilloscope()
{
}

QPixmap Oscilloscope::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    *size = QSize(pmWidth, pmHeight);
    if (*size != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
        paint->end();
        *pixmap = pixmap->scaled(requestedSize, Qt::IgnoreAspectRatio);
        pmHeight = requestedSize.height();
        pmWidth = requestedSize.width();
        paint->begin(pixmap);
        yZeroV = getYZeroV();
        drawBackground();
    }
    return *pixmap;
}

#ifndef Q_OS_ANDROID
void Oscilloscope::openSerialPort(QString portName, QString baudrate, QString dataBits, QString parity, QString interval)
{
    connect(serial, &QSerialPort::readyRead, this, &Oscilloscope::readData);
    serial->setPortName(portName);
    serial->setBaudRate(baudrate.toUInt());
    serial->setDataBits((QSerialPort::DataBits)dataBits.toUInt());
    serial->setParity((QSerialPort::Parity)parity.toUInt());
    serial->setStopBits(QSerialPort::StopBits::OneStop);
    serial->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    this->interval = interval.toUInt();
    if (serial->open(QIODevice::ReadOnly)) {
        status = connected;
        emit sendStatusConn(status);
        QString message("Connected   (" + portName + "   " + baudrate + "bps   " + interval + "μs)");
        emit sendMessage(message);
    } else {
        status = disconnected;
        emit sendMessage("Connection error", 3000);
    }
}
#else
void Oscilloscope::openSerialPort(QString portName, QString baudrate, QString dataBits, QString parity, QString interval)
{
    this->interval = interval.toUInt();
    quint8 parity_android;
    if (parity.toUInt() == QSerialPort::OddParity)
        parity_android = 1;
    else
        parity_android = parity.toUInt();
    jstring name = env->NewStringUTF(portName.toStdString().c_str());
    usbSerial.callMethod<void>
                        ("openConnection"
                         , "(Landroid/content/Context;Ljava/lang/String;IIII)V"
                         , context, name, baudrate.toUInt(), dataBits.toUInt(), 1, parity_android);
}
#endif

void Oscilloscope::closeSerialPort()
{
#ifndef Q_OS_ANDROID

    if (serial->isOpen()) {
        serial->close();
    }
#else
    usbSerial.callMethod<void>("closeConnection");
#endif
    status = disconnected;
    emit sendStatusConn(status);
    emit sendMessage("Disconnected");
    isPaused = false;
    emit isPausedChanged(false);
}

#ifndef Q_OS_ANDROID
void Oscilloscope::readData()
{
    QByteArray data = serial->readAll();
    if (!isPaused)
        processData(data);
}
#else
void Oscilloscope::readData(const QByteArray &data)
{
    if (!isPaused)
        processData(data);
}

void Oscilloscope::readStatus(int status, const QString &message)
{
    this->status = (Status)status;
    switch(status) {
    case connected:
        this->status = connected;
        emit sendStatusConn(this->status);
        emit sendMessage(message);
        break;
    case disconnected:
        this->status = disconnected;
        emit sendStatusConn(this->status);
        emit sendMessage("Disconnected");
        break;
    case connection_lost:
        this->status = disconnected;
        emit sendStatusConn(this->status);
        emit sendMessage("Disconnected");
        emit sendMessage("Connection lost", 3000);
        break;
    case error_connecting:
        emit sendMessage("Connection error", 3000);
        break;
    case error_opening:
        emit sendMessage("Opening error", 3000);
        break;
    case permission_not_granted:
        emit sendMessage("Permission not granted", 3000);
        break;
    }
}
#endif

#ifndef Q_OS_ANDROID
QStringList Oscilloscope::fillPortsInfo()
{
    const QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    QStringList list;
    for (const QSerialPortInfo& info : infos) {
        list.append(info.portName());
    }
    return list;
}
#else
QStringList Oscilloscope::fillPortsInfo()
{
    QJniObject drivers = usbSerial.callObjectMethod
                         ("drivers"
                          , "(Landroid/content/Context;)Ljava/util/List;"
                          , context);
    int size = drivers.callMethod<jint>("size");
    QStringList list;
    for (int i = 0; i < size; i++) {
        QJniObject driver = drivers.callObjectMethod("get", "(I)Ljava/lang/Object;", i);
        QString name = driver.toString();
        if (name.contains("com.hoho.android.usbserial.driver."))
            name = name.right(name.length() - QString("com.hoho.android.usbserial.driver.").length());
        list.append(name);
    }
    return list;
}
#endif

void Oscilloscope::processData(const QByteArray &data)
{
    static bool isTriggered = true;
    static float oldxPos = 0;
    static qint16 oldyPos = pixmap->height();
    static quint8 prevValue = 0;
    static quint16 deltaRising = 0;
    static quint16 deltaFalling = 0;
    static quint16 tsRising = 0;
    static quint16 tsFalling = 0;
    static quint16 triggerCont = 0;
    quint16 height = pmHeight;
    quint16 width = pmWidth;
    paint->setPen(QPen(QColor(signalColor), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    float xDelta = width * interval / 1000.0 / timeRange;
    for (quint16 i = 0; i < data.size(); i++) {
        // Find V limits
        if ((quint8)data[i] > maxValue) {
            maxValue = data[i];
        }
        if ((quint8)data[i] < minValue) {
            minValue = data[i];
        }
        // Find triggers and signal properties
        if ((quint8)data[i] - prevValue > 0) { // rising
            deltaRising += (quint8)data[i] - prevValue;
            deltaFalling = 0;
        }
        if ((quint8)data[i] - prevValue < 0) { // falling
            deltaFalling += prevValue - (quint8)data[i];
            deltaRising = 0;
        }
        prevValue = data[i];
        if (((float)deltaRising) / 0xFF * 5 > triggerValue / 1000.0) {
            if ((triggerType == rising || triggerType == none) && triggerCont - tsRising > 2) {
                isTriggered = true;
                rawFreq = triggerCont - tsRising;
                rawDuty = triggerCont - tsFalling;
                rawFreqAvg = 0.1 * rawFreq + 0.9 * rawFreqAvg;
                rawDutyAvg = 0.1 * rawDuty + 0.9 * rawDutyAvg;
            }
            deltaRising = 0;
            deltaFalling = 0;
            tsRising = triggerCont;
        }
        if (((float)deltaFalling) / 0xFF * 5 > triggerValue / 1000.0) {
            if (triggerType == falling  && triggerCont - tsFalling > 2) {
                isTriggered = true;
                rawFreq = triggerCont - tsFalling;
                rawDuty = triggerCont - tsRising;
                rawFreqAvg = 0.1 * rawFreq + 0.9 * rawFreqAvg;
                rawDutyAvg = 0.1 * rawDuty + 0.9 * rawDutyAvg;
            }
            deltaRising = 0;
            deltaFalling = 0;
            tsFalling = triggerCont;
        }
        triggerCont++;
        // Capture
        if (isCapturing) {
            captureBuffer->append(data[i]);
        }
        // Draw new value
        if (isTriggered == true || triggerType == none) {
            float newxPos = oldxPos + xDelta;
            qint16 newyPos = height + yZeroV - getYVal(data[i]);
            // End of pixmap
            if (newxPos > width) {
                if (triggerType != none) {
                    isTriggered = false;
                    deltaRising = 0;
                    deltaFalling = 0;
                }
                emit sendPixmap();

                if (pendingExport) {
                    savePixmap(filename);
                    pendingExport = false;
                }
                if (isCapturing) {
                    isCapturing = false;
                    saveCsv(filename);
                    delete captureBuffer;
                }
                if (pendingCsv) {
                    isCapturing = true;
                    pendingCsv = false;
                    captureBuffer = new QByteArray(timeRange/interval+1, 0);
                }

                if (pendingPause) {
                    pendingPause = false;
                    isPaused = true;
                    emit isPausedChanged(true);
                    return;
                }
                drawBackground();
                paint->setPen(QPen(QColor(signalColor), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                newxPos = 0;
                maxValue = 0;
                minValue = 0;
            }
            // Draw new value
            else {
                paint->drawLine(newxPos + width * hAdjust, newyPos + height * vAdjust, oldxPos + width * hAdjust, oldyPos + height * vAdjust);
            }
            oldxPos = newxPos;
            oldyPos = newyPos;
        }
    }
}

qint16 Oscilloscope::getYZeroV()
{
    return pmHeight * (5-voltRange)/voltRange/2;
}

quint16 Oscilloscope::getYVal(quint8 value)
{
    return value * 5.0 / 0xFF * pmHeight / voltRange;
}

void Oscilloscope::drawBackground()
{
    quint16 height = pmHeight;
    quint16 width = pmWidth;
    paint->fillRect(0, 0, width, height, QColor(backgroundColor));
    // 0 volts line
    paint->setPen(QPen(Qt::red, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    quint16 yPos = height  + yZeroV;
    paint->drawLine(0, yPos + height * vAdjust, width, yPos + height * vAdjust);
    paint->setPen(QColor(gridColor));
    for (quint8 i = 0; i < 6; i++) {   // horizontal lines
        paint->drawLine(0, height * i / 6, width, height * i / 6);
    }
    for (quint8 i = 0; i < 10; i++) {   // vertical lines
        paint->drawLine(width * i / 10, 0, width * i / 10, height);
    }
    QFont font = paint->font();
    font.setPixelSize(14 * pixelRatio);
    paint->setFont(font);
    paint->setPen(QColor(textColor));
    paint->drawText(5, height - 5, QString::number(vDiv) + "mV/d    " + QString::number(hDiv) + "μs/d    Vmax=" + QString::number((float)maxValue / 0xFF * 5, 'f',
                    3) + "    Vmin=" + QString::number((float)minValue / 0xFF * 5, 'f', 3));
    QString freq, duty;
    if (rawFreqAvg == 0) {
        freq = "0";
        duty = "0";
    } else {
        freq = QString::number((float)1000000 / (rawFreqAvg * interval), 'f', 0);
        duty = QString::number(100 * (float)rawDutyAvg / rawFreqAvg, 'f', 0);
    }
    paint->drawText(5, font.pixelSize() + 5, freq + "Hz   " + duty + "%");
}

void Oscilloscope::exportImage(QUrl url)
{
    QString filename;
#ifndef Q_OS_ANDROID
    filename = url.path();
#else
    filename = url.toString();
#endif
    if (filename.isEmpty()) {
        return;
    } else {
        if (status == connected && !isPaused) {
            pendingExport = true;
            this->filename = filename;
        } else
            savePixmap(filename);
    }
}

void Oscilloscope::savePixmap(QString filename)
{
    if (!pixmap->toImage().save(filename, "BMP")) {
        emit sendMessage("Export error", 3000);
    } else {
        emit sendMessage("Exported", 3000);
    }
}

void Oscilloscope::exportCsv(QUrl url)
{
    QString filename;
#ifndef Q_OS_ANDROID
    filename = url.path();
#else
    filename = url.toString();
#endif
    if (filename.isEmpty()) {
        return;
    } else {
        if (status == connected) {
            if (isPaused) {
                isPaused = false;
                emit isPausedChanged(false);
            }
            pendingCsv = true;
            this->filename = filename;
        }
    }
}

void Oscilloscope::saveCsv(QString filename)
{
    QFile data(filename);
    if(data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream output(&data);
        output << "Interval\t" + QString::number(interval) + "\n";
        for (quint16 i=0; i < captureBuffer->size(); i++) {
            float volt = (quint8)captureBuffer->at(i) * 5.0/0xFF;
            output << QString::number(volt, 'f', 3) + "\n";
        }
    }
    data.close();
}

void Oscilloscope::setIsPaused(const bool value) {
    if (value == true)
        pendingPause = true;
    else
    {
        isPaused = false;
        pendingPause = false;
        emit isPausedChanged(false);
    }
}
