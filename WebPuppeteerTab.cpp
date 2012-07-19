#include "WebPuppeteerTab.hpp"
#include "WebPuppeteerWebElement.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QWebFrame>
#include <QPainter>
#include <QPrinter>
#include <QWebView>
#include <QNetworkReply>
#include <QEvent>
#include <QKeyEvent>

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *_parent): QWebPage(_parent) {
	parent = _parent;

	// define standard values
	setViewportSize(QSize(1024,768));
	setForwardUnsupportedContent(true);

	// disable scrollbars, not as anyone is going to use them anyway
	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadFile(QNetworkReply*)));
}

void WebPuppeteerTab::downloadFile(QNetworkReply*reply) {
	// test stuff
	qDebug("finished: %s", reply->isFinished() ? "yes":"no");
}

WebPuppeteer *WebPuppeteerTab::getParent() {
	return parent;
}

bool WebPuppeteerTab::browse(const QString &url) {
	mainFrame()->load(QUrl(url));
	return wait();
}

void WebPuppeteerTab::setReturnBool(bool r) {
	return_bool = r;
}

bool WebPuppeteerTab::screenshot(const QString &filename) {
	QImage img(viewportSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::fullshot(const QString &filename) {
	QImage img(mainFrame()->contentsSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::print(const QString &filename) {
	QPrinter print;
	print.setOutputFileName(filename);
	print.setOutputFormat(QPrinter::PdfFormat);
	print.setPaperSize(QPrinter::A4);

//	print.setPageMargins(0, 0, 0, 0, QPrinter::Inch);
	// we know our page is 72dpi, how many dpi do we need on the printer to fit it ?
//	int dpi = (mainFrame()->contentsSize().width() * print.paperSize(QPrinter::Inch).width() / 72.0) * 1.06;
//	print.setResolution(dpi);
//	QPainter print_p(&print);
//	QSize size = mainFrame()->contentsSize();
//	mainFrame()->render(&print_p, QWebFrame::ContentsLayer, QRegion(0, 0, size.width(), size.height()));
//	print_p.end();
	mainFrame()->print(&print);
	return true;
}

QScriptValue WebPuppeteerTab::eval(const QString &js) {
	return parent->engine().newVariant(mainFrame()->evaluateJavaScript(js));
}

QScriptValue WebPuppeteerTab::document() {
	return parent->engine().newQObject(new WebPuppeteerWebElement(this, mainFrame()->documentElement()), QScriptEngine::ScriptOwnership);
}

QString WebPuppeteerTab::treeDump() {
	return mainFrame()->renderTreeDump();
}

void WebPuppeteerTab::interact() {
	QEventLoop e;

	QWebView *v = new QWebView();
	v->setAttribute(Qt::WA_DeleteOnClose, true);
	connect(v, SIGNAL(destroyed(QObject*)), &e, SLOT(quit()));
	v->setPage(this);
	v->show();
	e.exec();
}

QScriptValue WebPuppeteerTab::get(const QString &url) {
	QNetworkRequest req(url);
	QNetworkReply *rep = networkAccessManager()->get(req);
	QEventLoop e;

	connect(rep, SIGNAL(finished()), &e, SLOT(quit()));
	e.exec();

	if (rep->error() != QNetworkReply::NoError) {
		qDebug("GET error: %s", qPrintable(rep->errorString()));
		rep->deleteLater();
		return parent->engine().currentContext()->throwError(QScriptContext::UnknownError, rep->errorString());
	}

	return parent->engine().newVariant((rep->readAll()));
}

bool WebPuppeteerTab::wait() {
	QEventLoop e;

	connect(this, SIGNAL(loadFinished(bool)), &e, SLOT(quit()));
	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setReturnBool(bool)));
	e.exec();

	return return_bool;
}

void WebPuppeteerTab::type(const QString &text) {
	QKeyEvent ev(QEvent::KeyPress, 0, Qt::NoModifier, text);
	event(&ev);
}

void WebPuppeteerTab::typeEnter() {
	QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
	event(&ev);
}

