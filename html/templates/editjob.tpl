{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Edit Job {$jid}</h1>





			<h2>Change Project</h2>

Select new project for job: &quot;{$description}&quot;
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







		</div>

{include file="footer.tpl"}
