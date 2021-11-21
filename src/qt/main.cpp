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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlComponent>

#include "oscilloscope.h"

//jobject context;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
#ifdef Q_OS_ANDROID
    jobject context = QNativeInterface::QAndroidApplication::context();
    Oscilloscope oscilloscope(app.devicePixelRatio(), context);
    const QUrl url(QStringLiteral("qrc:/mainwindow_device.qml"));
#else
    Oscilloscope oscilloscope(app.devicePixelRatio());
    const QUrl url(QStringLiteral("qrc:/mainwindow.qml"));
#endif
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
    app.setWindowIcon(QIcon(":res/oscilloscope.png"));
    app.setOrganizationName("DanielGeA");
    app.setOrganizationDomain("DanielGeA");
    engine.rootContext()->setContextProperty("oscilloscope", &oscilloscope);
    engine.addImageProvider(QLatin1String("imgProvider"), &oscilloscope);
    engine.load(url);
    return app.exec();
}
