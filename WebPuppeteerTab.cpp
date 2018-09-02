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
#include <QTimer>
#include <QBuffer>
#include <QTemporaryFile>
#include "TimeoutTrigger.hpp"

WebPuppeteerTabNetSpy::WebPuppeteerTabNetSpy(QObject *parent): QNetworkAccessManager(parent) {
	connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(spyFinished(QNetworkReply*)));
	cnx_count = 0;
	cnx_index = 0;
	data_output = NULL;
}

int WebPuppeteerTabNetSpy::getCount() const {
	return cnx_count;
}

void WebPuppeteerTabNetSpy::setOutputFile(QFile *output) {
	if (data_output != NULL) {
		data_output->close();
		data_output->deleteLater();
	}
	data_output = output;
	if (data_output != NULL) {
		// write version
		qint64 p = 1+8+8+8;
		data_output->write((const char *)&p, sizeof(p));
		data_output->write(QByteArray(1, '\xff')); // version

		// timestamp
		qint64 t = QDateTime::currentMSecsSinceEpoch();
		data_output->write((const char *)&t, sizeof(t));

		// query id (zero)
		p = 0;
		data_output->write((const char *)&p, sizeof(p));

		// version
		p = 0x01;
		data_output->write((const char *)&p, sizeof(p));
	}
}

QNetworkReply *WebPuppeteerTabNetSpy::createRequest(Operation op, const QNetworkRequest &oreq, QIODevice *outgoingData) {
	cnx_index += 1;
	// we end duplicating request :(
	QNetworkRequest req = oreq;
	req.setAttribute(QNetworkRequest::User, cnx_index);

	if (data_output != NULL) {
		// store request in output file
		QByteArray op_str;

		// fetch HTTP operation
		switch (op) {
			case QNetworkAccessManager::HeadOperation:
				op_str = "HEAD";
				break;
			case QNetworkAccessManager::GetOperation:
				op_str = "GET";
				break;
			case QNetworkAccessManager::PutOperation:
				op_str = "PUT";
				break;
			case QNetworkAccessManager::PostOperation:
				op_str = "POST";
				break;
			case QNetworkAccessManager::DeleteOperation:
				op_str = "DELETE";
				break;
			case QNetworkAccessManager::CustomOperation:
				op_str = req.attribute(QNetworkRequest::CustomVerbAttribute).toByteArray();
				break;
			case QNetworkAccessManager::UnknownOperation:
				op_str = "????";
				break;
		}

		// store request, first store size
		qint64 p = 0;
		data_output->write((const char *)&p, sizeof(p));
		// keep start of record position in memory
		p = data_output->pos() - sizeof(p);

		// write request
		data_output->write(QByteArray(1, '\x01'));
		// write time
		qint64 t = QDateTime::currentMSecsSinceEpoch();
		data_output->write((const char *)&t, sizeof(t));
		data_output->write((const char *)&cnx_index, sizeof(cnx_index));
		data_output->write(op_str + QByteArray(1, 0) + req.url().toEncoded() + QByteArray(1, 0));

		// write headers
		QList<QByteArray> h = req.rawHeaderList();

		// header count
		int h_size = h.size();
		data_output->write((const char *)&h_size, sizeof(h_size));

		// each request header
		for (int i = 0; i < h.size(); ++i) {
			data_output->write(h.at(i) + QByteArray(1, 0) + req.rawHeader(h.at(i)) + QByteArray(1, 0));
		}

		// write body, if any
		if (outgoingData != NULL) {
			qint64 out_pos = outgoingData->pos();
			qint64 out_end = outgoingData->size();
			qint64 out_len = out_end - out_pos;

			data_output->write((const char *)&out_len, sizeof(out_len));
			if (out_len > 0) {
				// write actual data
				char buf[8192];

				while(true) {
					qint64 c = outgoingData->read((char*)&buf, sizeof(buf));
					if (c == 0) {
						break;
					}
					if (c == -1) {
						// wtf?
						qDebug("error reading outgoingData");
						break;
					}

					data_output->write((const char*)&buf, c);
				}

				// reset position to initial state (hopefully seekable)
				outgoingData->seek(out_pos);
			}
		}

		// now write size of record
		qint64 final_pos = data_output->pos();
		qint64 d_size = final_pos - p - sizeof(p);

		data_output->seek(p);
		data_output->write((const char*)&d_size, sizeof(d_size));
		data_output->seek(final_pos);
	}
	//qDebug("Req: %s %s", qPrintable(op_str), qPrintable(req.url().toString()));
	QNetworkReply *reply = QNetworkAccessManager::createRequest(op, req, outgoingData);

	if (data_output != NULL) {
		connect(reply, SIGNAL(readyRead()), this, SLOT(spyConnectionData()));
		connect(reply, SIGNAL(metaDataChanged()), this, SLOT(spyMetaData()));
	}

	if (cnx_count == 0)
		started();
	cnx_count++;
	return reply;
}

