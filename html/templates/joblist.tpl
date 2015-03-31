{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

{if $projectname != ""}
	<h1>Jobs in Project {$projectname}</h1>
{else}
	<h1>All Jobs</h1>
{/if}

<h2>View by Project</h2>

<form>Project:
<select name="byproject">
{html_options values=$project_idx  output=$projects selected=$defaultprojectidx}
</select>
Description contains:
<input type="textfield" name="filter" size="50" value="{$defaultfilter}">

<p>

Published:
<select name="published">
{html_options options=$publisheds selected=$published}
</select>

Status:
<select name="status">
{html_options options=$statuses selected=$status}
</select>

Submission time:
<select name="submittime">
{html_options options=$submittimes selected=$submittime}
</select>
<p>

Items per page <select name="numperpage">
{html_options values=$numperpages output=$numperpages selected=$defnumperpage}
</select>

Embargoed:
<select name="embargoed">
{html_options options=$embargoeds selected=$embargoed}
</select>

<p>
<input type="hidden" name="action" value="joblist">
<input type="hidden" name="subaction" value="appinput">
<input type="Submit" value="Filter" >
<button type=button onclick="window.location.href='{$clearurl}'">Reset Filter</button>
</form>

{if $job_list}
    <p>Click on a job's number to edit some of its details. Click on Publish
    to publish to the sources selected in your
    <a href="?action=profile">Profile</a>.
    Click on Embargo to embargo the job for the default number of days
    (also in your profile).

	<table width="100%">
	<tr>
    <td align="left">
		<a href="?action=joblist&page=1">First</a>
		{if $suppress_prev != "1" }
		<a href="?action=joblist&page=prev">Previous</a>
		{/if}
	</td>
    <td align="center">
        {section loop=$avail_pages name=pag}
        {if $page == $avail_pages[pag]}
            {$avail_pages[pag]}
        {else}
            <a href="?action=joblist&page={$avail_pages[pag]}">{$avail_pages[pag]}</a>
        {/if}
        {/section}
    </td>
	<td align="right">
		{if $suppress_next != "1" }
		<a href="?action=joblist&page=next">Next</a>
		{/if}
		<a href="?action=joblist&page=last">Last</a>
    </td>
	</tr>
	</table>


	
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
			<th class="MYTABLE"><a href="?action=joblist&orderby=6&orderdir={$orderdir}&byproject={$byproject}"><b>Embargo</b></a></td>
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

    {include file="publish_inc.tpl"}
    <!-- Embargo -->
    {if $job_list[sec1].embargo_status != 0}
		<td class="MYTABLE">{$job_list[sec1].embargo}
        day{if abs($job_list[sec1].embargo) != 1}s{/if}
        <a href="?action=cancelembargojob&jid={$job_list[sec1].jid}">
        (cancel)</a></td>
    {else}
        <td class="MYTABLE">
            <a href="?action=embargojob&jid={$job_list[sec1].jid}">Embargo</a>
        </td>
    {/if}
	</tr>
{/section}

	</table>


	<table width="100%">
	<tr>
    <td align="left">
		<a href="?action=joblist&page=1">First</a>
		{if $suppress_prev != "1" }
		<a href="?action=joblist&page=prev">Previous</a>
		{/if}
	</td>
    <td align="center">
        {section loop=$avail_pages name=pag}
        {if $page == $avail_pages[pag]}
            {$avail_pages[pag]}
        {else}
            <a href="?action=joblist&page={$avail_pages[pag]}">{$avail_pages[pag]}</a>
        {/if}
        {/section}
    </td>
	<td align="right">
		{if $suppress_next != "1" }
		<a href="?action=joblist&page=next">Next</a>
		{/if}
		<a href="?action=joblist&page=last">Last</a>
    </td>
	</tr>
	</table>

{else}
    <p>No jobs</p>
{/if}




		</div>

{include file="footer.tpl"}
