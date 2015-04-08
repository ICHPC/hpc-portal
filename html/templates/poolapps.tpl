{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Compute Pool Applications</h1>


{if $apps}	
<table class="MYTABLE">
		<thead>
		<tr class="MYTABLE">	
			<th class="MYTABLE">Application</td>
			<th class="MYTABLE">Script</td>
			<th class="MYTABLE">Input Files</td>
			<th class="MYTABLE">Output Files</td>
			<th class="MYTABLE">Memory/MB</td>
			<th class="MYTABLE">NCPUs</td>
			<th class="MYTABLE">Walltime/hr</td>
		</tr>
		</thead>
		<tbody>
{section name=sec1 loop=$apps}
	
	<tr class="MYTABLE">
		<td class="MYTABLE">{$apps[sec1].name}</td>
		<td class="MYTABLE">{$apps[sec1].script}</td>
		<td class="MYTABLE">
	<table class="MYTABLE"  >
{section name=sec2 loop=$apps[sec1].input}
		<tr class="MYTABLE">
		<!--<td class="MYTABLE">{$apps[sec1].input[sec2].index}</td>-->
		<td class="MYTABLE">{$apps[sec1].input[sec2].mimetype}</td>
		<td class="MYTABLE">{$apps[sec1].input[sec2].description}</td>
		</tr>
{/section}
		</table></td>
		<td class="MYTABLE"><table class="MYTABLE">
{section name=sec2 loop=$apps[sec1].output}
		<tr class="MYTABLE">
		<!--<td class="MYTABLE">{$apps[sec1].output[sec2].index}</td>-->
		<td class="MYTABLE">{$apps[sec1].output[sec2].filename}</td>
		<td class="MYTABLE">{$apps[sec1].output[sec2].mimetype}</td>
		<td class="MYTABLE">{$apps[sec1].output[sec2].description}</td>
		</tr>
{/section}
		</table></td>
		<td class="MYTABLE">{$apps[sec1].memory}</td>
		<td class="MYTABLE">{$apps[sec1].ncpus}</td>
		<td class="MYTABLE">{$apps[sec1].walltime}</td>
		<!--<td class="MYTABLE"></td>-->
	</tr>
{/section}
	</tbody>
</table>

{else}
	<p>No Applications
{/if}

{if $mypool}
<p><a href="?action=pools&amp;subaction=refresh&amp;pool={$pool}">Refresh Application List</a>
{/if}



		</div>

{include file="footer.tpl"}
