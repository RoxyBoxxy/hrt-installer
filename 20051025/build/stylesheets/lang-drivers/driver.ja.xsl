<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:output encoding="UTF-8"/>
<xsl:param name="latex.inputenc"/>
<!-- <xsl:variable name="latex.use.ucs">1</xsl:variable> -->
<!-- <xsl:param name="latex.ucs.options">cjkjis,autogenerated</xsl:param> -->
<!-- <xsl:param name="latex.fontenc">C40,T1</xsl:param> -->
<xsl:param name="latex.book.preamble.pre">
\usepackage{CJK}
\usepackage[CJK, overlap]{ruby}
\CJKencfamily[dnp]{JIS}{min}
\CJKfontenc{JIS}{dnp}
</xsl:param>
<xsl:param name="latex.hyperref.param.common">CJKbookmarks,bookmarksnumbered,colorlinks,backref,bookmarks,breaklinks,linktocpage,plainpages=false</xsl:param>
<xsl:param name="latex.book.preamble.post"/>
<xsl:param name="latex.book.preamble.post.l10n">
    <xsl:text>\renewcommand{\rmdefault}{pnc}</xsl:text>
    <xsl:text>\renewcommand{\sfdefault}{phv}</xsl:text>
</xsl:param>
<xsl:param name="latex.language.option">none</xsl:param>
<xsl:param name="latex.documentclass.book">a4paper,10pt,twoside,openright</xsl:param>
</xsl:stylesheet>