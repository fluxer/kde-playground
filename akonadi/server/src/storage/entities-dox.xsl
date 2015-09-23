<!--
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
<xsl:output method="text" encoding="utf-8"/>

<xsl:template match="/">
// autogenerated from akonadi.db and entities-dox.xsl
/**
\page akonadi_server_database Database Design

\section akonadi_server_database_layout Database Layout

This is an overview of the database layout of the \ref akonadi_design_storage "storage server".
The schema gets generated by the server using the helper class DbInitializer, based on the
definition found in @c server/src/storage/akonadidb.xml.

\dot
digraph "Akonadi Database Layout" {
  graph [rankdir="LR" fontsize="10"]
  node [fontsize="10" shape="record" style="filled" fillcolor="lightyellow"]
  edge [fontsize="10"]

  <xsl:for-each select="database/table">
  <xsl:value-of select="@name"/>[label="&lt;1&gt;<xsl:value-of select="@name"/><xsl:for-each select="column">|&lt;<xsl:value-of select="@name"/>&gt;<xsl:value-of select="@name"/></xsl:for-each>" URL="classAkonadi_1_1<xsl:value-of select="@name"/>.html"];

  <xsl:for-each select="column[@refTable != '']">
  <xsl:value-of select="../@name"/>:<xsl:value-of select="@name"/> -&gt; <xsl:value-of select="@refTable"/>:<xsl:value-of select="@refColumn"/>[label="n:1"];
  </xsl:for-each>

  </xsl:for-each>

  <xsl:for-each select="database/relation">
  <xsl:value-of select="@table1"/>:<xsl:value-of select="@column1"/> -&gt; <xsl:value-of select="@table2"/>:<xsl:value-of select="@column2"/>[label="n:m" arrowtail=normal];
  </xsl:for-each>
}
\enddot


\section akonadi_server_database_codegeneration Code Generation

Code to access the database is generated from @c akonadidb.xml using an XSL stylesheet, @c entities.xsl.
The generated code encapsulates basic database operations, such as retrieving, inserting, updating and
removing records, as well as methods to retrieve related records. They also contain methods to retrieve
table and column names for creating SQL queries in a typo-safe way.

The following classes are generated:
<xsl:for-each select="database/table">
- Akonadi::Server::<xsl:value-of select="@name"/>
</xsl:for-each>

For the helper tables used for n:m relations, the following classes are generated. They are only useful
when creating SQL queries that handle the n:m relations manually.
<xsl:for-each select="database/relation">
- Akonadi::Server::<xsl:value-of select="@table1"/><xsl:value-of select="@table2"/>Relation</xsl:for-each>

*/
</xsl:template>

</xsl:stylesheet>

