{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Create new Job</h1>

			<h2> Select Application Type</h2>


<form>

{if $apps}

<select name="application">
{html_options values=$app_idx  output=$apps selected=$default_app_idx}
</select>


			<h2>Select Project</h2>

{if $projects} 
<select name="project">
{html_options values=$project_idx  output=$projects selected=$default_project_idx}
</select>

{else}
<p>No Projects Defined
{/if}

<input type="hidden" name="action" value="newjob">
<input type="hidden" name="pool" value="{$pool}">
<input type="hidden" name="subaction" value="appinput">

<p>
<input type="Submit" value="Continue" >

</form>

{else}

<P> No applications configured
{/if}





		</div>

{include file="footer.tpl"}
