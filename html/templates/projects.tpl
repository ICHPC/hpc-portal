{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Projects</h1>
            <p>You may define projects to organise your jobs. A job is assigned
            a project when it is run; you may reassign it later.</p>

	<h2>Current Projects</h2>

{if $projects}	
<table class="MYTABLE">
		<tr>	
			<th>Project Name</th>
			<th></th><th></th>
		</tr>

{section name=sec1 loop=$projects}
	<tr>
		<td>{$projects[sec1]}</td>
		<td><a href="?action=projects&subaction=delete&project_id={$project_idx[sec1]}">Delete</a></td>
		<td><a href="?action=joblist&byproject={$project_idx[sec1]}">View jobs</a></td>
	<tr>
{/section}
</table>
{else}
	<p>No projects
{/if}

<h2>Add Project</h2>
<form>
<table class="MYTABLE">
    <tr>
    <td>Project name</td>
    <td><Input name ="name" type="textfield"></input></td>
    </tr>
	<input type = "hidden" name ="action" value="projects" >
	<input type = "hidden" name ="subaction" value="add" >
</table>
    <p></p>
	<input type="submit" value="Add">
</form>





		</div>

{include file="footer.tpl"}
