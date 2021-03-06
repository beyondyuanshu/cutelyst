/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_PLUGIN_STATUSMESSAGE
#define CUTELYST_PLUGIN_STATUSMESSAGE

#include <Cutelyst/plugin.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/context.h>

namespace Cutelyst {

class StatusMessagePrivate;
class CUTELYST_PLUGIN_STATUSMESSAGE_EXPORT StatusMessage : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StatusMessage)
public:
    StatusMessage(Application *parent);
    virtual ~StatusMessage();

    /**
     * The key prefix inside Session where messages will be stored. Defaults to "status_msg".
     */
    QString sessionPrefix() const;
    void setSessionPrefix(const QString &sessionPrefix);

    /**
     * The name of the URL param that holds the token on the page where you want to retrieve/display the status message. Defaults to "mid".
     */
    QString tokenParam() const;
    void setTokenParam(const QString &tokenParam);

    /**
     * The name of the stash key where "success" status messages are loaded when $c->load_status_msgs is called. Defaults to status_msg.
     */
    QString statusMsgStashKey() const;
    void setStatusMsgStashKey(const QString &statusMsgStashKey);

    /**
     * The name of the stash key where error messages are loaded when $c->load_status_msgs is called. Defaults to error_msg.
     */
    QString errorMgStashKey() const;
    void setErrorMgStashKey(const QString &errorMgStashKey);

    /**
     * Load both messages that match the token param (mid=###) into the stash for display by the view.
     */
    static void load(Context *c);

    /**
     * Saves an error message returning the generated message id (mid)
     */
    static QString error(Context *c, const QString &msg);

    /**
     * Saves an error message returning query parameters with the generated message id (mid) and it's token
     */
    static ParamsMultiMap errorQuery(Context *c, const QString &msg, ParamsMultiMap query = ParamsMultiMap());

    /**
     * Saves a status message returning the generated message id (mid)
     */
    static QString status(Context *c, const QString &msg);

    /**
     * Saves an status message returning query parameters with the generated message id (mid) and it's token
     */
    static ParamsMultiMap statusQuery(Context *c, const QString &msg, ParamsMultiMap query = ParamsMultiMap());

    /**
     * DEPRECATED \sa error Saves an error message returning the generated message id (mid)
     */
    static Q_DECL_DEPRECATED QString setError(Context *c, const QString &msg);

    /**
     * DEPRECATED \sa status Saves a status message returning the generated message id (mid)
     */
    static Q_DECL_DEPRECATED QString setStatus(Context *c, const QString &msg);

protected:
    StatusMessagePrivate *d_ptr;
};

}

#endif // CUTELYST_PLUGIN_STATUSMESSAGE
