/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "response_p.h"

#include "context_p.h"
#include "engine.h"
#include "common.h"

#include <QtCore/QJsonDocument>

using namespace Cutelyst;

Response::Response(Context *c, Engine *engine, const Cutelyst::Headers &defaultHeaders) : QIODevice(c)
  , d_ptr(new ResponsePrivate(c, engine, defaultHeaders))
{
    open(QIODevice::WriteOnly);
}

qint64 Response::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return -1;
}

qint64 Response::writeData(const char *data, qint64 len)
{
    Q_D(Response);

    if (len <= 0) {
        return len;
    }

    // Finalize headers if someone manually writes output
    if (!(d->flags & ResponsePrivate::FinalizedHeaders)) {
        d->engine->finalizeHeaders(d->context);
        d->flags |= ResponsePrivate::IOWrite;
    }

    return d->engine->write(d->context, data, len, d->context->engineData());
}

Response::~Response()
{
    delete d_ptr;
}

quint16 Response::status() const
{
    Q_D(const Response);
    return d->status;
}

void Response::setStatus(quint16 status)
{
    Q_D(Response);
    d->status = status;
}

bool Response::hasBody() const
{
    Q_D(const Response);
    return !d->bodyData.isEmpty() || d->bodyIODevice || (d->flags & ResponsePrivate::IOWrite);
}

QByteArray &Response::body()
{
    Q_D(Response);
    if (d->bodyIODevice) {
        delete d->bodyIODevice;
        d->bodyIODevice = nullptr;
    }

    return d->bodyData;
}

QIODevice *Response::bodyDevice() const
{
    Q_D(const Response);
    return d->bodyIODevice;
}

void Response::setBody(QIODevice *body)
{
    Q_D(Response);
    Q_ASSERT(body && body->isOpen() && body->isReadable());

    body->setParent(d->context);

    d->bodyData = QByteArray();
    d->bodyIODevice = body;
}

void Response::setBody(const QByteArray &body)
{
    Q_D(Response);
    qDebug() << "setBody" << body;
    d->setBodyData(body);
}

void Response::setJsonBody(const QJsonDocument &documment)
{
    Q_D(Response);
    const QByteArray body = documment.toJson(QJsonDocument::Compact);
    d->setBodyData(body);
    d->headers.setContentType(QStringLiteral("application/json"));
}

QString Response::contentEncoding() const
{
    Q_D(const Response);
    return d->headers.contentEncoding();
}

void Cutelyst::Response::setContentEncoding(const QString &encoding)
{
    Q_D(Response);
    if (d->flags & ResponsePrivate::FinalizedHeaders) {
        qCWarning(CUTELYST_RESPONSE, "Useless setting a header value after finalize_headers and the response callback has been called. Not what you want.");
    }
    d->headers.setContentEncoding(encoding);
}

qint64 Response::contentLength() const
{
    Q_D(const Response);
    return d->headers.contentLength();
}

void Response::setContentLength(qint64 length)
{
    Q_D(Response);
    if (d->flags & ResponsePrivate::FinalizedHeaders) {
        qCWarning(CUTELYST_RESPONSE, "Useless setting a header value after finalize_headers and the response callback has been called. Not what you want.");
    }
    d->headers.setContentLength(length);
}

QString Response::contentType() const
{
    Q_D(const Response);
    return d->headers.contentType();
}

QString Response::contentTypeCharset() const
{
    Q_D(const Response);
    return d->headers.contentTypeCharset();
}

QVariant Response::cookie(const QByteArray &name) const
{
    Q_D(const Response);
    return QVariant::fromValue(d->cookies.value(name));
}

QList<QNetworkCookie> Response::cookies() const
{
    Q_D(const Response);
    return d->cookies.values();
}

void Response::setCookie(const QNetworkCookie &cookie)
{
    Q_D(Response);
    d->cookies.insert(cookie.name(), cookie);
}

void Response::setCookies(const QList<QNetworkCookie> &cookies)
{
    Q_D(Response);
    for (const QNetworkCookie &cookie : cookies) {
        d->cookies.insert(cookie.name(), cookie);
    }
}

int Response::removeCookies(const QByteArray &name)
{
    Q_D(Response);
    return d->cookies.remove(name);
}

void Response::redirect(const QUrl &url, quint16 status)
{
    Q_D(Response);
    d->location = url;
    setStatus(status);
    if (url.isValid() && !hasBody()) {
        QByteArray buf = QByteArrayLiteral(
                    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0"
                    "Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                    "  <head>\n"
                    "    <title>Moved</title>\n"
                    "  </head>\n"
                    "  <body>\n"
                    "     <p>This item has moved <a href=");
        buf.append(url.toEncoded());
        buf.append(QByteArrayLiteral(">here</a>.</p>\n"
                                     "  </body>\n"
                                     "</html>\n"));
        d->setBodyData(buf);
        d->headers.setContentType(QStringLiteral("text/html; charset=utf-8"));
    }
}

void Response::redirect(const QString &url, quint16 status)
{
    redirect(QUrl(url), status);
}

QUrl Response::location() const
{
    Q_D(const Response);
    return d->location;
}

QString Response::header(const QString &field) const
{
    Q_D(const Response);
    return d->headers.header(field);
}

void Response::setHeader(const QString &field, const QString &value)
{
    Q_D(Response);
    if (d->flags & ResponsePrivate::FinalizedHeaders) {
        qCWarning(CUTELYST_RESPONSE, "Useless setting a header value after finalize_headers and the response callback has been called. Not what you want.");
    }
    d->headers.setHeader(field, value);
}

Headers &Response::headers()
{
    Q_D(Response);
    return d->headers;
}

bool Response::isSequential() const
{
    return true;
}

void ResponsePrivate::setBodyData(const QByteArray &body)
{
    if (bodyIODevice) {
        delete bodyIODevice;
        bodyIODevice = nullptr;
    }
    bodyData = body;
    headers.setContentLength(body.size());
}

#include "moc_response.cpp"
