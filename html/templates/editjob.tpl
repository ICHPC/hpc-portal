{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Edit Job {$jid}</h1>

{if !is_null($embargo_days)}
    <p> Job is under embargo  
    {if $embargo_days > 0} for {$embargo_days} more days.
    {else} and has passed its embargo and is awaiting
    <a href="?action=joblist&embargoed=3&orderby=6&orderdir=0&status=3">
        attention.</a>
    {/if}
{/if}



<h2>Change Project</h2>

Select new project for job
<form>
{if $projects} 
<select name="project">
{html_options values=$project_idx  output=$projects selected=$default_project_idx}
</select>

{else}
<p>No Projects Defined
{/if}



<input type="hidden" name="action" value="editjob">
<input type="hidden" name="jid"    value="{$jid}">

<p>
<input type="Submit" value="Update" >

</form>

<h2>Embargo Job</h2>
<p>You may embargo the job.  Currently this waits until the
embargo has passed, then lists the job on your home page with links to
publish it.
            
<form>
Embargo for <input name=embargo_days type="textfield" size="3" value={$default_embargo_days}>
</input> days from today.
<input type="hidden" name="jid"    value="{$jid}">
<input type="hidden" name="action" value="embargojob">
<p>
<input type="Submit" value="Embargo" >
</form>
<p>Or, cancel the embargo</p>
<form>
<input type="hidden" name="jid"    value="{$jid}">
<input type="hidden" name="action" value="cancelembargojob">
<p>
<input type="Submit" value="Cancel Embargo" >
</form>






		</div>

{include file="footer.tpl"}