void WebPuppeteerTabNetSpy::spyMetaData() {
	if (data_output == NULL) {
		return;
	}

	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	qint64 id = reply->request().attribute(QNetworkRequest::User).value<qint64>();

	// store all headers via rawHeaderPairs()
	QBuffer *buf = new QBuffer();
	buf->open(QBuffer::ReadWrite);

	// metadata update
	buf->write(QByteArray(1, '\x03'));

	// write time
	qint64 t = QDateTime::currentMSecsSinceEpoch();
	buf->write((const char *)&t, sizeof(t));

	// write request id
	buf->write((const char *)&id, sizeof(id));

	int http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QByteArray http_reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();

	buf->write((const char *)&http_status_code, sizeof(http_status_code));
	buf->write(http_reason + QByteArray(1, 0));

	// write headers
	const QList<QNetworkReply::RawHeaderPair> &headers = reply->rawHeaderPairs();
	// headers count
	int h_size = headers.size();
	buf->write((const char *)&h_size, sizeof(h_size));

	for(int i = 0; i < headers.length(); i++) {
		const QPair<QByteArray,QByteArray> &v = headers.at(i);
		buf->write(v.first + QByteArray(1, 0) + v.second + QByteArray(1, 0));
	}

	const QByteArray &b = buf->data();
	qint64 p = b.length();

	data_output->write((const char *)&p, sizeof(p));
	data_output->write(b);
}

void WebPuppeteerTabNetSpy::spyConnectionData() {
	if (data_output == NULL) {
		return;
	}

	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	qint64 ba = reply->bytesAvailable();
	if (ba == 0) {
		return;
	}
	qint64 id = reply->request().attribute(QNetworkRequest::User).value<qint64>();

	//qDebug("SPY: #%lld pos %lld ba %lld", id, reply->pos(), ba);

	// we're assuming other recipient of QNetworkReply bytesAvailable() signal will always consume the whole buffer
	QByteArray sdata = reply->peek(ba);

	// write partial response data
	qint64 p = 1+8+8+ba; // qint64 + qint64 + ba
	data_output->write((const char *)&p, sizeof(p));

	data_output->write(QByteArray(1, '\x02'));

	// write time
	qint64 t = QDateTime::currentMSecsSinceEpoch();
	data_output->write((const char *)&t, sizeof(t));

	// write request id
	data_output->write((const char *)&id, sizeof(id));

	// write data
	data_output->write(sdata, ba);
}

void WebPuppeteerTabNetSpy::spyFinished(QNetworkReply*reply) {
	if (data_output != NULL) {
		qint64 id = reply->request().attribute(QNetworkRequest::User).value<qint64>();

		qint64 p = 1+8+8;
		data_output->write((const char *)&p, sizeof(p));
		data_output->write(QByteArray(1, '\x04')); // end of request

		// write time
		qint64 t = QDateTime::currentMSecsSinceEpoch();
		data_output->write((const char *)&t, sizeof(t));

		// write request id
		data_output->write((const char *)&id, sizeof(id));

		data_output->flush();
	}

	cnx_count--;
	if (cnx_count == 0)
		allFinished();
}

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *_parent): QWebPage(_parent) {
	parent = _parent;
	spy = new WebPuppeteerTabNetSpy(this);

	// define standard values
	setViewportSize(QSize(1024,768));
	setForwardUnsupportedContent(true);

	setNetworkAccessManager(spy);

	connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadFile(QNetworkReply*)));
	connect(networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,const QList<QSslError>&)), this, SLOT(handleSslErrors(QNetworkReply*,const QList<QSslError>&)));
}

bool WebPuppeteerTab::saveNetwork(const QString &filename) {
	if (filename == "") {
		spy->setOutputFile(NULL); // will cause previous qfile to be closed
		return true;
	}

	QFile *f = new QFile(filename);
	if (!f->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		qDebug("saveNetwork: failed to create output file %s", qPrintable(filename));
		return false;
	}
	spy->setOutputFile(f);
	return true;
}

void WebPuppeteerTab::test(QNetworkReply*reply) {
	QNetworkRequest req = reply->request();
	qDebug("REQ finished: %s", qPrintable(req.url().toString()));
}

