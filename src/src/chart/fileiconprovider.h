/*
 * Copyright (c) 2007, Aconex.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <QtGui/QApplication>
#include <QtGui/QFileIconProvider>

class FileIconProvider : public QFileIconProvider
{
public:
    FileIconProvider();

    typedef enum { View, Folio, Archive, Html, Image,	// IconType++
		   Package, SpreadSheet, WordProcessor } FileIconType;
    QIcon icon(FileIconType type) const;

    QIcon icon(IconType type) const;
    QIcon icon(const QFileInfo &info) const;
    QString type(const QFileInfo &info) const;

private:
    struct {
	QIcon file;
	QIcon folder;
	QIcon computer;

	QIcon fileView;		// kmchart view
	QIcon fileFolio;	// PCP folio
	QIcon fileArchive;	// PCP archive
	QIcon fileHtml;
	QIcon fileImage;
	QIcon filePackage;
	QIcon fileSpreadSheet;
	QIcon fileWordProcessor;
    } my;
};

extern FileIconProvider *fileIconProvider;

#endif	// FILEICONPROVIDER_H
