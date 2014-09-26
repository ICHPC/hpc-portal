<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" />
<xsl:template match="/api/query/search">
	<xsl:for-each select="p">
		http://en.wikipedia.org/wiki/<xsl:value-of select="@title" />
<!--, Wikipedia (updated <xsl:value-of select="@timestamp"/>)</a>-->
		<xsl:text>&#10;</xsl:text>
	</xsl:for-each>
</xsl:template>

</xsl:stylesheet>