QWebPage *WebPuppeteerTab::createWindow(WebWindowType) {
	// we don't care about the type, since modal has no meaning here
	WebPuppeteerTab *tab = new WebPuppeteerTab(parent);
	// inherit settings
	tab->user_agent = user_agent;
	queue.append(tab);
	return tab;
}

QScriptValue WebPuppeteerTab::getNewWindow() {
	if (queue.size() == 0) return parent->engine().nullValue();
	return parent->engine().newQObject(queue.takeFirst(), QScriptEngine::ScriptOwnership);
}

void WebPuppeteerTab::trustCertificate(const QString &hash) {
	trusted_certificates.insert(QByteArray::fromHex(hash.toLatin1()));
}

void WebPuppeteerTab::handleSslErrors(QNetworkReply *reply,const QList<QSslError>&list) {
	bool ignore_ok = true;

	for(int i = 0; i < list.size(); i++) {
		bool this_ignore_ok = false;
		switch(list.at(i).error()) {
			case QSslError::UnableToGetLocalIssuerCertificate:
			case QSslError::CertificateUntrusted:
			case QSslError::UnableToVerifyFirstCertificate:
			{
				QByteArray hash = list.at(i).certificate().digest(QCryptographicHash::Sha1);
				if (trusted_certificates.contains(hash)) {
					this_ignore_ok = true;
//					qDebug("Ignorable SSL error");
				} else {
					qDebug("The following error could be ignored by calling tab.trustCertificate(\"%s\")", hash.toHex().data());
				}
			}
			default: break;
		}

		if (!this_ignore_ok) {
			qDebug("SSL Error: %d %s", list.at(i).error(), qPrintable(list.at(i).errorString()));
			ignore_ok = false;
		}
	}

	if (ignore_ok) {
//		qDebug("Ignoring SSL errors");
		reply->ignoreSslErrors();
	}
}

bool WebPuppeteerTab::shouldInterruptJavaScript() {
	return true;
}

void WebPuppeteerTab::javaScriptAlert(QWebFrame*, const QString &msg) {
	qDebug("Got javascript alert: %s", qPrintable(msg));
}

bool WebPuppeteerTab::javaScriptConfirm(QWebFrame*, const QString &msg) {
	qDebug("Got javascript confirm: %s", qPrintable(msg));
	return true;
}

bool WebPuppeteerTab::javaScriptPrompt(QWebFrame *, const QString &msg, const QString &defaultValue, QString *result) {
	qDebug("Got javascript prompt: %s", qPrintable(msg));
	*result = defaultValue;
	return false;
}

bool WebPuppeteerTab::supportsExtension(Extension e) {
	switch(e) {
		case QWebPage::ChooseMultipleFilesExtension: return true;
		case QWebPage::ErrorPageExtension: return true;
		default: return false;
	}
}

bool WebPuppeteerTab::extension(Extension e, ExtensionOption *option, ExtensionReturn *output) {
	switch(e) {
		case QWebPage::ChooseMultipleFilesExtension:
		{
			if (output == NULL) return false;
			//ChooseMultipleFilesExtensionReturn *ret = (ChooseMultipleFilesExtensionReturn*)output;

			// TODO
			
			return true;
		}
		case QWebPage::ErrorPageExtension:
		{
			ErrorPageExtensionOption *opt = (ErrorPageExtensionOption*)option;
			ErrorPageExtensionReturn *ret = (ErrorPageExtensionReturn*)output;

			qDebug("HTTP error %d: %s", opt->error, qPrintable(opt->errorString));
			if (ret != 0) {
				ret->baseUrl = QUrl("http://localhost/");
				ret->contentType = "text/html";
				ret->encoding = "UTF-8";
				ret->content = "<h1>Load error</h1><p>Could not load page :(</p>";
			}
			return true;
		}
		default:
			return false;
	}
}

void WebPuppeteerTab::downloadFile(QNetworkReply*reply) {
	// Won't be finished at this point, need to connect() to reply signals to detect end of download
	// Just in case, check if reply->isFinished()
	if (reply->isFinished()) {
		downloadFileFinished(reply);
	}
	connect(reply, SIGNAL(finished()), this, SLOT(downloadFileFinished()));
}

void WebPuppeteerTab::downloadFileFinished(QNetworkReply*reply) {
	if (reply == NULL)
		reply = qobject_cast<QNetworkReply*>(sender());

	if (reply->error() != QNetworkReply::NoError) {
		qDebug("error downloading file: %d", reply->error());
		return;
	}
	QString filename = reply->request().url().path();
	int idx = filename.lastIndexOf('/');
	if (idx != -1) filename = filename.mid(idx+1);

	QScriptValue arr = parent->engine().newObject();
	arr.setProperty("filename", filename);
	arr.setProperty("filedata", QString::fromLatin1(reply->readAll().toBase64()));

	file_queue.append(arr);

	qDebug("File download success: %s", qPrintable(filename));
}

