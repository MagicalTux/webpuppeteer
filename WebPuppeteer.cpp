#include <QCoreApplication>
#include <QFile>
#include <QScriptEngine>
#include "WebPuppeteer.hpp"
#include "WebPuppeteerConsole.hpp"

WebPuppeteer::WebPuppeteer(QString _file) {
	file = _file;
	exit_code = 0;
	console = new WebPuppeteerConsole(this);
}

void WebPuppeteer::start() {
	QFile f(file);
	if (!f.exists()) {
		qDebug("script does not exist");
		QCoreApplication::exit(1);
		return;
	}

	if (!f.open(QIODevice::ReadOnly)) {
		qDebug("script could not be opened");
		QCoreApplication::exit(2);
		return;
	}

	QScriptEngine e;
	e.globalObject().setProperty("console", e.newQObject(console, QScriptEngine::QtOwnership, QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater));
	QScriptValue res = e.evaluate(QString::fromUtf8(f.readAll()), file);

	if (res.isError()) {
		qDebug("Error while executing script: %s", qPrintable(res.toString()));
		exit_code = 1;
	}

	QCoreApplication::exit(exit_code);
}

