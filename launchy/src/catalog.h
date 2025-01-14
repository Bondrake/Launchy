/*
Launchy: Application Launcher
Copyright (C) 2007-2009  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CATALOG_H
#define CATALOG_H

#include <QString>
#include <QDataStream>
#include <QFileInfo>
#include <QSet>


/**
\brief CatItem (Catalog Item) stores a single item in the index
*/
class CatItem {
public:

    /** The full path of the indexed item */
    QString fullPath;
    /** The abbreviated name of the indexed item */
    QString shortName;
    /** The lowercase name of the indexed item */
    QString lowName;
    /** A path to an icon for the item */
    QString icon;
    /** How many times this item has been called by the user */
    int usage;
    /** This is unused, and meant for plugin writers and future extensions */
    void* data;
    /** The plugin id of the creator of this CatItem */
    int id;
    /** an hash **/
    size_t hash;

    CatItem() : usage(0), data(0), id(0), hash(0) {}



    CatItem(QString full, bool isDir = false)
        : usage(0), data(0), id(0), fullPath(full), hash(0) {
            int last = fullPath.lastIndexOf("/");
            if (last == -1) {
                shortName = fullPath;

            } else {
                shortName = fullPath.mid(last+1);
                if (!isDir)
                    shortName = shortName.mid(0,shortName.lastIndexOf("."));
            }

            lowName = shortName.toLower();
            data = NULL;
            usage = 0;
            id = 0;
            calculateHash();
    }


    CatItem(QString full, QString shortN)
        : usage(0), data(0), id(0), fullPath(full), shortName(shortN), hash(0)
    {
        lowName = shortName.toLower();
        data = NULL;
        usage = 0;
        id = 0;
        calculateHash();
    }

    CatItem(QString full, QString shortN, uint i_d)
        : usage(0), data(0), fullPath(full), shortName(shortN), id(i_d), hash(0)
    {
        lowName = shortName.toLower();
        data = NULL;
        usage = 0;
        calculateHash();
    }
    /** This is the constructor most used by plugins
    \param full The full path of the file to execute
    \param The abbreviated name for the entry
    \param i_d Your plugin id (0 for Launchy itself)
    \param iconPath The path to the icon for this entry
    \warning It is usually a good idea to append ".your_plugin_name" to the end of the full parameter
    so that there are not multiple items in the index with the same full path.
    */
    CatItem(QString full, QString shortN, uint i_d, QString iconPath)
        : usage(0), data(0), fullPath(full), shortName(shortN), icon(iconPath), id(i_d), hash(0)
    {
        lowName = shortName.toLower();
        data = NULL;
        usage = 0;
        calculateHash();
    }

    CatItem(const CatItem &s) {
        fullPath = s.fullPath;
        shortName = s.shortName;
        lowName = s.lowName;
        icon = s.icon;
        usage = s.usage;
        data = s.data;
        id = s.id;
        hash = s.hash;
    }

    CatItem& operator=( const CatItem &s ) {
        fullPath = s.fullPath;
        shortName = s.shortName;
        lowName = s.lowName;
        icon = s.icon;
        usage = s.usage;
        data = s.data;
        id = s.id;
        hash = s.hash;
        return *this;
    }

    void calculateHash()
    {
        if ( isLink() ) {
            QFileInfo fime(fullPath);
            QString linkTarget = fime.symLinkTarget();
            if (linkTarget != "") {
                hash = qHash(linkTarget);
            } else {
                // Shortcut that points to virtual objects doesn't have target on Windows.
                // Instead use file content for hash.
                QFile file(fullPath);
                if (file.open(QFile::ReadOnly)) {
                    hash = qHash(file.readAll());
                    file.close();
                } else {
                    // fallback to fullPath
                    hash = qHash(fullPath);
                }
            }
        } else {
            hash = qHash(fullPath);
        }
    }

    bool isLink() const
    {
        QFileInfo info(fullPath);
        return info.isSymLink() || info.suffix().toLower() == "lnk";
    }

    bool operator==(const CatItem& other) const{
        return hash == other.hash;
    }

    bool operator!=(const CatItem& other) const
    {
        return !(*this == other);
    }
};


/** InputData shows one segment (between tabs) of a user's query
    A user's query is typically represented by List<InputData>
    and each element of the list represents a segment of the query.

    E.g.  query = "google <tab> this is my search" will have 2 InputData segments
    in the list.  One for "google" and one for "this is my search"
*/
class InputData
{
private:
    /** The user's entry */
    QString text;
    /** Any assigned labels to this query segment */
    QSet<uint> labels;
    /** A pointer to the best catalog match for this segment of the query */
    CatItem topResult;
    /** The plugin id of this query's owner */
    uint id;
public:
    /** Get the labels applied to this query segment */
    QSet<uint>  getLabels() { return labels; }
    /** Apply a label to this query segment */
    void setLabel(uint l) { labels.insert(l); }
    /** Remove a label from this query segment */
    void removeLabel(uint l) { labels.remove(l); }
    /** Check if it has the given label applied to it */
    bool hasLabel(uint l) { return labels.contains(l); }

    /** Set the id of this query

    This can be used to override the owner of the selected catalog item, so that
    no matter what item is chosen from the catalog, the given plugin will be the one
    to execute it.

    \param i The plugin id of the plugin to execute the query's best match from the catalog
    */
    void setID(uint i) { id = i; }

    /** Returns the current owner id of the query */
    uint getID() const { return id; }

    /** Get the text of the query segment */
    QString  getText() const { return text; }

    /** Set the text of the query segment */
    void setText(QString t) { text = t; }

    /** Get the text of the query segment */
    bool hasText() const { return text.length() > 0; }

    /** Get a pointer to the best catalog match for this segment of the query */
    CatItem&  getTopResult() { return topResult; }

    /** Change the best catalog match for this segment */
    void setTopResult(CatItem sr) { topResult = sr; }

    InputData() { id = 0; }
    InputData(QString str) : text(str) { id = 0;}

    friend QDataStream &operator<<(QDataStream &out, const InputData &inputData);
    friend QDataStream &operator>>(QDataStream &in, InputData &inputData);
};

bool CatLess(CatItem* left, CatItem* right);
bool CatLessNoPtr(CatItem& a, CatItem& b);

inline QDataStream &operator<<(QDataStream &out, const CatItem &item) {
    out << item.fullPath;
    out << item.shortName;
    out << item.lowName;
    out << item.icon;
    out << item.usage;
    out << item.id;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, CatItem &item) {
    in >> item.fullPath;
    in >> item.shortName;
    in >> item.lowName;
    in >> item.icon;
    in >> item.usage;
    in >> item.id;
    item.calculateHash();
    return in;
}


inline QDataStream &operator<<(QDataStream &out, const InputData &inputData) {
    out << inputData.text;
    out << inputData.labels;
    out << inputData.topResult;
    out << inputData.id;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, InputData &inputData) {
    in >> inputData.text;
    in >> inputData.labels;
    in >> inputData.topResult;
    in >> inputData.id;
    return in;
}


#endif
