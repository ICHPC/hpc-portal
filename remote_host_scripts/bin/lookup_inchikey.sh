#!/bin/bash

#INCHI=InChI%3D1S%2FC6H5NO%2Fc8-7-6-4-2-1-3-5-6%2Fh1-5H
#I2=1S/C2H6/c1-2/h1-2H3
#I2=InChI=1S/C6H5NO/c8-7-6-4-2-1-3-5-6/h1-5H  InChI=1S/C2H6/c1-2/h1-2H3

wikipedia_lookup() {

V=`mktemp`
echo '<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" />
<xsl:template match="/api/query/search">
  <xsl:for-each select="p">
    https://en.wikipedia.org/wiki/<xsl:value-of select="@title" />
<!--, Wikipedia (updated <xsl:value-of select="@timestamp"/>)</a>-->
    <xsl:text>&#10;</xsl:text>
  </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
' > $V

	RESULT=` curl -s http://en.wikipedia.org/w/api.php \
	-G \
	--data action=query \
	--data list=search \
	--data srwhat=text \
	--data format=xml  \
	--data srlimit=1 \
	--data-binary  "srsearch=\"$INCHI\"" \
	| xsltproc  $V  -`

rm $V
}

echo "http://www.ch.ic.ac.uk/rzepa/data-descriptors/"

for INCHI_F in $*; do
	INCHI=$INCHI_F
	wikipedia_lookup 

	if [ "$RESULT" == "" ]; then
		# try without InChi prefix
		INCHI=`echo "$INCHI_F" | sed "s/InChI=//g"`
		wikipedia_lookup
	fi

	if [ "$RESULT" != "" ]; then
		echo $RESULT
	fi
done

# Pubmed

for INCHI_F in $*; do
	PUBMED=`curl -D -  http://www.ncbi.nlm.nih.gov/sites/entrez -G --data db=pccompound --data "term=$INCHI_F" -s | grep -e "^Location" | sed "s/Location: /http:/g" | sed 's/\s//g' `
	if [ "$PUBMED" != "" ]; then
		ID=`echo $PUBMED | sed 's/^.*=//g'`
#		echo "<a href=\"$PUBMED\">PubChem Compound Database ID $ID</a>"
		echo $PUBMED
	fi
#	curl -D -  http://www.ncbi.nlm.nih.gov/sites/entrez -G --data db=pcsubstance --data "term=$INCHI_F" -s | grep -q "$INCHI_F"
#
#	if [ "$?" == "0" ]; then
#		echo "<a href=\"http://www.ncbi.nlm.nih.gov/sites/entrez?db=pcsubstance&term=$INCHI_F\">Pubchem Substance Database results</a>"
#	fi

done

for INCHI_F in $*; do
#	echo "<a href=\"http://www.chemspider.com/Search.aspx?q=$INCHI_F\">Search on Chemspider</a>"
	echo "http://www.chemspider.com/Search.aspx?q=$INCHI_F"
done

# Spectra dspace:

for INCHI_F in $*; do
	TF=`mktemp`
	TFAUTH=`mktemp`
	TFHAND=`mktemp`
	curl -s -k https://spectradspace.lib.imperial.ac.uk:8443/dspace/search \
		--data-binary "query=\"$INCHI_F\"" \
		--data rpp=100 \
		--data scope=/ \
		--data sort_by=2 \
		--data order=DESC \
		--data submit=Go > $TF 
		grep -e "dspace/handle/10042" $TF \
			| sed 's/<a href="//g' \
			| sed 's/\">/ /g' \
			| sed 's/<\/a>/, /g' \
			| sed 's/\/dspace\/handle\///g' > $TFHAND

		grep -e 'span class="author"' $TF | sed 's/<[^>]*>//g' > $TFAUTH
		paste   $TFHAND $TFAUTH | \
			awk '{ 
				doi=$1;
				$1="";
				print "http://dx.doi.org/"doi;
			}'
	rm $TF
	rm $TFAUTH $TFHAND
done
