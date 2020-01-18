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
#include "fileiconprovider.h"
#include "console.h"

#define DESPERATE 0

FileIconProvider *fileIconProvider;

FileIconProvider::FileIconProvider() : QFileIconProvider()
{
    // generic Qt QFileIconProvider types
    my.file = QIcon(":/filegeneric.png");
    my.folder = QIcon(":/filefolder.png");
    my.computer = QIcon(":/computer.png");

    // PCP GUI specific images
    my.fileView = QIcon(":/fileview.png");
    my.fileFolio = QIcon(":/filefolio.png");
    my.fileArchive = QIcon(":/filearchive.png");

    // images for several other common file types
    my.fileHtml = QIcon(":/filehtml.png");
    my.fileImage = QIcon(":/fileimage.png");
    my.filePackage = QIcon(":/filepackage.png");
    my.fileSpreadSheet = QIcon(":/filespreadsheet.png");
    my.fileWordProcessor = QIcon(":/filewordprocessor.png");
}

QIcon FileIconProvider::icon(FileIconType type) const
{
    console->post("FileIconProvider::icon extended types");
    switch (type) {
    case View:
	return my.fileView;
    case Folio:
	return my.fileFolio;
    case Archive:
	return my.fileArchive;
    case Html:
	return my.fileHtml;
    case Image:
	return my.fileImage;
    case Package:
	return my.filePackage;
    case SpreadSheet:
	return my.fileSpreadSheet;
    case WordProcessor:
	return my.fileWordProcessor;
    default:
	break;
    }
    return my.file;
}

QIcon FileIconProvider::icon(IconType type) const
{
    console->post("FileIconProvider::icon type");
    switch (type) {
    case File:
	return my.file;
    case Folder:
	return my.folder;
    case Computer:
	return my.computer;
    default:
	break;
    }
    return QFileIconProvider::icon(type);
}

QString FileIconProvider::type(const QFileInfo &fi) const
{
    console->post("FileIconProvider::type string");
    return QFileIconProvider::type(fi);
}

QIcon FileIconProvider::icon(const QFileInfo &fi) const
{
#if DESPERATE
    console->post("FileIconProvider::icon - %s",
			(const char *)fi.filePath().toAscii());
#endif

    if (fi.isFile()) {
	QFile file(fi.filePath());
	file.open(QIODevice::ReadOnly);
	char block[9];
	int count = file.read(block, sizeof(block)-1);
	if (count == sizeof(block)-1) {
	    static const char *viewmagic[] = { "#kmchart", "#pmchart" };
	    static char foliomagic[] = "PCPFolio";
	    static char archmagic[] = "\0\0\0\204\120\5\46\2"; //PM_LOG_MAGIC|V2

	    if (memcmp(viewmagic[0], block, sizeof(block)-1) == 0)
		return my.fileView;
	    if (memcmp(viewmagic[1], block, sizeof(block)-1) == 0)
		return my.fileView;
	    if (memcmp(foliomagic, block, sizeof(block)-1) == 0)
		return my.fileFolio;
	    if (memcmp(archmagic, block, sizeof(block)-1) == 0)
		return my.fileArchive;
	}
#if DESPERATE
	console->post("  Got %d bytes from %s: \"%c%c%c%c%c%c%c%c\"", count,
		(const char *) fi.filePath().toAscii(), block[0], block[1],
		block[2], block[3], block[4], block[5], block[6], block[7]);
#endif
	QString ext = fi.suffix();
	if (ext == "htm" || ext == "html")
	    return my.fileHtml;
	if (ext == "svg" || ext == "gif" || ext == "jpg" || ext == "jpeg" ||
	    ext == "png" || ext == "xpm" || ext == "odg" /* ... */ )
	    return my.fileImage;
	if (ext == "tar" || ext == "tgz" || ext == "deb" || ext == "rpm" ||
	    ext == "zip" || ext == "bz2" || ext == "gz")
	    return my.filePackage;
	if (ext == "ods" || ext == "xls")
	    return my.fileSpreadSheet;
	if (ext == "odp" || ext == "doc")
	    return my.fileWordProcessor;
	return my.file;	// catch-all for every other regular file
    }
    else if (fi.isDir()) {
	return my.folder;
    }
    return QFileIconProvider::icon(fi);
}
