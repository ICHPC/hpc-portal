{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Pool Status</h1>
	
	<p>Status of compute pool at {$time}.</p>

	{if $status}
		<table>
		<tr>	
			<td><b>Machine</b></td>
			<td><b>Memory</b></td>
			<td><b># CPUs</b></td>
			<td><b>Status</b></td>
		</tr>

{section name=sec1 loop=$status}
	<tr>
		<td>{$status[sec1].hostname}</td>
		<td>{$status[sec1].memory}</td>
		<td>{$status[sec1].ncpus}</td>
		<td>{$status[sec1].status}</td>
	</tr>
{/section}

	</table>
	{else}
		<p>No machines active in pool.</p>
	{/if}




		</div>

{include file="footer.tpl"}
