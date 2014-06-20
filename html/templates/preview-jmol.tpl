
{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Preview</h1>


    <script src="https://scanweb.cc.ic.ac.uk/uportal2/jmol/jmol/Jmol.js"></script> <!-- REQUIRED -->
		<script>
      jmolInitialize("http://jmol.sourceforge.net/jmol");
      jmolCheckBrowser("popup", "http://jmol.sourceforge.net/browsercheck", "onClick");
      jmolSetAppletColor("white");
    </script>
		<script>
		var caffeine = ""+<r><![CDATA[
{$content}
]]></r>;

		  jmolAppletInline("600", caffeine);
		  jmolBr();
		</script>





		</div>

{include file="footer.tpl"}