QScriptValue WebPuppeteerTab::getDownloadedFile() {
	if (file_queue.isEmpty()) return parent->engine().nullValue();
	return file_queue.takeFirst();
}

WebPuppeteer *WebPuppeteerTab::getParent() {
	return parent;
}

bool WebPuppeteerTab::load(const QString &url, int timeout) {
	mainFrame()->load(QUrl(url));
	return wait(timeout);
}

void WebPuppeteerTab::waitFinish(int idle) {
	QEventLoop e;
	TimeoutTrigger t(idle);

	connect(&t, SIGNAL(timeout()), &e, SLOT(quit()));
	connect(networkAccessManager(), SIGNAL(started()), &t, SLOT(start()));
	connect(networkAccessManager(), SIGNAL(allFinished()), &t, SLOT(end()));

	// if something pending, stop timer
	if (static_cast<WebPuppeteerTabNetSpy*>(networkAccessManager())->getCount() > 0)
		t.start();

	e.exec();
}

bool WebPuppeteerTab::post(const QString &url, const QString &post, const QString content_type, int timeout) {
	QNetworkRequest req(url);
	req.setHeader(QNetworkRequest::ContentTypeHeader, content_type);
	mainFrame()->load(req, QNetworkAccessManager::PostOperation, post.toUtf8());
	return wait(timeout);
}

bool WebPuppeteerTab::browse(const QString &url, int timeout) {
	mainFrame()->load(mainFrame()->url().resolved(QUrl(url)));
	return wait(timeout);
}

void WebPuppeteerTab::back() {
	triggerAction(QWebPage::Back);
}

void WebPuppeteerTab::reload(bool force_no_cache) {
	if (force_no_cache) {
		triggerAction(QWebPage::ReloadAndBypassCache);
	} else {
		triggerAction(QWebPage::Reload);
	}
}

void WebPuppeteerTab::setReturnBool(bool r) {
	return_bool = r;
}

bool WebPuppeteerTab::screenshot(const QString &filename) {
	// disable scrollbars, not as anyone is going to use them anyway
	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	QImage img(viewportSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();

	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

	return img.save(filename);
}

bool WebPuppeteerTab::fullshot(const QString &filename) {
	// disable scrollbars, not as anyone is going to use them anyway
	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	QImage img(mainFrame()->contentsSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();

	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

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

QString WebPuppeteerTab::printBase64() {
	QTemporaryFile t;
	if (!t.open()) return QString();
	if (!print(t.fileName())) return QString();
	QByteArray data = t.readAll();
	t.remove();
	return QString::fromLatin1(data.toBase64());
}

QScriptValue WebPuppeteerTab::eval(const QString &js) {
	return parent->engine().newVariant(mainFrame()->evaluateJavaScript(js));
}

QScriptValue WebPuppeteerTab::document() {
	return parent->engine().newQObject(new WebPuppeteerWebElement(this, mainFrame()->documentElement()), QScriptEngine::ScriptOwnership);
}

void WebPuppeteerTab::interact() {
	QEventLoop e;

	QWebView *v = new QWebView();
	v->resize(QSize(1280,1024));
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

bool WebPuppeteerTab::wait(int timeout) {
	QEventLoop e;

	connect(this, SIGNAL(loadFinished(bool)), &e, SLOT(quit()));
	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setReturnBool(bool)));
	if (timeout > 0)
		QTimer::singleShot(timeout*1000, &e, SLOT(quit()));
	return_bool = false;

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

void WebPuppeteerTab::typeTab() {
	QKeyEvent ev(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
	event(&ev);
}

QString WebPuppeteerTab::getHtml() {
	return mainFrame()->toHtml();
}

void WebPuppeteerTab::overrideUserAgent(const QString &ua) {
	user_agent = ua;
}

void WebPuppeteerTab::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID) {
	qDebug("JavaScript %s:%d: %s", qPrintable(sourceID), lineNumber, qPrintable(message));
}

QString WebPuppeteerTab::userAgentForUrl(const QUrl&) const {
	if (user_agent.isEmpty())
		return "Mozilla/5.0 (%Platform%%Security%%Subplatform%) AppleWebKit/%WebKitVersion% (KHTML, like Gecko) %AppVersion Safari/%WebKitVersion%";
	return user_agent;
}

void WebPuppeteerTab::setDefaultCharset(const QString &charset) {
	settings()->setDefaultTextEncoding(charset);
}

