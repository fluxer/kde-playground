//
//     Copyright 2008 Jim Bublitz <jbublitz@nwinternet.com>
//     Earlier copyrights 1998 - 2007 Jim Bublitz also apply

//                 Generated by twine

// This file is part of PyKDE4.

// PyKDE4 is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.

// PyKDE4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


class KCategorizedSortFilterProxyModel : QSortFilterProxyModel
{
%TypeHeaderCode
#include <kcategorizedsortfilterproxymodel.h>
%End



public:
    enum AdditionalRoles
    {
        CategoryDisplayRole,
        CategorySortRole
    };

                            KCategorizedSortFilterProxyModel (QObject* parent /TransferThis/ = 0);
    virtual void            sort (int column, Qt::SortOrder order = Qt::AscendingOrder);
    bool                    isCategorizedModel () const;
    void                    setCategorizedModel (bool categorizedModel);
    int                     sortColumn () const;
    Qt::SortOrder           sortOrder () const;
    void                    setSortCategoriesByNaturalComparison (bool sortCategoriesByNaturalComparison);
    bool                    sortCategoriesByNaturalComparison () const;

protected:
    virtual bool            lessThan (const QModelIndex& left, const QModelIndex& right) const;
    virtual bool            subSortLessThan (const QModelIndex& left, const QModelIndex& right) const;
    virtual int             compareCategories (const QModelIndex& left, const QModelIndex& right) const;
public:
    ~KCategorizedSortFilterProxyModel ();
};
// KCategorizedSortFilterProxyModel


