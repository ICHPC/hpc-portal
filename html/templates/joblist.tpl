{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

{if $projectname != ""}
	<h1>Jobs in Project {$projectname}</h1>
{else}
	<h1>All Jobs</h1>
{/if}

<h2>View by Project</h2>

<form>
<select name="byproject">
<option label="{$defaultprojectname}" value="{$defaultprojectid}">{$defaultprojectname}</option>
{html_options values=$project_idx  output=$projects}
</select>
Filter <input type="textfield" name="filter" size="50" value="{$defaultfilter}">
<input type="hidden" name="action" value="joblist">
<input type="hidden" name="subaction" value="appinput">

<p>
<input type="Submit" value="Continue" >

</form>


	<table width="100%">
	<tr>
	<td align="left">
		{if $suppress_prev != "1" }
		<a href="?action=joblist&page=prev">Previous</a>
		{/if}
	</td>
	<td align="right">
		{if $suppress_next != "1" }
		<a href="?action=joblist&page=next">Next</a>
		{/if}
	</td>
	</tr>
	</table>


	
	{if $job_list}
		<table class="MYTABLE">
		<thead>
		<tr>	
			<th ><a href="?action=joblist&orderby=0&orderdir={$orderdir}&byproject={$byproject}">Job ID</a></td>
			<th class="MYTABLE"><a href="?action=joblist&orderby=1&orderdir={$orderdir}&byproject={$byproject}"><B>Application</B></a></td>
			<th class="MYTABLE"><b>Description</b></td>
			<th class="MYTABLE"><a href="?action=joblist&orderby=2&orderdir={$orderdir}&byproject={$byproject}"><B>Submission Time</B></a></td>
			<th class="MYTABLE"><a href="?action=joblist&orderby=3&orderdir={$orderdir}&byproject={$byproject}"><B>Wall Time</B></a></td>
			<th class="MYTABLE"><a href="?action=joblist&orderby=4&orderdir={$orderdir}&byproject={$byproject}"><B>Status</B></a></td>
			<th class="MYTABLE"><b>Input files</b></td>
			<th class="MYTABLE"><b>Output files</b></td>
			<th class="MYTABLE"><b>Delete</b></td>
			<th class="MYTABLE"><a href="?action=joblist&orderby=5&orderdir={$orderdir}&byproject={$byproject}"><b>Repository</b></a></td>
		</tr>
		</thead>

{section name=sec1 loop=$job_list}
	<tr>
		<td class="MYTABLE"><a href="?action=editjob&jid={$job_list[sec1].jid}">{$job_list[sec1].jid}</a></td>
		<td class="MYTABLE">{$job_list[sec1].app_name}</td>
		<td class="MYTABLE">{$job_list[sec1].description}</td>
		<td class="MYTABLE">{$job_list[sec1].submit_time}</td>
		<td class="MYTABLE">{$job_list[sec1].wall_time}</td>
		<td class="MYTABLE">{$job_list[sec1].status}</td>

<td width="10%">
<form>
<table class="MYTABLE"><tr><td class="MYTABLE">
<select name="inputfile">
{html_options values=$job_list[sec1].input_values selected=$job_list[sec1].input_selected output=$job_list[sec1].input_name}
</select>
</td></tr>
<tr><td class="MYTABLE"><Input type="submit" name="subaction" value="Download"></td></tr>
<!--<tr><td class="MYTABLE"><Input type="submit" name="subaction" value="Preview"></td></tr>-->
</table>
<input type="hidden" name="jid" value="{$job_list[sec1].jid}">
<input type="hidden" name="action" value="inputdownload">
</form>
</td>

<td width="10%">
<form>
<table class="MYTABLE">
<tr><td class="MYTABLE">
<select name="outputfile">
{html_options values=$job_list[sec1].output_values selected=$job_list[sec1].output_selected output=$job_list[sec1].output_name}
</select></td></tr>
<tr><td class="MYTABLE"><Input type="submit" name="subaction" value="Download">
<!--<Input type="submit" name="subaction" value="Preview">-->
</td></tr>
</table>
<input type="hidden" name="jid" value="{$job_list[sec1].jid}">
<input type="hidden" name="action" value="outputdownload">
</form>

</td>




		<!--<td class="MYTABLE"><a href="?action=delete&jid={$job_list[sec1].jid}&orderby={$orderby}&orderdir={$orderdir}&byproject={$byproject}">Delete</a></td>-->
		<td class="MYTABLE"><a href="?action=delete&jid={$job_list[sec1].jid}">Delete</a></td>

	{if $job_list[sec1].publish == "publish" }
		<td class="MYTABLE"><a href="?action=publish&subaction=publish&jid={$job_list[sec1].jid}">Publish</a></td>
	{elseif $job_list[sec1].publish == "view" }
		<td class="MYTABLE">
		{if !empty($job_list[sec1].handle) }
				<a href="http://hdl.handle.net/{$job_list[sec1].handle}">Dspace</a>
		{/if}
		{if !empty($job_list[sec1].chempound) }
				</br><a href="{$job_list[sec1].chempound}">Chempound</a>
		{/if}
		{if !empty($job_list[sec1].figshare) }
				{if !empty($job_list[sec1].figshare_draft) && $job_list[sec1].figshare_draft=="1"}
					</br><a href="http://figshare.com/preview/_preview/{$job_list[sec1].figshare}">Figshare</a>&nbsp;<a href="?action=figsharepub&jid={$job_list[sec1].jid}">(Publish)</a>
				{else}
					</br><a href="http://dx.doi.org/{$job_list[sec1].figshare}">Figshare</a>
				{/if}
		{/if}

		</td>
	{elseif $job_list[sec1].publish == "na" }
		<td class="MYTABLE">---</td>
	{else}
		<td class="MYTABLE"></td>
	{/if}

	</tr>
{/section}

	</table>


	<table width="100%">
	<tr>
	<td align="left">
		{if $suppress_prev != "1" }
		<a href="?action=joblist&page=prev">Previous</a>
		{/if}
	</td>
	<td align="right">
		{if $suppress_next != "1" }
		<a href="?action=joblist&page=next">Next</a>
		{/if}
	</td>
	</tr>
	</table>

	{else}
		<p>No jobs</p>
	{/if}




		</div>

{include file="footer.tpl"}
