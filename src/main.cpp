#include "SoundStream.h"

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

auto main(int argc, char *argv[]) -> int
{
    if (qEnvironmentVariableIsEmpty("QSG_RENDER_LOOP")) {
        qputenv("QSG_RENDER_LOOP", QByteArrayLiteral("basic"));
    }

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("Basic"));
    }

    if (qEnvironmentVariableIsEmpty(
            "SOUNDSTREAM_SHOW_KIRIGAMI_PLATFORM_WARNING")) {
        QLoggingCategory::setFilterRules(
            QStringLiteral("kf.kirigami.platform.warning=false"));
    }

    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("soundstream");

    KAboutData about(QStringLiteral("soundstream"),
                     i18n("SoundStream"),
                     QStringLiteral("0.1.0"),
                     i18n("PipeWire patchbay for KDE Plasma"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2026 SoundStream contributors"));
    KAboutData::setApplicationData(about);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("audio-x-generic")));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    const QUrl url(QStringLiteral("qrc:/org/kde/soundstream/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    SoundStream soundStream(app, engine);

    return app.exec();
}
